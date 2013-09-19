/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2011, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TDecCAVLC.cpp
    \brief    CAVLC decoder class
*/

#include "TDecCAVLC.h"
#include "SEIread.h"

//! \ingroup TLibDecoder
//! \{

#if ENC_DEC_TRACE

#define READ_CODE(size, code, name)     xReadCodeTr ( size, code, name )
#define READ_UVLC(      code, name)     xReadUvlcTr (       code, name )
#define READ_SVLC(      code, name)     xReadSvlcTr (       code, name )
#define READ_FLAG(      code, name)     xReadFlagTr (       code, name )

Void  xTraceSPSHeader (TComSPS *pSPS)
{
  fprintf( g_hTrace, "=========== Sequence Parameter Set ID: %d ===========\n", pSPS->getSPSId() );
}

Void  xTracePPSHeader (TComPPS *pPPS)
{
  fprintf( g_hTrace, "=========== Picture Parameter Set ID: %d ===========\n", pPPS->getPPSId() );
}

Void  xTraceSliceHeader (TComSlice *pSlice)
{
  fprintf( g_hTrace, "=========== Slice ===========\n");
}


Void  TDecCavlc::xReadCodeTr           (UInt length, UInt& rValue, const Char *pSymbolName)
{
  xReadCode (length, rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(%d) : %d\n", pSymbolName, length, rValue ); 
}

Void  TDecCavlc::xReadUvlcTr           (UInt& rValue, const Char *pSymbolName)
{
  xReadUvlc (rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(v) : %d\n", pSymbolName, rValue ); 
}

Void  TDecCavlc::xReadSvlcTr           (Int& rValue, const Char *pSymbolName)
{
  xReadSvlc(rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s s(v) : %d\n", pSymbolName, rValue ); 
}

Void  TDecCavlc::xReadFlagTr           (UInt& rValue, const Char *pSymbolName)
{
  xReadFlag(rValue);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(1) : %d\n", pSymbolName, rValue ); 
}

#else

#define READ_CODE(size, code, name)     xReadCode ( size, code )
#define READ_UVLC(      code, name)     xReadUvlc (       code )
#define READ_SVLC(      code, name)     xReadSvlc (       code )
#define READ_FLAG(      code, name)     xReadFlag (       code )

#endif



// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecCavlc::TDecCavlc()
{
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
  m_iSliceGranularity = 0;
#endif
}

TDecCavlc::~TDecCavlc()
{
  
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 * unmarshal a sequence of SEI messages from bitstream.
 */
void TDecCavlc::parseSEI(SEImessages& seis)
{
  assert(!m_pcBitstream->getNumBitsUntilByteAligned());
  do
  {
    parseSEImessage(*m_pcBitstream, seis);
    /* SEI messages are an integer number of bytes, something has failed
     * in the parsing if bitstream not byte-aligned */
    assert(!m_pcBitstream->getNumBitsUntilByteAligned());
  } while (0x80 != m_pcBitstream->peekBits(8));
  assert(m_pcBitstream->getNumBitsLeft() == 8); /* rsbp_trailing_bits */
}

Void TDecCavlc::parsePPS(TComPPS* pcPPS)
{
#if ENC_DEC_TRACE  
  xTracePPSHeader (pcPPS);
#endif
  UInt  uiCode;
  READ_UVLC( uiCode, "pic_parameter_set_id");                      pcPPS->setPPSId (uiCode);
  READ_UVLC( uiCode, "seq_parameter_set_id");                      pcPPS->setSPSId (uiCode);
  // entropy_coding_mode_flag
  READ_UVLC( uiCode, "num_temporal_layer_switching_point_flags" ); pcPPS->setNumTLayerSwitchingFlags( uiCode );
  for ( UInt i = 0; i < pcPPS->getNumTLayerSwitchingFlags(); i++ )
  {
    READ_FLAG( uiCode, "temporal_layer_switching_point_flag" );    pcPPS->setTLayerSwitchingFlag( i, uiCode > 0 ? true : false );
  }
  
  // num_ref_idx_l0_default_active_minus1
  // num_ref_idx_l1_default_active_minus1
  // pic_init_qp_minus26  /* relative to 26 */
  READ_FLAG( uiCode, "constrained_intra_pred_flag" );              pcPPS->setConstrainedIntraPred( uiCode ? true : false );
#if FINE_GRANULARITY_SLICES
  READ_CODE( 2, uiCode, "slice_granularity" );                     pcPPS->setSliceGranularity(uiCode);
#endif
#if E045_SLICE_COMMON_INFO_SHARING
  READ_FLAG( uiCode, "shared_pps_info_enabled_flag" );             pcPPS->setSharedPPSInfoEnabled( uiCode ? true : false);
#endif

  // alf_param() ?

#if SUB_LCU_DQP
  if( pcPPS->getSPS()->getUseDQP() )
  {
    READ_UVLC( uiCode, "max_cu_qp_delta_depth");
    pcPPS->setMaxCuDQPDepth(uiCode);
    pcPPS->setMinCuDQPSize( pcPPS->getSPS()->getMaxCUWidth() >> ( pcPPS->getMaxCuDQPDepth()) );
  }
  else
  {
    pcPPS->setMaxCuDQPDepth( 0 );
    pcPPS->setMinCuDQPSize( pcPPS->getSPS()->getMaxCUWidth() >> ( pcPPS->getMaxCuDQPDepth()) );
  }
#endif


  return;
}

Void TDecCavlc::parseSPS(TComSPS* pcSPS)
{
#if ENC_DEC_TRACE  
  xTraceSPSHeader (pcSPS);
#endif
  
  UInt  uiCode;

  READ_CODE( 8,  uiCode, "profile_idc" );                        pcSPS->setProfileIdc( uiCode );
  READ_CODE( 8,  uiCode, "reserved_zero_8bits" );
  READ_CODE( 8,  uiCode, "level_idc" );                          pcSPS->setLevelIdc( uiCode );
  READ_UVLC(     uiCode, "seq_parameter_set_id" );               pcSPS->setSPSId( uiCode );
  READ_CODE( 3,  uiCode, "max_temporal_layers_minus1" );         pcSPS->setMaxTLayers( uiCode+1 );
  READ_CODE( 16, uiCode, "pic_width_in_luma_samples" );          pcSPS->setWidth       ( uiCode    );
  READ_CODE( 16, uiCode, "pic_height_in_luma_samples" );         pcSPS->setHeight      ( uiCode    );
  //READ_UVLC ( uiCode, "pic_width_in_luma_samples" ); pcSPS->setWidth       ( uiCode    );
  //READ_UVLC ( uiCode, "pic_height_in_luma_samples" ); pcSPS->setHeight      ( uiCode    );
#if FULL_NBIT
  READ_UVLC(     uiCode, "bit_depth_luma_minus8" );
  g_uiBitDepth = 8 + uiCode;
  g_uiBitIncrement = 0;
  pcSPS->setBitDepth(g_uiBitDepth);
  pcSPS->setBitIncrement(g_uiBitIncrement);
#else
  READ_UVLC(     uiCode, "bit_depth_luma_minus8" );
  g_uiBitDepth = 8;
  g_uiBitIncrement = uiCode;
  pcSPS->setBitDepth(g_uiBitDepth);
  pcSPS->setBitIncrement(g_uiBitIncrement);
#endif
  
  g_uiBASE_MAX  = ((1<<(g_uiBitDepth))-1);
  
#if IBDI_NOCLIP_RANGE
  g_uiIBDI_MAX  = g_uiBASE_MAX << g_uiBitIncrement;
#else
  g_uiIBDI_MAX  = ((1<<(g_uiBitDepth+g_uiBitIncrement))-1);
#endif
  READ_UVLC( uiCode,    "bit_depth_chroma_minus8" );
#if E057_INTRA_PCM && E192_SPS_PCM_BIT_DEPTH_SYNTAX
  READ_CODE( 4, uiCode, "pcm_bit_depth_luma_minus1" );           pcSPS->setPCMBitDepthLuma   ( 1 + uiCode );
  READ_CODE( 4, uiCode, "pcm_bit_depth_chroma_minus1" );         pcSPS->setPCMBitDepthChroma ( 1 + uiCode );
#endif
#if DISABLE_4x4_INTER
  xReadFlag( uiCode ); pcSPS->setDisInter4x4( uiCode ? true : false );
#endif
  // log2_max_frame_num_minus4
  // pic_order_cnt_type
  // if( pic_order_cnt_type  = =  0 )
  //   log2_max_pic_order_cnt_lsb_minus4
  // else if( pic_order_cnt_type  = =  1 ) {
  //   delta_pic_order_always_zero_flag
  //   offset_for_non_ref_pic
  //   num_ref_frames_in_pic_order_cnt_cycle
  //   for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
  //     offset_for_ref_frame[ i ]
  //  }
  // max_num_ref_frames
  // gaps_in_frame_num_value_allowed_flag
  READ_UVLC( uiCode, "log2_min_coding_block_size_minus3" );
  UInt log2MinCUSize = uiCode + 3;
  READ_UVLC( uiCode, "log2_diff_max_min_coding_block_size" );
  UInt uiMaxCUDepthCorrect = uiCode;
  pcSPS->setMaxCUWidth  ( 1<<(log2MinCUSize + uiMaxCUDepthCorrect) ); g_uiMaxCUWidth  = 1<<(log2MinCUSize + uiMaxCUDepthCorrect);
  pcSPS->setMaxCUHeight ( 1<<(log2MinCUSize + uiMaxCUDepthCorrect) ); g_uiMaxCUHeight = 1<<(log2MinCUSize + uiMaxCUDepthCorrect);
  READ_UVLC( uiCode, "log2_min_transform_block_size_minus2" );   pcSPS->setQuadtreeTULog2MinSize( uiCode + 2 );
  READ_UVLC( uiCode, "log2_diff_max_min_transform_block_size" ); pcSPS->setQuadtreeTULog2MaxSize( uiCode + pcSPS->getQuadtreeTULog2MinSize() );
  pcSPS->setMaxTrSize( 1<<(uiCode + pcSPS->getQuadtreeTULog2MinSize()) );
#if E057_INTRA_PCM
  READ_UVLC( uiCode, "log2_min_pcm_coding_block_size_minus3" );  pcSPS->setPCMLog2MinSize (uiCode+3); 
#endif
  READ_UVLC( uiCode, "max_transform_hierarchy_depth_inter" );    pcSPS->setQuadtreeTUMaxDepthInter( uiCode+1 );
  READ_UVLC( uiCode, "max_transform_hierarchy_depth_intra" );    pcSPS->setQuadtreeTUMaxDepthIntra( uiCode+1 );
  g_uiAddCUDepth = 0;
  while( ( pcSPS->getMaxCUWidth() >> uiMaxCUDepthCorrect ) > ( 1 << ( pcSPS->getQuadtreeTULog2MinSize() + g_uiAddCUDepth )  ) ) g_uiAddCUDepth++;    
  pcSPS->setMaxCUDepth( uiMaxCUDepthCorrect+g_uiAddCUDepth  ); g_uiMaxCUDepth  = uiMaxCUDepthCorrect+g_uiAddCUDepth;
  // BB: these parameters may be removed completly and replaced by the fixed values
  pcSPS->setMinTrDepth( 0 );
  pcSPS->setMaxTrDepth( 1 );
#if LM_CHROMA 
  READ_FLAG( uiCode, "chroma_pred_from_luma_enabled_flag" );     pcSPS->setUseLMChroma ( uiCode ? true : false ); 
#endif
#if MTK_NONCROSS_INLOOP_FILTER
  READ_FLAG( uiCode, "loop_filter_across_slice_flag" );          pcSPS->setLFCrossSliceBoundaryFlag( uiCode ? true : false);
#endif
#if MTK_SAO
  READ_FLAG( uiCode, "sample_adaptive_offset_enabled_flag" );    pcSPS->setUseSAO       ( uiCode ? true : false );  
#endif
  READ_FLAG( uiCode, "adaptive_loop_filter_enabled_flag" );      pcSPS->setUseALF ( uiCode ? true : false );
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  READ_FLAG( uiCode, "pcm_loop_filter_disable_flag" );           pcSPS->setPCMFilterDisableFlag ( uiCode ? true : false );
#endif
  READ_FLAG( uiCode, "cu_qp_delta_enabled_flag" );               pcSPS->setUseDQP ( uiCode ? true : false );
  READ_FLAG( uiCode, "temporal_id_nesting_flag" );               pcSPS->setTemporalIdNestingFlag ( uiCode > 0 ? true : false );

  // !!!KS: Syntax not in WD !!!

  xReadUvlc ( uiCode ); pcSPS->setPadX        ( uiCode    );
  xReadUvlc ( uiCode ); pcSPS->setPadY        ( uiCode    );

  xReadFlag( uiCode ); pcSPS->setUseLDC ( uiCode ? true : false );
  xReadFlag( uiCode ); pcSPS->setUseMRG ( uiCode ? true : false );
  
  // AMVP mode for each depth (AM_NONE or AM_EXPL)
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xReadFlag( uiCode );
    pcSPS->setAMVPMode( i, (AMVP_MODE)uiCode );
  }

#if REF_SETTING_FOR_LD
  // these syntax elements should not be sent at SPS when the full reference frame management is supported
  xReadFlag( uiCode ); pcSPS->setUseNewRefSetting( uiCode>0 ? true : false );
  if ( pcSPS->getUseNewRefSetting() )
  {
    xReadUvlc( uiCode );
    pcSPS->setMaxNumRefFrames( uiCode );
  }
#endif

  return;
}


Void TDecCavlc::parseSliceHeader (TComSlice*& rpcSlice)
{
  UInt  uiCode;
  Int   iCode;
  
#if ENC_DEC_TRACE
  xTraceSliceHeader(rpcSlice);
#endif
  
  // lightweight_slice_flag
  READ_FLAG( uiCode, "lightweight_slice_flag" );
  Bool bEntropySlice = uiCode ? true : false;

  
  // if( !lightweight_slice_flag ) {
  if (!bEntropySlice)
  {
    //   slice_type
    READ_UVLC (    uiCode, "slice_type" );            rpcSlice->setSliceType((SliceType)uiCode);
    READ_UVLC (    uiCode, "pic_parameter_set_id" );  rpcSlice->setPPSId(uiCode);
    //   frame_num
    //   if( IdrPicFlag )
    //     idr_pic_id
    //   if( pic_order_cnt_type  = =  0 )
    //     pic_order_cnt_lsb  
    READ_CODE (10, uiCode, "pic_order_cnt_lsb" );     rpcSlice->setPOC(uiCode);             // 9 == SPS->Log2MaxFrameNum()
    //   if( slice_type  = =  P  | |  slice_type  = =  B ) {
    //     num_ref_idx_active_override_flag
    //   if( num_ref_idx_active_override_flag ) {
    //     num_ref_idx_l0_active_minus1
    //     if( slice_type  = =  B )
    //       num_ref_idx_l1_active_minus1
    //   }
    if (!rpcSlice->isIntra())
    {
      READ_FLAG( uiCode, "num_ref_idx_active_override_flag");
      if (uiCode)
      {
        READ_CODE (3, uiCode, "num_ref_idx_l0_active_minus1" );  rpcSlice->setNumRefIdx( REF_PIC_LIST_0, uiCode + 1 );
        if (rpcSlice->isInterB())
        {
          READ_CODE (3, uiCode, "num_ref_idx_l1_active_minus1" );  rpcSlice->setNumRefIdx( REF_PIC_LIST_1, uiCode + 1 );
        }
        else
        {
          rpcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
        }
      }
      else
      {
        rpcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
        rpcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
      }
    }
    // }
  }
  // ref_pic_list_modification( )
  // ref_pic_list_combination( )
  if (rpcSlice->isInterB())
  {
    READ_FLAG( uiCode, "ref_pic_list_combination_flag" );       rpcSlice->setRefPicListCombinationFlag( uiCode ? 1 : 0 );
    if(uiCode)
    {
      READ_UVLC( uiCode, "num_ref_idx_lc_active_minus1" );      rpcSlice->setNumRefIdx( REF_PIC_LIST_C, uiCode + 1 );
      
      READ_FLAG( uiCode, "ref_pic_list_modification_flag_lc" ); rpcSlice->setRefPicListModificationFlagLC( uiCode ? 1 : 0 );
      if(uiCode)
      {
        for (UInt i=0;i<rpcSlice->getNumRefIdx(REF_PIC_LIST_C);i++)
        {
          READ_FLAG( uiCode, "pic_from_list_0_flag" );
          rpcSlice->setListIdFromIdxOfLC(i, uiCode);
          READ_UVLC( uiCode, "ref_idx_list_curr" );
          rpcSlice->setRefIdxFromIdxOfLC(i, uiCode);
          rpcSlice->setRefIdxOfLC((RefPicList)rpcSlice->getListIdFromIdxOfLC(i), rpcSlice->getRefIdxFromIdxOfLC(i), i);
        }
      }
    }
    else
    {
      rpcSlice->setRefPicListModificationFlagLC(false);
      rpcSlice->setNumRefIdx(REF_PIC_LIST_C, 0);
    }
  }
  else
  {
    rpcSlice->setRefPicListCombinationFlag(false);      
  }
  
  // if( nal_ref_idc != 0 )
  //   dec_ref_pic_marking( )
  // if( entropy_coding_mode_flag  &&  slice_type  !=  I)
  //   cabac_init_idc
  // first_slice_in_pic_flag
  // if( first_slice_in_pic_flag == 0 )
  //   slice_address
#if FINE_GRANULARITY_SLICES
  int iNumCUs = ((rpcSlice->getSPS()->getWidth()+rpcSlice->getSPS()->getMaxCUWidth()-1)/rpcSlice->getSPS()->getMaxCUWidth())*((rpcSlice->getSPS()->getHeight()+rpcSlice->getSPS()->getMaxCUHeight()-1)/rpcSlice->getSPS()->getMaxCUHeight());
  int iMaxParts = (1<<(rpcSlice->getSPS()->getMaxCUDepth()<<1));
  int iNumParts = (1<<(rpcSlice->getPPS()->getSliceGranularity()<<1));
  UInt uiLCUAddress = 0;
  int iReqBitsOuter = 0;
  while(iNumCUs>(1<<iReqBitsOuter))
  {
    iReqBitsOuter++;
  }
  int iReqBitsInner = 0;
  while((iNumParts)>(1<<iReqBitsInner)) 
  {
    iReqBitsInner++;
  }
  READ_FLAG( uiCode, "first_slice_in_pic_flag" );
  UInt uiAddress;
  UInt uiInnerAddress = 0;
  if(!uiCode)
  {
    READ_CODE( iReqBitsOuter+iReqBitsInner, uiAddress, "slice_address" );
    uiLCUAddress = uiAddress >> iReqBitsInner;
    uiInnerAddress = uiAddress - (uiLCUAddress<<iReqBitsInner);
  }
  //set uiCode to equal slice start address (or entropy slice start address)
  uiCode=(iMaxParts*uiLCUAddress)+(uiInnerAddress*(iMaxParts>>(rpcSlice->getPPS()->getSliceGranularity()<<1)));
  
  rpcSlice->setEntropySliceCurStartCUAddr( uiCode );
  rpcSlice->setEntropySliceCurEndCUAddr(iNumCUs*iMaxParts);
#endif
  if (bEntropySlice)
  {
    rpcSlice->setNextSlice        ( false );
    rpcSlice->setNextEntropySlice ( true  );
#if !FINE_GRANULARITY_SLICES
    READ_UVLC( uiCode, "slice_address" );
    rpcSlice->setEntropySliceCurStartCUAddr( uiCode ); // start CU addr for entropy slice
#endif
  }
  else
  {
    rpcSlice->setNextSlice        ( true  );
    rpcSlice->setNextEntropySlice ( false );
    
#if !FINE_GRANULARITY_SLICES
    READ_UVLC( uiCode, "slice_address" );
    rpcSlice->setSliceCurStartCUAddr( uiCode );        // start CU addr for slice
    rpcSlice->setEntropySliceCurStartCUAddr( uiCode ); // start CU addr for entropy slice  
#else
    rpcSlice->setSliceCurStartCUAddr(uiCode);
    rpcSlice->setSliceCurEndCUAddr(iNumCUs*iMaxParts);
#endif
    // if( !lightweight_slice_flag ) {
    //   slice_qp_delta
    // should be delta
    READ_SVLC( iCode, "slice_qp" );  rpcSlice->setSliceQp          (iCode);
   
    //   if( sample_adaptive_offset_enabled_flag )
    //     sao_param()
    //   if( deblocking_filter_control_present_flag ) {
    //     disable_deblocking_filter_idc
    // this should be an idc
    READ_FLAG ( uiCode, "loop_filter_disable" );  rpcSlice->setLoopFilterDisable(uiCode ? 1 : 0);
    //     if( disable_deblocking_filter_idc  !=  1 ) {
    //       slice_alpha_c0_offset_div2
    //       slice_beta_offset_div2
    //     }
    //   }
    //   if( slice_type = = B )
    //     collocated_from_l0_flag
#if AMVP_NEIGH_COL
    if ( rpcSlice->getSliceType() == B_SLICE )
    {
      READ_FLAG( uiCode, "collocated_from_l0_flag" );
      rpcSlice->setColDir(uiCode);
    }
#endif
    
    //   if( adaptive_loop_filter_enabled_flag ) {
    //     if( !shared_pps_info_enabled_flag )
    //       alf_param( )
    //     alf_cu_control_param( )
    //   }
    // }
  }
  // !!!! Syntax elements not in the WD  !!!!!
  
  if (!bEntropySlice)
  {
    xReadFlag ( uiCode ); rpcSlice->setSymbolMode( uiCode );
    
    xReadFlag (uiCode);   rpcSlice->setDRBFlag          (uiCode ? 1 : 0);
    if ( !rpcSlice->getDRBFlag() )
    {
      xReadCode(2, uiCode); rpcSlice->setERBIndex( (ERBIndex)uiCode );    assert (uiCode == ERB_NONE || uiCode == ERB_LTR);
    }      
  }
  
  return;
}

Void TDecCavlc::resetEntropy          (TComSlice* pcSlice)
{
  m_bRunLengthCoding = ! pcSlice->isIntra();
  m_uiRun = 0;
  
#if !CAVLC_COEF_LRG_BLK
  ::memcpy(m_uiLPTableD8,        g_auiLPTableD8,        10*128*sizeof(UInt));
#endif
  ::memcpy(m_uiLPTableD4,        g_auiLPTableD4,        3*32*sizeof(UInt));
  ::memcpy(m_uiLastPosVlcIndex,  g_auiLastPosVlcIndex,  10*sizeof(UInt));

#if CAVLC_RQT_CBP
  ::memcpy(m_uiCBP_YUV_TableD, g_auiCBP_YUV_TableD, 4*8*sizeof(UInt));
  ::memcpy(m_uiCBP_YS_TableD,  g_auiCBP_YS_TableD,  2*4*sizeof(UInt));
  ::memcpy(m_uiCBP_YCS_TableD, g_auiCBP_YCS_TableD, 2*8*sizeof(UInt));
  ::memcpy(m_uiCBP_4Y_TableD,  g_auiCBP_4Y_TableD,  2*15*sizeof(UInt));
  m_uiCBP_4Y_VlcIdx = 0;
#else
  m_uiCbpVlcIdx[0] = 0;
  m_uiCbpVlcIdx[1] = 0;
  ::memcpy(m_uiCBPTableD,        g_auiCBPTableD,        2*8*sizeof(UInt));
  ::memcpy(m_uiBlkCBPTableD,     g_auiBlkCBPTableD,     2*15*sizeof(UInt));
  m_uiBlkCbpVlcIdx = 0;
#endif

#if UNIFY_INTER_TABLE
  ::memcpy(m_uiMI1TableD, g_auiComMI1TableD, 9*sizeof(UInt));
#else  
  ::memcpy(m_uiMI1TableD,        g_auiMI1TableD,        8*sizeof(UInt));
  ::memcpy(m_uiMI2TableD,        g_auiMI2TableD,        15*sizeof(UInt));
  
  if ( pcSlice->getNoBackPredFlag() || pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0)
  {
    ::memcpy(m_uiMI1TableD,        g_auiMI1TableDNoL1,        8*sizeof(UInt));
    ::memcpy(m_uiMI2TableD,        g_auiMI2TableDNoL1,        15*sizeof(UInt));
  }
  
  if ( pcSlice->getNumRefIdx(REF_PIC_LIST_0) <= 1 && pcSlice->getNumRefIdx(REF_PIC_LIST_1) <= 1 )
  {
    if ( pcSlice->getNoBackPredFlag() || ( pcSlice->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcSlice->getNumRefIdx(REF_PIC_LIST_C) <= 1 ) )
    {
      ::memcpy(m_uiMI1TableD,        g_auiMI1TableDOnly1RefNoL1,        8*sizeof(UInt));
    }
    else
    {
      ::memcpy(m_uiMI1TableD,        g_auiMI1TableDOnly1Ref,        8*sizeof(UInt));
    }
  }
  if (pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0)
  {
    m_uiMI1TableD[8] = 8;
  }
  else  // GPB case
  {
    m_uiMI1TableD[8] = m_uiMI1TableD[6];
    m_uiMI1TableD[6] = 8;
  }
#endif
  
#if FIXED_MPM
  ::memcpy(m_uiIntraModeTableD17, g_auiIntraModeTableD17, 17*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableD34, g_auiIntraModeTableD34, 34*sizeof(UInt));
#elif MTK_DCM_MPM
  ::memcpy(m_uiIntraModeTableD17[0], g_auiIntraModeTableD17[0], 16*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableD34[0], g_auiIntraModeTableD34[0], 33*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableD17[1], g_auiIntraModeTableD17[1], 16*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableD34[1], g_auiIntraModeTableD34[1], 33*sizeof(UInt));
#else
  ::memcpy(m_uiIntraModeTableD17, g_auiIntraModeTableD17, 16*sizeof(UInt));
  ::memcpy(m_uiIntraModeTableD34, g_auiIntraModeTableD34, 33*sizeof(UInt));
#endif
#if AMP
  ::memcpy(m_uiSplitTableD, g_auiInterModeTableD, 4*11*sizeof(UInt));
#else
  ::memcpy(m_uiSplitTableD, g_auiInterModeTableD, 4*7*sizeof(UInt));
#endif
  m_uiMITableVlcIdx = 0;

#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
  ::memset(m_ucCBP_YUV_TableCounter, 0, 4*4*sizeof(UChar));
  ::memset(m_ucCBP_4Y_TableCounter,  0, 2*2*sizeof(UChar));
  ::memset(m_ucCBP_YCS_TableCounter, 0, 2*4*sizeof(UChar));
  ::memset(m_ucCBP_YS_TableCounter,  0, 2*3*sizeof(UChar));
#else
  ::memset(m_ucCBFTableCounter,        0,        2*4*sizeof(UChar));
  ::memset(m_ucBlkCBPTableCounter,     0,        2*2*sizeof(UChar));
#endif

  ::memset(m_ucMI1TableCounter,        0,          4*sizeof(UChar));
  ::memset(m_ucSplitTableCounter,      0,        4*4*sizeof(UChar));

#if CAVLC_RQT_CBP
  m_ucCBP_YUV_TableCounterSum[0] = m_ucCBP_YUV_TableCounterSum[1] = m_ucCBP_YUV_TableCounterSum[2] = m_ucCBP_YUV_TableCounterSum[3] = 0;
  m_ucCBP_4Y_TableCounterSum[0] = m_ucCBP_4Y_TableCounterSum[1] = 0;
  m_ucCBP_YCS_TableCounterSum[0] = m_ucCBP_YCS_TableCounterSum[1] = 0;
  m_ucCBP_YS_TableCounterSum[0] = m_ucCBP_YS_TableCounterSum[1] = 0;
#else
  m_ucCBFTableCounterSum[0] = m_ucCBFTableCounterSum[1] = 0;
  m_ucBlkCBPTableCounterSum[0] = m_ucBlkCBPTableCounterSum[1] = 0;
#endif

  m_ucSplitTableCounterSum[0] = m_ucSplitTableCounterSum[1] = m_ucSplitTableCounterSum[2]= m_ucSplitTableCounterSum[3] = 0;
  m_ucMI1TableCounterSum = 0;
#endif
}

Void TDecCavlc::parseTerminatingBit( UInt& ruiBit )
{
  ruiBit = false;
  Int iBitsLeft = m_pcBitstream->getNumBitsLeft();
  if(iBitsLeft <= 8)
  {
    UInt uiPeekValue = m_pcBitstream->peekBits(iBitsLeft);
    if (uiPeekValue == (1<<(iBitsLeft-1)))
      ruiBit = true;
  }
}

Void TDecCavlc::parseAlfCtrlDepth              ( UInt& ruiAlfCtrlDepth )
{
  UInt uiSymbol;
  xReadUnaryMaxSymbol(uiSymbol, g_uiMaxCUDepth-1);
  ruiAlfCtrlDepth = uiSymbol;
}

Void TDecCavlc::parseAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if (!m_bAlfCtrl)
    return;
  
  if( uiDepth > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }
  
  UInt uiSymbol;
  xReadFlag( uiSymbol );
  
  if (uiDepth > m_uiMaxAlfCtrlDepth)
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, m_uiMaxAlfCtrlDepth);
  }
  else
  {
    pcCU->setAlfCtrlFlagSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  }
}

Void TDecCavlc::parseSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  return;
}

/** parse the motion vector predictor index
 * \param pcCU
 * \param riMVPIdx 
 * \param iMVPNum
 * \param uiAbsPartIdx
 * \param uiDepth
 * \param eRefList
 * \returns Void
 */
Void TDecCavlc::parseMVPIdx( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
#if MRG_AMVP_FIXED_IDX_F470
  xReadUnaryMaxSymbol(uiSymbol, AMVP_MAX_NUM_CANDS-1);
#else
  xReadUnaryMaxSymbol(uiSymbol, iMVPNum-1);
#endif
  riMVPIdx = uiSymbol;
}

/** parse the split flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecCavlc::parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if (pcCU->getSlice()->isIntra())
  {
    if( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    {
      pcCU->setDepthSubParts( uiDepth, uiAbsPartIdx );
      return ;
    }

    UInt uiSymbol;
    xReadFlag( uiSymbol );
    pcCU->setDepthSubParts( uiDepth + uiSymbol, uiAbsPartIdx );

    return ;
  }

  UInt tmp=0;
  UInt cx=0;
  UInt uiMode ;
  {
#if AMP
    UInt iMaxLen= (uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth)? 9:10;
#else
    UInt iMaxLen= (uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth)? 5:6;
#endif

    while (tmp==0 && cx<iMaxLen)
    {
      xReadFlag( tmp );
      cx++;
    };
    if(tmp!=0)
      cx--;

    UInt uiDepthRemember = uiDepth;
    if ( uiDepth == g_uiMaxCUDepth - g_uiAddCUDepth )
    {
      uiDepth = 3;
    }
    UInt x = m_uiSplitTableD[uiDepth][cx];
    /* Adapt table */
    uiMode = x;
#if CAVLC_COUNTER_ADAPT
    adaptCodeword(cx, m_ucSplitTableCounter[uiDepth],  m_ucSplitTableCounterSum[uiDepth],   m_uiSplitTableD[uiDepth],  NULL, 4 );
#else
    if (cx>0)
    {    
      UInt cy = max(0,cx-1);
      UInt y = m_uiSplitTableD[uiDepth][cy];
      m_uiSplitTableD[uiDepth][cy] = x;
      m_uiSplitTableD[uiDepth][cx] = y;
    }
#endif
    uiDepth = uiDepthRemember;
  }

  if (uiMode==0)
  {
    pcCU->setDepthSubParts( uiDepth + 1, uiAbsPartIdx );
  }
  else
  {
    pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setMergeFlagSubParts(false, uiAbsPartIdx,0, uiDepth );
    pcCU->setDepthSubParts( uiDepth    , uiAbsPartIdx );
    if (uiMode ==1)
    {
      TComMv cZeroMv(0,0);
      pcCU->setPredModeSubParts( MODE_SKIP,  uiAbsPartIdx, uiDepth );
      pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
      pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd    ( cZeroMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      pcCU->setTrIdxSubParts( 0, uiAbsPartIdx, uiDepth );
      pcCU->setCbfSubParts  ( 0, 0, 0, uiAbsPartIdx, uiDepth );

#if HHI_MRG_SKIP
      if ( pcCU->getSlice()->getSPS()->getUseMRG() )
      {
      pcCU->setMergeFlagSubParts( true, uiAbsPartIdx, 0, uiDepth );
      } 
      else
#endif
      {
        if ( pcCU->getSlice()->isInterP() )
        {
          pcCU->setInterDirSubParts( 1, uiAbsPartIdx, 0, uiDepth );
          
          if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
            pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
          if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
            pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( NOT_VALID, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        }
        else
        {
          pcCU->setInterDirSubParts( 3, uiAbsPartIdx, 0, uiDepth );
          
          if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
            pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllRefIdx(  0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
          if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
            pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllRefIdx( 0, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        }
      }
    }
    else if (uiMode==2)
    {
      pcCU->setPredModeSubParts( MODE_INTER, uiAbsPartIdx, uiDepth );
      pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      pcCU->setMergeFlagSubParts(true, uiAbsPartIdx,0, uiDepth );
      pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    }
    else if (uiMode==6)
    {
      if (uiDepth != g_uiMaxCUDepth - g_uiAddCUDepth)
      {
        pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
        pcCU->setPartSizeSubParts( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
        UInt uiTrLevel = 0;
        UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)]+2;
        UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()]+2;
        uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;
        pcCU->setTrIdxSubParts( uiTrLevel, uiAbsPartIdx, uiDepth );       
      }
      else
      {
        pcCU->setPredModeSubParts( MODE_INTER, uiAbsPartIdx, uiDepth );
        pcCU->setPartSizeSubParts( SIZE_NxN, uiAbsPartIdx, uiDepth );
      }
    }
    else
    {
      pcCU->setPredModeSubParts( MODE_INTER, uiAbsPartIdx, uiDepth );
      pcCU->setPartSizeSubParts( PartSize(uiMode-3), uiAbsPartIdx, uiDepth );
      pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    }
  }
}

/** parse partition size
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecCavlc::parsePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{

  UInt uiMode=0;

  if ( pcCU->getSlice()->isIntra() && pcCU->isIntra( uiAbsPartIdx ) )
  {
    uiMode = 1;
    if ( uiDepth == (g_uiMaxCUDepth - g_uiAddCUDepth ))
    {
      UInt uiSymbol;
      xReadFlag( uiSymbol );
      uiMode = uiSymbol ? 1 : 2;
    }
  }
  else if (uiDepth != (g_uiMaxCUDepth - g_uiAddCUDepth ) || pcCU->getPartitionSize(uiAbsPartIdx ) != SIZE_NxN)
  { 
    return;
  }
  else
  {
    UInt uiSymbol;
    xReadFlag( uiSymbol );
    if(uiSymbol)
    {
      uiMode = 1;
    }
    else
    {
#if DISABLE_4x4_INTER
      if(pcCU->getSlice()->getSPS()->getDisInter4x4() && ( (g_uiMaxCUWidth>>uiDepth)==8) && ( (g_uiMaxCUHeight>>uiDepth)==8) )
      {
        uiMode = 2;
      }
      else
      {
#endif
      xReadFlag( uiSymbol );
      uiMode = uiSymbol ? 2 : 0;
#if DISABLE_4x4_INTER
      }
#endif
    }
  }
  PartSize ePartSize;
  PredMode eMode;
  if (uiMode > 0)
  {
    eMode = MODE_INTRA;
    ePartSize = (uiMode==1) ? SIZE_2Nx2N:SIZE_NxN;
  }
  else
  {
    eMode = MODE_INTER;
    ePartSize = SIZE_NxN;
  }
  pcCU->setPredModeSubParts( eMode    , uiAbsPartIdx, uiDepth );
  pcCU->setPartSizeSubParts( ePartSize, uiAbsPartIdx, uiDepth );
  pcCU->setSizeSubParts( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );

  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
  {
    UInt uiTrLevel = 0;
    UInt uiWidthInBit  = g_aucConvertToBit[pcCU->getWidth(uiAbsPartIdx)] + 2;
    UInt uiTrSizeInBit = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxTrSize()] + 2;
    uiTrLevel          = uiWidthInBit >= uiTrSizeInBit ? uiWidthInBit - uiTrSizeInBit : 0;
    if( pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_NxN )
    {
      pcCU->setTrIdxSubParts( 1 + uiTrLevel, uiAbsPartIdx, uiDepth );
    }
    else
    {
      pcCU->setTrIdxSubParts( uiTrLevel, uiAbsPartIdx, uiDepth );
    }
  }
}

/** parse prediction mode
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecCavlc::parsePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if( pcCU->getSlice()->isIntra() )
  {
    pcCU->setPredModeSubParts( MODE_INTRA, uiAbsPartIdx, uiDepth );
    return ;
  }
}

#if E057_INTRA_PCM
/** Parse I_PCM information. 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \param uiDepth CU depth
 * \returns Void
 *
 * If I_PCM flag indicates that the CU is I_PCM, parse its PCM alignment bits and codes.  
 */
Void TDecCavlc::parseIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

  xReadFlag( uiSymbol );

  if ( uiSymbol )
  {
    Bool bIpcmFlag   = true;

    xReadPCMAlignZero();

    pcCU->setPartSizeSubParts  ( SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
    pcCU->setSizeSubParts      ( g_uiMaxCUWidth>>uiDepth, g_uiMaxCUHeight>>uiDepth, uiAbsPartIdx, uiDepth );
    pcCU->setIPCMFlagSubParts  ( bIpcmFlag, uiAbsPartIdx, uiDepth );

    UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
    UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
    UInt uiChromaOffset = uiLumaOffset>>2;

    Pel* piPCMSample;
    UInt uiWidth;
    UInt uiHeight;
    UInt uiSampleBits;
    UInt uiX, uiY;

    piPCMSample = pcCU->getPCMSampleY() + uiLumaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx);
    uiHeight = pcCU->getHeight(uiAbsPartIdx);
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample;
        xReadCode(uiSampleBits, uiSample);

        piPCMSample[uiX] = uiSample;
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCb() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample;
        xReadCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCr() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#else
    uiSampleBits = g_uiBitDepth;
#endif

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample;
        xReadCode(uiSampleBits, uiSample);
        piPCMSample[uiX] = uiSample;
      }
      piPCMSample += uiWidth;
    }
  }
}
#endif

#if MTK_DCM_MPM
Void TDecCavlc::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{ 
  Int  uiIPredMode = 0;
  Int  iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);

  Int uiPreds[2] = {-1, -1};
  Int uiPredNum = pcCU->getIntraDirLumaPredictor(uiAbsPartIdx, uiPreds);  

  if ( g_aucIntraModeBitsAng[iIntraIdx] < 5 )
  {
     UInt uiSymbol;
     xReadFlag( uiSymbol );
     if ( uiSymbol )
     {
#if FIXED_MPM
       xReadFlag( uiSymbol );
       uiIPredMode = uiPreds[uiSymbol];
#else
       if(uiPredNum == 1)
       {
        uiIPredMode = uiPreds[0];
       }
       else
       {
        xReadFlag( uiSymbol );
        uiIPredMode = uiPreds[uiSymbol];
       }
#endif
     }
     else
     {
      xReadFlag( uiSymbol ); uiIPredMode  = uiSymbol;
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) { xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 1; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) { xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 2; }

      for(UInt i = 0; i < uiPredNum; i++)
      {
        if(uiIPredMode >= uiPreds[i]) { uiIPredMode ++;}
      }
    }
  }
  else 
  {
    Int  iDir, iDirLarger, iRankIntraMode, iRankIntraModeLarger;

    const UInt *huff;
    const UInt *lengthHuff;
    UInt  totMode;
    UInt  *m_uiIntraModeTableD;

#if FIXED_MPM
    if ( g_aucIntraModeBitsAng[iIntraIdx] == 5 )
    {
      totMode = 17;
      huff = huff17_2;
      lengthHuff = lengthHuff17_2;
      m_uiIntraModeTableD = m_uiIntraModeTableD17;
    }
    else
    {
      totMode = 34;
      huff = huff34_2;
      lengthHuff = lengthHuff34_2;
      m_uiIntraModeTableD = m_uiIntraModeTableD34;
    }

    UInt uiCode;
    UInt uiLength = lengthHuff[totMode - 1];  

    m_pcBitstream->pseudoRead( uiLength, uiCode );

    if ( ( uiCode >> ( uiLength - lengthHuff[0] ) ) == huff[0] )
    {
      m_pcBitstream->read( lengthHuff[0], uiCode );

      UInt uiPredIdx= 0;
      xReadFlag( uiPredIdx );
      uiIPredMode = uiPreds[ uiPredIdx ];
    }
    else
    {
      iRankIntraMode = 0;

      for(Int i = 1; i < totMode; i++) 
      {  
        if( (uiCode >> ( uiLength - lengthHuff[i]) ) == huff[i] )
        {
          m_pcBitstream->read( lengthHuff[i], uiCode );
          iRankIntraMode = i;
          break;
        }
      }

      iRankIntraMode --;
      iDir = m_uiIntraModeTableD[ iRankIntraMode ];
      iRankIntraModeLarger = max( 0, iRankIntraMode - 1 );
      iDirLarger = m_uiIntraModeTableD[ iRankIntraModeLarger ];
      m_uiIntraModeTableD[ iRankIntraModeLarger ] = iDir;
      m_uiIntraModeTableD[ iRankIntraMode ] = iDirLarger;

      for(UInt i = 0; i < uiPredNum; i++)
      {
        if(iDir >= uiPreds[i]) 
        {
          iDir ++;
        }
      }

      uiIPredMode = iDir;
    }
#else
  if ( g_aucIntraModeBitsAng[iIntraIdx] == 5 )
  {
  totMode = (uiPredNum == 1)? 17: 16;
  huff = huff17_2[uiPredNum - 1];
  lengthHuff = lengthHuff17_2[uiPredNum - 1];
  m_uiIntraModeTableD = m_uiIntraModeTableD17[uiPredNum - 1];
  }
  else
  {
  totMode = (uiPredNum == 1)? 34: 33;
  huff = huff34_2[uiPredNum - 1];
  lengthHuff = lengthHuff34_2[uiPredNum - 1];
  m_uiIntraModeTableD = m_uiIntraModeTableD34[uiPredNum - 1];
  }
 
  UInt uiCode;
  UInt uiLength = lengthHuff[totMode - 1];  

  m_pcBitstream->pseudoRead(uiLength,uiCode);

  if ((uiCode>>(uiLength- lengthHuff[0])) == huff[0])
  {
      m_pcBitstream->read(lengthHuff[0],uiCode);

     if(uiPredNum == 1)
     {
        uiIPredMode = uiPreds[0];
     }
     else if(uiPredNum == 2)
     {           
        UInt uiPredIdx= 0;
        xReadFlag( uiPredIdx );
        uiIPredMode = uiPreds[uiPredIdx];
     }
  }
  else
  {
      iRankIntraMode = 0;

      for(Int i = 1; i < totMode; i++) 
     {  
        if( (uiCode>>(uiLength- lengthHuff[i])) == huff[i])
        {
          m_pcBitstream->read(lengthHuff[i], uiCode);
          iRankIntraMode = i;
          break;
        }
      }

     iRankIntraMode --;
     iDir = m_uiIntraModeTableD[iRankIntraMode];
     iRankIntraModeLarger = max(0,iRankIntraMode-1);
     iDirLarger = m_uiIntraModeTableD[iRankIntraModeLarger];
     m_uiIntraModeTableD[iRankIntraModeLarger] = iDir;
     m_uiIntraModeTableD[iRankIntraMode] = iDirLarger;

     for(UInt i = 0; i < uiPredNum; i++)
     {
       if(iDir >= uiPreds[i]) 
       {
         iDir ++;
       }
     }

     uiIPredMode = iDir;
      

    }
#endif
  }
#if ADD_PLANAR_MODE && !FIXED_MPM
  if (uiIPredMode == 2)
  {
    UInt planarFlag;
    xReadFlag( planarFlag );
    if ( planarFlag )
    {
      uiIPredMode = PLANAR_IDX;
    }
  }
#endif
 
  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );     
}
#else
Void TDecCavlc::parseIntraDirLumaAng  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;
  Int  uiIPredMode;
  Int  iMostProbable = pcCU->getMostProbableIntraDirLuma( uiAbsPartIdx );
  Int  iIntraIdx = pcCU->getIntraSizeIdx(uiAbsPartIdx);
  Int  iDir, iDirLarger, iRankIntraMode, iRankIntraModeLarger;

  Int  iLeft          = pcCU->getLeftIntraDirLuma( uiAbsPartIdx );
  Int  iAbove         = pcCU->getAboveIntraDirLuma( uiAbsPartIdx );
  UInt ind=(iLeft==iAbove)? 0 : 1;

  const UInt *huff17=huff17_2[ind];
  const UInt *lengthHuff17=lengthHuff17_2[ind];
  const UInt *huff34=huff34_2[ind];
  const UInt *lengthHuff34=lengthHuff34_2[ind];

  if ( g_aucIntraModeBitsAng[iIntraIdx] < 5 )
  {
    xReadFlag( uiSymbol );
    if ( uiSymbol )
      uiIPredMode = iMostProbable;
    else
    {
      xReadFlag( uiSymbol ); uiIPredMode  = uiSymbol;
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 2 ) { xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 1; }
      if ( g_aucIntraModeBitsAng[iIntraIdx] > 3 ) { xReadFlag( uiSymbol ); uiIPredMode |= uiSymbol << 2; }
      if(uiIPredMode >= iMostProbable) 
        uiIPredMode ++;
    }
  }
  else if ( g_aucIntraModeBitsAng[iIntraIdx] == 5 )
  {
    UInt uiCode;
    UInt uiLength = lengthHuff17[15];
    m_pcBitstream->pseudoRead(uiLength,uiCode);
    if ((uiCode>>(uiLength- lengthHuff17[0])) == huff17[0])
    {
      m_pcBitstream->read(lengthHuff17[0],uiCode);
      uiIPredMode = iMostProbable;
    }
    else
    {
      iRankIntraMode = 0;
      for (Int i=1;i<17;i++)
      {
        if( (uiCode>>(uiLength- lengthHuff17[i])) == huff17[i])
        {
          m_pcBitstream->read(lengthHuff17[i], uiCode);
          iRankIntraMode = i;
          break;
        }
      }
      
      if ( iRankIntraMode > 0 )
        iRankIntraMode --;
      iDir = m_uiIntraModeTableD17[iRankIntraMode];
      
      iRankIntraModeLarger = max(0,iRankIntraMode-1);
      iDirLarger = m_uiIntraModeTableD17[iRankIntraModeLarger];
      
      m_uiIntraModeTableD17[iRankIntraModeLarger] = iDir;
      m_uiIntraModeTableD17[iRankIntraMode] = iDirLarger;
      
      uiIPredMode = (iDir>=iMostProbable? iDir+1: iDir);
    }
  }
  else
  {
    UInt uiCode;
    UInt uiLength = lengthHuff34[32];
    m_pcBitstream->pseudoRead(uiLength,uiCode);
    if ((uiCode>>(uiLength- lengthHuff34[0])) == huff34[0])
    {
      m_pcBitstream->read(lengthHuff34[0],uiCode);
      uiIPredMode = iMostProbable;
    }
    else
    {
      iRankIntraMode = 0;
      for (Int i=1;i<34;i++)
      {
        if( (uiCode>>(uiLength- lengthHuff34[i])) == huff34[i])
        {
          m_pcBitstream->read(lengthHuff34[i], uiCode);
          iRankIntraMode = i;
          break;
        }
      }
      
      if ( iRankIntraMode > 0 )
        iRankIntraMode --;
      iDir = m_uiIntraModeTableD34[iRankIntraMode];
      
      iRankIntraModeLarger = max(0,iRankIntraMode-1);
      iDirLarger = m_uiIntraModeTableD34[iRankIntraModeLarger];
      
      m_uiIntraModeTableD34[iRankIntraModeLarger] = iDir;
      m_uiIntraModeTableD34[iRankIntraMode] = iDirLarger;
      
      uiIPredMode = (iDir>=iMostProbable? iDir+1: iDir);
    }
  }
  
#if ADD_PLANAR_MODE
  if (uiIPredMode == 2)
  {
    UInt planarFlag;
    xReadFlag( planarFlag );
    if ( planarFlag )
    {
      uiIPredMode = PLANAR_IDX;
    }
  }
#endif
  pcCU->setLumaIntraDirSubParts( (UChar)uiIPredMode, uiAbsPartIdx, uiDepth );     
}
#endif

Void TDecCavlc::parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if FIXED_MPM
  UInt uiSymbol;
  xReadFlag( uiSymbol );

  if( uiSymbol == 0 )
  {
    uiSymbol = DM_CHROMA_IDX;
  } 
  else 
  {
    if( pcCU->getSlice()->getSPS()->getUseLMChroma() )
    {
      xReadFlag( uiSymbol );
    }
    else
    {
      uiSymbol = 1;
    }

    if( uiSymbol == 0 )
    {
      uiSymbol = LM_CHROMA_IDX;
    } 
    else
    {
      xReadUnaryMaxSymbol( uiSymbol, 3 );
#if CHROMA_CODEWORD_SWITCH 
      uiSymbol = ChromaMapping[uiSymbol];
#endif
      UInt uiAllowedChromaDir[ NUM_CHROMA_MODE ];
      pcCU->getAllowedChromaDir( uiAbsPartIdx, uiAllowedChromaDir );
      uiSymbol = uiAllowedChromaDir[ uiSymbol ];
    }
  }
  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  return;
#else
  UInt uiSymbol;
  UInt uiMode = pcCU->getLumaIntraDir(uiAbsPartIdx);
#if ADD_PLANAR_MODE
  if ( (uiMode == 2 ) || (uiMode == PLANAR_IDX) )
  {
    uiMode = 4;
  }
#endif
#if LM_CHROMA
  Int  iMaxMode = pcCU->getSlice()->getSPS()->getUseLMChroma() ? 3 : 4;

  Int  iMax = uiMode < iMaxMode ? 3 : 4; 

  xReadUnaryMaxSymbol( uiSymbol, iMax );
  
  //switch codeword
  if (uiSymbol == 0)
  {
    uiSymbol = 4;
  }
  else if (uiSymbol == 1 && pcCU->getSlice()->getSPS()->getUseLMChroma())
  {
    uiSymbol = LM_CHROMA_IDX;
  }
  else
  {
#if CHROMA_CODEWORD_SWITCH
    uiSymbol = ChromaMapping[iMax-3][uiSymbol];
#endif
    
    if (pcCU->getSlice()->getSPS()->getUseLMChroma())
       uiSymbol --;

    if (uiSymbol <= uiMode)
       uiSymbol --;
  }

#else // -- LM_CHROMA

  Int  iMax = uiMode < 4 ? 3 : 4;
  xReadUnaryMaxSymbol( uiSymbol, iMax );
  
  //switch codeword
  if (uiSymbol == 0)
  {
    uiSymbol = 4;
  }
  else
  {
#if CHROMA_CODEWORD_SWITCH
    uiSymbol = ChromaMapping[iMax-3][uiSymbol];
#endif
    if (uiSymbol <= uiMode)
    {
      uiSymbol --;
    }
  }
#endif // --> LM_CHROMA

  //printf("uiMode %d, chroma %d, codeword %d, imax %d\n", uiMode, uiSymbol, uiRead, iMax);

#if ADD_PLANAR_MODE
  if (uiSymbol == 2)
  {
    uiMode = pcCU->getLumaIntraDir(uiAbsPartIdx);
    if (uiMode == 2)
    {
      uiSymbol = PLANAR_IDX;
    }
    else if (uiMode != PLANAR_IDX)
    {
      UInt planarFlag;
      xReadFlag( planarFlag );
      if ( planarFlag )
      {
        uiSymbol = PLANAR_IDX;
      }
    }
  }
#endif
  pcCU->setChromIntraDirSubParts( uiSymbol, uiAbsPartIdx, uiDepth );
  
  return ;
#endif
}

Void TDecCavlc::parseInterDir( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiSymbol;

#if UNIFY_INTER_TABLE
  UInt uiNumRefIdxOfLC = pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C);
  UInt uiValNumRefIdxOfLC = min(4,pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C));
  UInt uiValNumRefIdxOfL0 = min(2,pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_0));
  UInt uiValNumRefIdxOfL1 = min(2,pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_1));

  if ( pcCU->getSlice()->getRefIdxCombineCoding() )
  {
    UInt uiIndex,tmp;
#if CAVLC_COUNTER_ADAPT
    Int x,cx;
#else
    Int x,cx,y,cy;
#endif
    
    UInt *m_uiMITableD = m_uiMI1TableD;

    UInt uiMaxVal;
    if (uiNumRefIdxOfLC > 0)
    {
      uiMaxVal = uiValNumRefIdxOfLC + uiValNumRefIdxOfL0*uiValNumRefIdxOfL1;
    }
    else if (pcCU->getSlice()->getNoBackPredFlag())
    {
      uiMaxVal = uiValNumRefIdxOfL0 + uiValNumRefIdxOfL0*uiValNumRefIdxOfL1;
    }
    else
    {
      uiMaxVal = uiValNumRefIdxOfL0 + uiValNumRefIdxOfL1 + uiValNumRefIdxOfL0*uiValNumRefIdxOfL1;
    }

#if CAVLC_UNIFY_INTER_TABLE_FIX
    Bool bCodeException = false;
    if ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 4 ||
         pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_0) > 2 ||
         pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_1) > 2 )
    {
      bCodeException = true;
    }
    else
    {
      bCodeException = false;
      uiMaxVal--;
    }
#endif

    xReadUnaryMaxSymbol( tmp, uiMaxVal );

    x = m_uiMITableD[tmp];
    uiIndex = x;
    
    /* Adapt table */
    
    cx = tmp;
#if CAVLC_COUNTER_ADAPT
    adaptCodeword(cx, m_ucMI1TableCounter,  m_ucMI1TableCounterSum,  m_uiMITableD,  NULL, 4 );
#else
    cy = max(0,cx-1);  
    y = m_uiMITableD[cy];
    m_uiMITableD[cy] = x;
    m_uiMITableD[cx] = y;
    m_uiMITableVlcIdx += cx == m_uiMITableVlcIdx ? 0 : (cx < m_uiMITableVlcIdx ? -1 : 1);
#endif
    
#if CAVLC_UNIFY_INTER_TABLE_FIX
    if (uiIndex < uiMaxVal || !bCodeException )
#else
    if (uiIndex < uiMaxVal)
#endif
    {
      if (uiNumRefIdxOfLC > 0)
      {
        if (uiIndex < uiValNumRefIdxOfLC)
        {
          ruiInterDir = 1;
          m_iRefFrame0[uiAbsPartIdx] = uiIndex;
        }
        else
        {
          UInt uiTmp = uiIndex-uiValNumRefIdxOfLC;
          ruiInterDir = 3;

          m_iRefFrame0[uiAbsPartIdx] = uiTmp/uiValNumRefIdxOfL1; //uiValNumRefIdxOfL1 == 1 or 2, so division can be converted to shift op
          m_iRefFrame1[uiAbsPartIdx] = uiTmp%uiValNumRefIdxOfL1;          
        }
      }
      else if (pcCU->getSlice()->getNoBackPredFlag())
      {
        if (uiIndex < uiValNumRefIdxOfL0)
        {
          ruiInterDir = 1;
          m_iRefFrame0[uiAbsPartIdx] = uiIndex;
        }
        else
        {
          UInt uiTmp = uiIndex-uiValNumRefIdxOfL0;
          ruiInterDir = 3;

          m_iRefFrame0[uiAbsPartIdx] = uiTmp/uiValNumRefIdxOfL1; //uiValNumRefIdxOfL1 == 1 or 2, so division can be converted to shift op
          m_iRefFrame1[uiAbsPartIdx] = uiTmp%uiValNumRefIdxOfL1;          
        }
      }
      else
      {
        if (uiIndex < uiValNumRefIdxOfL0)
        {
          ruiInterDir = 1;
          m_iRefFrame0[uiAbsPartIdx] = uiIndex;
        }
        else if (uiIndex < uiValNumRefIdxOfL1)
        {
          ruiInterDir = 2;
          m_iRefFrame1[uiAbsPartIdx] = uiIndex-uiValNumRefIdxOfL0;
        }
        else
        {
          UInt uiTmp = uiIndex-uiValNumRefIdxOfL0-uiValNumRefIdxOfL1;
          ruiInterDir = 3;

          m_iRefFrame0[uiAbsPartIdx] = uiTmp/uiValNumRefIdxOfL1; //uiValNumRefIdxOfL1 == 1 or 2, so division can be converted to shift op
          m_iRefFrame1[uiAbsPartIdx] = uiTmp%uiValNumRefIdxOfL1;          
        }
      }

      return;
    }
  }
#else  //UNIFY_INTER_TABLE  
  if ( pcCU->getSlice()->getRefIdxCombineCoding() )
  {
    UInt uiIndex,uiInterDir,tmp;

#if CAVLC_COUNTER_ADAPT
    Int x,cx;
#else
    Int x,cx,y,cy;
#endif

    UInt uiMaxVal = 7;
    uiMaxVal = 8;
    if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 1 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 1 )
    {
      if ( pcCU->getSlice()->getNoBackPredFlag() || ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 1 ) )
      {
        uiMaxVal = 1;
      }
      else
      {
        uiMaxVal = 2;
      }
    }
    else if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2 )
    {
      if ( pcCU->getSlice()->getNoBackPredFlag() || ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 2 ) )
      {
        uiMaxVal = 5;
      }
      else
      {
        uiMaxVal = 7;
      }
    }
    else if ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) <= 0 )
    {
      uiMaxVal = 4+1+MS_LCEC_UNI_EXCEPTION_THRES;
    }
    
    xReadUnaryMaxSymbol( tmp, uiMaxVal );

    UInt *m_uiMITableD = m_uiMI1TableD;
    x = m_uiMITableD[tmp];
    uiIndex = x;
    
    /* Adapt table */
    
    cx = tmp;

#if CAVLC_COUNTER_ADAPT
    adaptCodeword(cx, m_ucMI1TableCounter,  m_ucMI1TableCounterSum,  m_uiMITableD,  NULL, 4 );
#else
    cy = max(0,cx-1);
    y = m_uiMITableD[cy];
    m_uiMITableD[cy] = x;
    m_uiMITableD[cx] = y;
    m_uiMITableVlcIdx += cx == m_uiMITableVlcIdx ? 0 : (cx < m_uiMITableVlcIdx ? -1 : 1);
#endif
    {
      uiInterDir = min(2,uiIndex>>1);  
      if ( uiIndex >=4 )
      {
        uiInterDir = 2;
      }
      else
      {
        uiInterDir = 0;
      }
      if(uiInterDir!=2 && pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C)>0)
      {
        uiInterDir = 0;
        m_iRefFrame0[uiAbsPartIdx] = uiIndex;
      }
      else if (uiInterDir==0)
      {
        m_iRefFrame0[uiAbsPartIdx] = uiIndex;
      }
      else if (uiInterDir==1)
        m_iRefFrame1[uiAbsPartIdx] = uiIndex&1;
      else
      {
        m_iRefFrame0[uiAbsPartIdx] = (uiIndex>>1)&1;
        m_iRefFrame1[uiAbsPartIdx] = (uiIndex>>0)&1;
      }
    }
    ruiInterDir = uiInterDir+1;
    if ( x < 8 )
    {
      return;
    }
  }
#endif //UNIFY_INTER_TABLE
  
  m_iRefFrame0[uiAbsPartIdx] = 1000;
  m_iRefFrame1[uiAbsPartIdx] = 1000;
  
  xReadFlag( uiSymbol );
  
  if ( uiSymbol )
  {
    uiSymbol = 2;
  }
  else if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0)
  {
    uiSymbol = 0;
  }
  else if ( pcCU->getSlice()->getNoBackPredFlag() )
  {
    uiSymbol = 0;
  }
  else
  {
    xReadFlag( uiSymbol );
  }
  uiSymbol++;
  ruiInterDir = uiSymbol;
  return;
}

Void TDecCavlc::parseRefFrmIdx( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList )
{
  UInt uiSymbol;
  
  if (pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) <= 2 && pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) <= 2 && pcCU->getSlice()->isInterB())
  {
    if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C ) > 0 && eRefList==REF_PIC_LIST_C)
    {
      riRefFrmIdx = m_iRefFrame0[uiAbsPartIdx]; 
    }
    else if (eRefList==REF_PIC_LIST_0)
    {
      riRefFrmIdx = m_iRefFrame0[uiAbsPartIdx];      
    }
    if (eRefList==REF_PIC_LIST_1)
    {
      riRefFrmIdx = m_iRefFrame1[uiAbsPartIdx];
    }
    return;
  }    
  
  if ( ( m_iRefFrame0[uiAbsPartIdx] != 1000 || m_iRefFrame1[uiAbsPartIdx] != 1000 ) &&
      pcCU->getSlice()->getRefIdxCombineCoding() )
  {
    if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C ) > 0 && eRefList==REF_PIC_LIST_C )
    {
      riRefFrmIdx = m_iRefFrame0[uiAbsPartIdx]; 
    }
    else if (eRefList==REF_PIC_LIST_0)
    {
      riRefFrmIdx = m_iRefFrame0[uiAbsPartIdx];      
    }
    else if (eRefList==REF_PIC_LIST_1)
    {
      riRefFrmIdx = m_iRefFrame1[uiAbsPartIdx];
    }
    return;
  }
  
  UInt uiRefFrmIdxMinus = 0;
  if ( pcCU->getSlice()->getRefIdxCombineCoding() )
  {
    if ( pcCU->getInterDir( uiAbsPartIdx ) != 3 )
    {
      if ( pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 )
      {
        uiRefFrmIdxMinus = 4;
      }
      else
      {
        uiRefFrmIdxMinus = MS_LCEC_UNI_EXCEPTION_THRES+1;
      }
    }
    else if ( eRefList == REF_PIC_LIST_1 && pcCU->getCUMvField( REF_PIC_LIST_0 )->getRefIdx( uiAbsPartIdx ) < 2 )
    {
      uiRefFrmIdxMinus = 2;
    }
  }
  if ( pcCU->getSlice()->getNumRefIdx( eRefList ) - uiRefFrmIdxMinus <= 1 )
  {
    uiSymbol = 0;
    riRefFrmIdx = uiSymbol;
    riRefFrmIdx += uiRefFrmIdxMinus;
    return;
  }
  
  xReadFlag ( uiSymbol );
  if ( uiSymbol )
  {
    xReadUnaryMaxSymbol( uiSymbol, pcCU->getSlice()->getNumRefIdx( eRefList )-2 - uiRefFrmIdxMinus );
    
    uiSymbol++;
  }
  riRefFrmIdx = uiSymbol;
  riRefFrmIdx += uiRefFrmIdxMinus;
  
  return;
}

Void TDecCavlc::parseMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList )
{
  Int iHor, iVer;
  
  TComMv cTmpMv( 0, 0 );
#if AMP
  pcCU->getCUMvField( eRefList )->setAllMv( cTmpMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth, uiPartIdx );
#else
  pcCU->getCUMvField( eRefList )->setAllMv( cTmpMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth );
#endif  
  
  xReadSvlc( iHor );
  xReadSvlc( iVer );
  
  // set mvd
  TComMv cMv( iHor, iVer );
#if AMP
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth, uiPartIdx );
#else
  pcCU->getCUMvField( eRefList )->setAllMvd( cMv, pcCU->getPartitionSize( uiAbsPartIdx ), uiAbsPartIdx, uiDepth );
#endif  
  
  return;
}

Void TDecCavlc::parseDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiQp;
  Int  iDQp;
  
  xReadSvlc( iDQp );
#if SUB_LCU_DQP
  uiQp = pcCU->getRefQP( uiAbsPartIdx ) + iDQp;
#else
  uiQp = pcCU->getSlice()->getSliceQp() + iDQp;
#endif

#if SUB_LCU_DQP
  UInt uiAbsQpCUPartIdx = (uiAbsPartIdx>>(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()<<1)) ;
  UInt uiQpCUDepth =   min(uiDepth,pcCU->getSlice()->getPPS()->getMaxCuDQPDepth()) ;
  pcCU->setQPSubParts( uiQp, uiAbsQpCUPartIdx, uiQpCUDepth );
#else
  pcCU->setQPSubParts( uiQp, uiAbsPartIdx, uiDepth );
#endif
}

#if CAVLC_RQT_CBP
/** Function for parsing cbf and split 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \param uiTrDepth Transform depth
 * \param uiDepth CU Depth
 * \param uiSubdiv split flag
 * \returns 
 * This function performs parsing for cbf and split flag
 */
Void TDecCavlc::parseCbfTrdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiTrDepth, UInt uiDepth, UInt& uiSubdiv )
{
  UInt uiCbf,tmp;
  UInt uiCBP,uiCbfY,uiCbfU,uiCbfV;
  UInt n,cx;
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
  UInt uiQPartNumParent  = pcCU->getPic()->getNumPartInCU() >> ((uiDepth-1) << 1);
  UInt uiQPartNumCurr    = pcCU->getPic()->getNumPartInCU() >> ((uiDepth) << 1);

  UInt uiFlagPattern = xGetFlagPattern( pcCU, uiAbsPartIdx, uiDepth, uiSubdiv );

  n = pcCU->isIntra( uiAbsPartIdx ) ? 0 : 1;
  uiCbfY = uiCbfU = uiCbfV = 0;

#if LG_MRG_2Nx2N_CBF
  Bool bMRG_2Nx2N_TrDepth_Is_Zero = false;
  if(  uiTrDepth==0 
    && pcCU->getPartitionSize( uiAbsPartIdx) == SIZE_2Nx2N 
    && pcCU->getMergeFlag( uiAbsPartIdx ) )
  {
    bMRG_2Nx2N_TrDepth_Is_Zero = true;
  }
#endif

  if(uiFlagPattern < 8)
  {
    if( uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
    {
      if(uiFlagPattern & 0x04)
      {
        xReadFlag(uiCbfU);
      }
      if(uiFlagPattern & 0x02)
      {
        xReadFlag(uiCbfV);
      }
    }
    else
    {
      uiCbfU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1)?1:0;
      uiCbfV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1)?1:0;
    }
    if(uiFlagPattern & 0x01)
    {
      parseTransformSubdivFlag( uiSubdiv, 0);
    }
  }
  else
  {
    if(uiFlagPattern == 8)
    {
      if (uiAbsPartIdx % uiQPartNumParent ==0)
      {
        parseBlockCbf(pcCU, uiAbsPartIdx, TEXT_LUMA, uiTrDepth,uiDepth, uiQPartNumCurr);
      }
      uiCbfY = ( pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA ) >> uiTrDepth) & 0x1;
      if( uiLog2TrSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        uiCbfU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1)?1:0;
        uiCbfV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1)?1:0;
      }
    }
    else if(uiFlagPattern == 9)
    {
      bool bNeedToDecode = true;
      if ( n==1 && (uiAbsPartIdx%uiQPartNumParent) / uiQPartNumCurr == 3 )  // last one
      {
        UInt uiTempAbsPartIdx = uiAbsPartIdx/uiQPartNumParent*uiQPartNumParent;
        if ( pcCU->getCbf( uiTempAbsPartIdx + uiQPartNumCurr*0, TEXT_LUMA, uiTrDepth ) ||
          pcCU->getCbf( uiTempAbsPartIdx + uiQPartNumCurr*1, TEXT_LUMA, uiTrDepth ) ||
          pcCU->getCbf( uiTempAbsPartIdx + uiQPartNumCurr*2, TEXT_LUMA, uiTrDepth ) )
        {
          bNeedToDecode = true;
        }
        else
        {
          bNeedToDecode = false;
          xReadFlag( uiSubdiv );
          uiCbfY = 1;
        }
      }
      if ( bNeedToDecode )
      {
        UInt uiSymbol;
        xReadUnaryMaxSymbol(cx, n?2:3);
        uiSymbol = m_uiCBP_YS_TableD[n][cx];
        uiCbfY = uiSymbol >> 1;
        uiSubdiv = uiSymbol & 0x01; 
        adaptCodeword(cx, m_ucCBP_YS_TableCounter[n],  m_ucCBP_YS_TableCounterSum[n],  m_uiCBP_YS_TableD[n],  NULL, 3);
      }
    }
    else if (uiFlagPattern == 14)
    {
#if LG_MRG_2Nx2N_CBF
      if (bMRG_2Nx2N_TrDepth_Is_Zero)
      {
        UInt i;
        xReadUnaryMaxSymbol(cx, 6);

        for (i=0 ; i<8 ; i++)
        {
          if (m_uiCBP_YUV_TableD[1][i] == 0) 
          {
            cx++;
            break;
          }
          else if (cx==i) 
            break;          
        }
        uiCBP = m_uiCBP_YUV_TableD[1][cx];
        adaptCodeword(cx,  m_ucCBP_YUV_TableCounter[1],  m_ucCBP_YUV_TableCounterSum[1],  m_uiCBP_YUV_TableD[1],  NULL, 4);

        uiCbfY = (uiCBP>>2)&1;
        uiCbfU = (uiCBP>>1)&1;
        uiCbfV = (uiCBP>>0)&1;
      }
      else
      {      
#endif
        UInt  uiIdx = uiTrDepth? (2 + n) : n;
        xReadUnaryMaxSymbol(cx, 7);
        uiCBP = m_uiCBP_YUV_TableD[uiIdx][cx];
        adaptCodeword(cx,  m_ucCBP_YUV_TableCounter[uiIdx],  m_ucCBP_YUV_TableCounterSum[uiIdx],  m_uiCBP_YUV_TableD[uiIdx],  NULL, 4);
        uiCbfY = (uiCBP>>2)&1;
        uiCbfU = (uiCBP>>1)&1;
        uiCbfV = (uiCBP>>0)&1;
#if LG_MRG_2Nx2N_CBF
      }
#endif
    }
    else if ( uiFlagPattern == 11 || uiFlagPattern == 13 || uiFlagPattern == 15)
    {
      UInt uiSymbol, i;
      UInt uiCbfUV;
#if LG_MRG_2Nx2N_CBF
      if (bMRG_2Nx2N_TrDepth_Is_Zero)
      {        
        xReadUnaryMaxSymbol(uiSymbol, 5);

        for (i=0 ; i<8 ; i++)
        {
          if (m_uiCBP_YCS_TableD[1][i] == 0) 
          {
            uiSymbol++;
            break;
          }
          else if (uiSymbol==i) 
            break;
        }

        uiCBP = m_uiCBP_YCS_TableD[n][uiSymbol];
        adaptCodeword(uiSymbol, m_ucCBP_YCS_TableCounter[1], m_ucCBP_YCS_TableCounterSum[1],  m_uiCBP_YCS_TableD[1],  NULL, 4);
        uiCbfY = uiCBP >> 2;
        uiCbfUV = (uiCBP >> 1)& 0x01;
        uiSubdiv = uiCBP & 0x01;
      }
      else
      {
#endif  
      m_pcBitstream->pseudoRead(6, uiSymbol);
      for (i=0;i<8;i++)
      {
        if( (uiSymbol>>(6 - g_auiCBP_YCS_TableLen[n][i])) == g_auiCBP_YCS_Table[n][i] && g_auiCBP_YCS_TableLen[n][i])
        {
          m_pcBitstream->read(g_auiCBP_YCS_TableLen[n][i],uiSymbol);
          uiSymbol =i;
          break;
        }
      }

      uiCBP = m_uiCBP_YCS_TableD[n][uiSymbol];
      adaptCodeword(uiSymbol, m_ucCBP_YCS_TableCounter[n], m_ucCBP_YCS_TableCounterSum[n],  m_uiCBP_YCS_TableD[n],  NULL, 4);
      uiCbfY = uiCBP >> 2;
      uiCbfUV = (uiCBP >> 1)& 0x01;
      uiSubdiv = uiCBP & 0x01;
#if LG_MRG_2Nx2N_CBF
      }
#endif

      uiCbfU = 0; uiCbfV = 0;
      if (uiFlagPattern == 15)
      {
        if(uiCbfUV)
        {
          xReadUnaryMaxSymbol( uiSymbol , 2); 
          uiSymbol = n? (uiSymbol + 1): (3 - uiSymbol);
          uiCbfU = uiSymbol >> 1;
          uiCbfV = uiSymbol & 0x01;
        }
      }
      else
      {
        uiCbfU = (uiFlagPattern & 0x04) ?  uiCbfUV: 0;
        uiCbfV = (uiFlagPattern & 0x02) ?  uiCbfUV: 0;
      }
    }
    else if (uiFlagPattern == 10 || uiFlagPattern == 12)
    {
      UInt uiSymbol;
      xReadUnaryMaxSymbol(tmp, 3);
      uiSymbol = g_auiCBP_YC_TableD[n][tmp];
      uiCbfY = uiSymbol >> 1;

      uiCbfU = uiCbfV = 0;
      if(uiSymbol & 0x01)
      {
        uiCbfU = (uiFlagPattern & 0x04)? 1 : 0;
        uiCbfV = (uiFlagPattern & 0x02)? 1 : 0;
      }
    }
  }

  uiCbf = pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA );
  pcCU->setCbfSubParts( uiCbf | ( uiCbfY << uiTrDepth ), TEXT_LUMA, uiAbsPartIdx, uiDepth );

  uiCbf = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U );
  pcCU->setCbfSubParts( uiCbf | ( uiCbfU << uiTrDepth ), TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );

  uiCbf = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V );
  pcCU->setCbfSubParts( uiCbf | ( uiCbfV << uiTrDepth ), TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );

  if(!pcCU->isIntra(uiAbsPartIdx) && uiCbfY == 0 && uiCbfU == 0 && uiCbfV == 0)
  {
    uiSubdiv = 0;
  }

  return;
}

/** Function for parsing cbf and split 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \param uiDepth CU Depth
 * \param uiSubdiv split flag
 * \returns flag pattern
 * This function gets flagpattern for cbf and split flag
 */
UInt TDecCavlc::xGetFlagPattern( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiSubdiv )
{
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;
  UInt uiTrDepth =  uiDepth - pcCU->getDepth( uiAbsPartIdx );
  UInt patternYUV, patternDiv;
  UInt bY, bU, bV;


  UInt uiFullDepth = pcCU->getDepth(uiAbsPartIdx) + uiTrDepth;
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
  if(uiTrDepth == 0)
  {
    patternYUV = 7;
  }
  else if( uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    bY = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, uiTrDepth - 1)?1:0;
    bU = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1)?1:0;
    bV = pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1)?1:0;
    patternYUV = (bY<<2) + (bU<<1) + bV;
  }
  else
  {
    bY = pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, uiTrDepth - 1)?1:0;
    patternYUV = bY<<2;
  }


  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    patternDiv = 0; uiSubdiv = 1;
  }
#ifdef MOT_TUPU_MAXDEPTH1
  else if( (pcCU->getSlice()->getSPS()->getQuadtreeTUMaxDepthInter() == 1) && (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER) && ( pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_Nx2N) && (uiDepth == pcCU->getDepth(uiAbsPartIdx)) )
  {
    patternDiv = 0;  uiSubdiv = (uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx));
  }
#endif
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    patternDiv = 0; uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    patternDiv = 0; uiSubdiv = 0;
  }
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    patternDiv = 0; uiSubdiv = 0;
  } 
  else
  {
    patternDiv = 1;
  }

  return ((patternYUV<<1)+patternDiv);
}
#endif
Void TDecCavlc::parseCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  if (eType == TEXT_ALL)
  {
    UInt uiCbf,tmp;
    UInt uiCBP,uiCbfY,uiCbfU,uiCbfV;

#if CAVLC_COUNTER_ADAPT
    Int n, cx;
#else
    Int n,x,cx,y,cy;
#endif

    /* Start adaptation */
    n = pcCU->isIntra( uiAbsPartIdx ) ? 0 : 1;
#if CAVLC_RQT_CBP
    UInt vlcn = 0;
#else
    UInt vlcn = g_auiCbpVlcNum[n][m_uiCbpVlcIdx[n]];
#endif
    tmp = xReadVlc( vlcn );    
#if CAVLC_RQT_CBP
    uiCBP = m_uiCBP_YUV_TableD[n][tmp];
#else
    uiCBP = m_uiCBPTableD[n][tmp];
#endif

    /* Adapt LP table */
    cx = tmp;

#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
    adaptCodeword(cx, m_ucCBP_YUV_TableCounter[n],  m_ucCBP_YUV_TableCounterSum[n],  m_uiCBP_YUV_TableD[n],  NULL, 4);
#else
    adaptCodeword(cx, m_ucCBFTableCounter[n],  m_ucCBFTableCounterSum[n],  m_uiCBPTableD[n],  NULL, 4);
#endif
#else
    cy = max(0,cx-1);
    x = uiCBP;
    y = m_uiCBPTableD[n][cy];
    m_uiCBPTableD[n][cy] = x;
    m_uiCBPTableD[n][cx] = y;
    m_uiCbpVlcIdx[n] += cx == m_uiCbpVlcIdx[n] ? 0 : (cx < m_uiCbpVlcIdx[n] ? -1 : 1);
#endif

    uiCbfY = (uiCBP>>0)&1;
    uiCbfU = (uiCBP>>1)&1;
    uiCbfV = (uiCBP>>2)&1;
    
    uiCbf = pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA );
    pcCU->setCbfSubParts( uiCbf | ( uiCbfY << uiTrDepth ), TEXT_LUMA, uiAbsPartIdx, uiDepth );
    
    uiCbf = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U );
    pcCU->setCbfSubParts( uiCbf | ( uiCbfU << uiTrDepth ), TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
    
    uiCbf = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V );
    pcCU->setCbfSubParts( uiCbf | ( uiCbfV << uiTrDepth ), TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
  }
}

Void TDecCavlc::parseBlockCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth, UInt uiQPartNum )
{
  assert(uiTrDepth > 0);
  UInt uiCbf4, uiCbf;
  
#if CAVLC_COUNTER_ADAPT
  Int cx;
#else
  Int x,cx,y,cy;
#endif

  UInt tmp;
  
  UInt n = (pcCU->isIntra(uiAbsPartIdx) && eType == TEXT_LUMA)? 0:1;
#if CAVLC_RQT_CBP
  UInt vlcn = (n==0)?g_auiCBP_4Y_VlcNum[m_uiCBP_4Y_VlcIdx]:11;
#else
  UInt vlcn = (n==0)?g_auiBlkCbpVlcNum[m_uiBlkCbpVlcIdx]:11;
#endif
  tmp = xReadVlc( vlcn );    
#if CAVLC_RQT_CBP
  uiCbf4 = m_uiCBP_4Y_TableD[n][tmp];
#else
  uiCbf4 = m_uiBlkCBPTableD[n][tmp];
#endif

  cx = tmp;

#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
  adaptCodeword(cx, m_ucCBP_4Y_TableCounter[n], m_ucCBP_4Y_TableCounterSum[n], m_uiCBP_4Y_TableD[n], NULL, 2);
#else
  adaptCodeword(cx, m_ucBlkCBPTableCounter[n],  m_ucBlkCBPTableCounterSum[n],  m_uiBlkCBPTableD[n],  NULL, 2);
#endif
#else
  cy = max(0,cx-1);
  x = uiCbf4;
  y = m_uiBlkCBPTableD[n][cy];
  m_uiBlkCBPTableD[n][cy] = x;
  m_uiBlkCBPTableD[n][cx] = y;
#endif

#if CAVLC_RQT_CBP
  if(n==0)
    m_uiCBP_4Y_VlcIdx += cx == m_uiCBP_4Y_VlcIdx ? 0 : (cx < m_uiCBP_4Y_VlcIdx ? -1 : 1);
#else
  if(n==0)
    m_uiBlkCbpVlcIdx += cx == m_uiBlkCbpVlcIdx ? 0 : (cx < m_uiBlkCbpVlcIdx ? -1 : 1);
#endif

  uiCbf4++;
  uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );
  pcCU->setCbfSubParts( uiCbf | ( ((uiCbf4>>3)&0x01) << uiTrDepth ), eType, uiAbsPartIdx, uiDepth ); uiAbsPartIdx += uiQPartNum;
  uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );
  pcCU->setCbfSubParts( uiCbf | ( ((uiCbf4>>2)&0x01) << uiTrDepth ), eType, uiAbsPartIdx, uiDepth ); uiAbsPartIdx += uiQPartNum;
  uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );
  pcCU->setCbfSubParts( uiCbf | ( ((uiCbf4>>1)&0x01) << uiTrDepth ), eType, uiAbsPartIdx, uiDepth ); uiAbsPartIdx += uiQPartNum;
  uiCbf = pcCU->getCbf( uiAbsPartIdx, eType );
  pcCU->setCbfSubParts( uiCbf | ( (uiCbf4&0x01) << uiTrDepth ), eType, uiAbsPartIdx, uiDepth );
  
  return;
}

Void TDecCavlc::parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  
  if ( uiWidth > pcCU->getSlice()->getSPS()->getMaxTrSize() )
  {
    uiWidth  = pcCU->getSlice()->getSPS()->getMaxTrSize();
    uiHeight = pcCU->getSlice()->getSPS()->getMaxTrSize();
  }
  UInt uiSize   = uiWidth*uiHeight;
  
  // point to coefficient
  TCoeff* piCoeff = pcCoef;
  
#if !QC_MDCS
  // initialize scan
  const UInt*  pucScan;
#endif
  
#if CAVLC_COEF_LRG_BLK
#if CAVLC_COEF_LRG_BLK_CHROMA
  UInt maxBlSize = 32;
#else
  UInt maxBlSize = (eTType==TEXT_LUMA)? 32:8;
#endif
  UInt uiBlSize = min(maxBlSize,uiWidth);
#if !QC_MDCS
  UInt uiConvBit = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : uiBlSize];
#endif
  UInt uiNoCoeff = uiBlSize*uiBlSize;
#else
  //UInt uiConvBit = g_aucConvertToBit[ Min(8,uiWidth) ];
  UInt uiConvBit = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : min(8,uiWidth)    ];
#endif
#if !QC_MDCS
  pucScan        = g_auiFrameScanXY  [ uiConvBit + 1 ];
#endif
  
#if QC_MDCS
  UInt uiBlkPos;
#if CAVLC_COEF_LRG_BLK
  UInt uiLog2BlkSize = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : uiBlSize] + 2;
#else
  UInt uiLog2BlkSize = g_aucConvertToBit[ pcCU->isIntra( uiAbsPartIdx ) ? uiWidth : min(8,uiWidth)    ] + 2;
#endif
  const UInt uiScanIdx = pcCU->getCoefScanIdx(uiAbsPartIdx, uiWidth, eTType==TEXT_LUMA, pcCU->isIntra(uiAbsPartIdx));
#endif //QC_MDCS
  

#if !REMOVE_DIRECT_INTRA_DC_CODING
  UInt uiDecodeDCCoeff = 0;
  Int dcCoeff = 0;
  if (pcCU->isIntra(uiAbsPartIdx))
  {
    UInt uiAbsPartIdxL, uiAbsPartIdxA;
    TComDataCU* pcCUL   = pcCU->getPULeft (uiAbsPartIdxL, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    TComDataCU* pcCUA   = pcCU->getPUAbove(uiAbsPartIdxA, pcCU->getZorderIdxInCU() + uiAbsPartIdx);
    if (pcCUL == NULL && pcCUA == NULL)
    {
      uiDecodeDCCoeff = 1;
      dcCoeff = xReadVlc(eTType == TEXT_LUMA ? 3 : 1);
      if (dcCoeff)
      {
        UInt sign;
        xReadFlag(sign);
        if (sign)
        {
          dcCoeff = -dcCoeff;
        }
      }
    }
  }
#endif
  
  UInt uiScanning;
  
#if CAVLC_COEF_LRG_BLK
  static TCoeff scoeff[1024];
#else
  TCoeff scoeff[64];
#endif
  Int iBlockType;
  if( uiSize == 2*2 )
  {
    // hack: re-use 4x4 coding
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );

#if CAVLC_COEF_LRG_BLK
    xParseCoeff( scoeff, iBlockType, 4
#if CAVLC_RUNLEVEL_TABLE_REM
               , pcCU->isIntra(uiAbsPartIdx)
#endif
               );
#else
    xParseCoeff4x4( scoeff, iBlockType );
#endif
    
    for (uiScanning=0; uiScanning<4; uiScanning++)
    {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning];  
      piCoeff[ uiBlkPos ] =  scoeff[15-uiScanning];
#else
      piCoeff[ pucScan[ uiScanning ] ] = scoeff[15-uiScanning];
#endif //QC_MDCS
    }
  }
  else if ( uiSize == 4*4 )
  {
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V)
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
#if CAVLC_COEF_LRG_BLK
    xParseCoeff( scoeff, iBlockType, 4
#if CAVLC_RUNLEVEL_TABLE_REM
               , pcCU->isIntra(uiAbsPartIdx)
#endif
               );
#else
    xParseCoeff4x4( scoeff, iBlockType );
#endif
    
    for (uiScanning=0; uiScanning<16; uiScanning++)
    {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning];  
      piCoeff[ uiBlkPos ] =  scoeff[15-uiScanning];
#else
      piCoeff[ pucScan[ uiScanning ] ] = scoeff[15-uiScanning];
#endif //QC_MDCS
    }
  }
  else if ( uiSize == 8*8 )
  {
    if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) //8x8 specific
      iBlockType = eTType-2;
    else
      iBlockType = 2 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
#if CAVLC_COEF_LRG_BLK
    xParseCoeff( scoeff, iBlockType, 8
#if CAVLC_RUNLEVEL_TABLE_REM
               , pcCU->isIntra(uiAbsPartIdx)
#endif
               );
#else
    xParseCoeff8x8( scoeff, iBlockType );
#endif
    
    for (uiScanning=0; uiScanning<64; uiScanning++)
    {
#if QC_MDCS
      uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
      piCoeff[ uiBlkPos ] =  scoeff[63-uiScanning];
#else
      piCoeff[ pucScan[ uiScanning ] ] = scoeff[63-uiScanning];
#endif //QC_MDCS
    }
    
  }
  else
  {
    if (!pcCU->isIntra( uiAbsPartIdx ))
    {
      memset(piCoeff,0,sizeof(TCoeff)*uiSize);
      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );
#if CAVLC_COEF_LRG_BLK 
      xParseCoeff( scoeff, iBlockType, uiBlSize
#if CAVLC_RUNLEVEL_TABLE_REM
                 , pcCU->isIntra(uiAbsPartIdx)
#endif
                 );
#else
      xParseCoeff8x8( scoeff, iBlockType );
#endif
      
#if CAVLC_COEF_LRG_BLK
      for (uiScanning=0; uiScanning<uiNoCoeff; uiScanning++)
      {
#if QC_MDCS 
        uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
        uiBlkPos = (uiBlkPos/uiBlSize)* uiWidth + (uiBlkPos&(uiBlSize-1));
        piCoeff[ uiBlkPos ] =  scoeff[uiNoCoeff-uiScanning-1];
#else
        piCoeff[(pucScan[uiScanning]/uiBlSize)*uiWidth + (pucScan[uiScanning]&(uiBlSize-1))]=scoeff[uiNoCoeff-uiScanning-1];      
#endif
      }
#else
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {  
#if QC_MDCS
        uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
        uiBlkPos = (uiBlkPos/8)* uiWidth + uiBlkPos%8;
        piCoeff[ uiBlkPos ] =  scoeff[63-uiScanning];
#else
        piCoeff[(pucScan[uiScanning]/8)*uiWidth + (pucScan[uiScanning]%8)] = scoeff[63-uiScanning];
#endif //QC_MDCS
      }
#endif
      return;
    }
    
    if(pcCU->isIntra( uiAbsPartIdx ))
    {
      memset(piCoeff,0,sizeof(TCoeff)*uiSize);
      
      if (eTType==TEXT_CHROMA_U || eTType==TEXT_CHROMA_V) 
        iBlockType = eTType-2;
      else
        iBlockType = 5 + ( pcCU->isIntra(uiAbsPartIdx) ? 0 : pcCU->getSlice()->getSliceType() );

#if CAVLC_COEF_LRG_BLK
      xParseCoeff( scoeff, iBlockType, uiBlSize
#if CAVLC_RUNLEVEL_TABLE_REM
                 , pcCU->isIntra(uiAbsPartIdx)
#endif
                 );
      for (uiScanning=0; uiScanning<uiNoCoeff; uiScanning++)
      {
#if QC_MDCS
        uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
        piCoeff[ uiBlkPos ] =  scoeff[uiNoCoeff - uiScanning - 1];
#else
        piCoeff[ pucScan[ uiScanning ] ] = scoeff[uiNoCoeff - uiScanning - 1];
#endif //QC_MDCS
      }
#else
      xParseCoeff8x8( scoeff, iBlockType );
      
      for (uiScanning=0; uiScanning<64; uiScanning++)
      {
#if QC_MDCS
        uiBlkPos = g_auiSigLastScan[uiScanIdx][uiLog2BlkSize-1][uiScanning]; 
        piCoeff[ uiBlkPos ] =  scoeff[63-uiScanning];
#else
        piCoeff[ pucScan[ uiScanning ] ] = scoeff[63-uiScanning];
#endif //QC_MDCS
      }
#endif
    }
  }
  
#if !REMOVE_DIRECT_INTRA_DC_CODING
  if (uiDecodeDCCoeff == 1)
  {
    piCoeff[0] = dcCoeff;
  }
#endif
  
  return ;
}

Void TDecCavlc::parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize )
{
  xReadFlag( ruiSubdivFlag );
}

Void TDecCavlc::parseQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth )
{
  UInt uiSymbol;
  xReadFlag( uiSymbol );
  pcCU->setCbfSubParts( uiSymbol << uiTrDepth, eType, uiAbsPartIdx, uiDepth );
}

Void TDecCavlc::parseQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf )
{
  UInt uiSymbol;
  xReadFlag( uiSymbol );
  uiQtRootCbf = uiSymbol;
}

Void TDecCavlc::parseAlfFlag (UInt& ruiVal)
{
  xReadFlag( ruiVal );
}

Void TDecCavlc::parseAlfFlagNum( UInt& ruiVal, UInt minValue, UInt depth )
{
  UInt uiLength = 0;
  UInt maxValue = (minValue << (depth*2));
  UInt temp = maxValue - minValue;
  for(UInt i=0; i<32; i++)
  {
    if(temp&0x1)
    {
      uiLength = i+1;
    }
    temp = (temp >> 1);
  }
  if(uiLength)
  {
    xReadCode( uiLength, ruiVal );
  }
  else
  {
    ruiVal = 0;
  }
  ruiVal += minValue;
}

Void TDecCavlc::parseAlfCtrlFlag( UInt &ruiAlfCtrlFlag )
{
  UInt uiSymbol;
  xReadFlag( uiSymbol );
  ruiAlfCtrlFlag = uiSymbol;
}

Void TDecCavlc::parseAlfUvlc (UInt& ruiVal)
{
  xReadUvlc( ruiVal );
}

Void TDecCavlc::parseAlfSvlc (Int&  riVal)
{
  xReadSvlc( riVal );
}



#if MTK_SAO
Void TDecCavlc::parseAoFlag (UInt& ruiVal)
{
  xReadFlag( ruiVal );
}
Void TDecCavlc::parseAoUvlc (UInt& ruiVal)
{
  xReadUvlc( ruiVal );
}

Void TDecCavlc::parseAoSvlc (Int&  riVal)
{
  xReadSvlc( riVal );
}
#endif

Void TDecCavlc::parseMergeFlag ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx )
{
  if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N )
    return;
  UInt uiSymbol;
  xReadFlag( uiSymbol );
  pcCU->setMergeFlagSubParts( uiSymbol ? true : false, uiAbsPartIdx, uiPUIdx, uiDepth );
}

/** parse merge index
 * \param pcCU
 * \param ruiMergeIndex 
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \returns Void
 */
Void TDecCavlc::parseMergeIndex ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth )
{
#if MRG_AMVP_FIXED_IDX_F470
  UInt uiNumCand = MRG_MAX_NUM_CANDS;
#else
  Bool bLeftInvolved = false;
  Bool bAboveInvolved = false;
  Bool bCollocatedInvolved = false;
  Bool bCornerInvolved = false;
  UInt uiNumCand = 0;
  for( UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; ++uiIter )
  {
    if( pcCU->getNeighbourCandIdx( uiIter, uiAbsPartIdx ) == uiIter + 1 )
    {
      uiNumCand++;
      if( uiIter == 0 )
      {
        bLeftInvolved = true;
      }
      else if( uiIter == 1 )
      {
        bAboveInvolved = true;
      }
      else if( uiIter == 2 )
      {
        bCollocatedInvolved = true;
      }
      else if( uiIter == 3 )
      {
        bCornerInvolved = true;
      }
    }
  }
#endif
  assert( uiNumCand > 1 );
  UInt uiUnaryIdx = 0;
  for( ; uiUnaryIdx < uiNumCand - 1; ++uiUnaryIdx )
  {
    UInt uiSymbol = 0;
    xReadFlag( uiSymbol );
    if( uiSymbol == 0 )
    {
      break;
    }
  }
#if !(MRG_AMVP_FIXED_IDX_F470)
  if( !bLeftInvolved )
  {
    ++uiUnaryIdx;
  }
  if( !bAboveInvolved && uiUnaryIdx >= 1 )
  {
    ++uiUnaryIdx;
  }

  if( !bCollocatedInvolved && uiUnaryIdx >= 2 )
  {
    ++uiUnaryIdx;
  }
  if( !bCornerInvolved && uiUnaryIdx >= 3 )
  {
    ++uiUnaryIdx;
  }
#endif
  ruiMergeIndex = uiUnaryIdx;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TDecCavlc::xReadCode (UInt uiLength, UInt& ruiCode)
{
  assert ( uiLength > 0 );
  m_pcBitstream->read (uiLength, ruiCode);
}

Void TDecCavlc::xReadUvlc( UInt& ruiVal)
{
  UInt uiVal = 0;
  UInt uiCode = 0;
  UInt uiLength;
  m_pcBitstream->read( 1, uiCode );
  
  if( 0 == uiCode )
  {
    uiLength = 0;
    
    while( ! ( uiCode & 1 ))
    {
      m_pcBitstream->read( 1, uiCode );
      uiLength++;
    }
    
    m_pcBitstream->read( uiLength, uiVal );
    
    uiVal += (1 << uiLength)-1;
  }
  
  ruiVal = uiVal;
}

Void TDecCavlc::xReadSvlc( Int& riVal)
{
  UInt uiBits = 0;
  m_pcBitstream->read( 1, uiBits );
  if( 0 == uiBits )
  {
    UInt uiLength = 0;
    
    while( ! ( uiBits & 1 ))
    {
      m_pcBitstream->read( 1, uiBits );
      uiLength++;
    }
    
    m_pcBitstream->read( uiLength, uiBits );
    
    uiBits += (1 << uiLength);
    riVal = ( uiBits & 1) ? -(Int)(uiBits>>1) : (Int)(uiBits>>1);
  }
  else
  {
    riVal = 0;
  }
}

Void TDecCavlc::xReadFlag (UInt& ruiCode)
{
  m_pcBitstream->read( 1, ruiCode );
}

#if E057_INTRA_PCM
/** Parse PCM alignment zero bits.
 * \returns Void
 */
Void TDecCavlc::xReadPCMAlignZero( )
{
  UInt uiNumberOfBits = m_pcBitstream->getNumBitsUntilByteAligned();

  if(uiNumberOfBits)
  {
    UInt uiBits;
    UInt uiSymbol;

    for(uiBits = 0; uiBits < uiNumberOfBits; uiBits++)
    {
      xReadFlag( uiSymbol );

      if(uiSymbol)
      {
        printf("\nWarning! pcm_align_zero include a non-zero value.\n");
      }
    }
  }
}
#endif

Void TDecCavlc::xReadUnaryMaxSymbol( UInt& ruiSymbol, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    ruiSymbol = 0;
    return;
  }
  
  xReadFlag( ruiSymbol );
  
  if (ruiSymbol == 0 || uiMaxSymbol == 1)
  {
    return;
  }
  
  UInt uiSymbol = 0;
  UInt uiCont;
  
  do
  {
    xReadFlag( uiCont );
    uiSymbol++;
  }
  while( uiCont && (uiSymbol < uiMaxSymbol-1) );
  
  if( uiCont && (uiSymbol == uiMaxSymbol-1) )
  {
    uiSymbol++;
  }
  
  ruiSymbol = uiSymbol;
}

Void TDecCavlc::xReadExGolombLevel( UInt& ruiSymbol )
{
  UInt uiSymbol ;
  UInt uiCount = 0;
  do
  {
    xReadFlag( uiSymbol );
    uiCount++;
  }
  while( uiSymbol && (uiCount != 13));
  
  ruiSymbol = uiCount-1;
  
  if( uiSymbol )
  {
    xReadEpExGolomb( uiSymbol, 0 );
    ruiSymbol += uiSymbol+1;
  }
  
  return;
}

Void TDecCavlc::xReadEpExGolomb( UInt& ruiSymbol, UInt uiCount )
{
  UInt uiSymbol = 0;
  UInt uiBit = 1;
  
  
  while( uiBit )
  {
    xReadFlag( uiBit );
    uiSymbol += uiBit << uiCount++;
  }
  
  uiCount--;
  while( uiCount-- )
  {
    xReadFlag( uiBit );
    uiSymbol += uiBit << uiCount;
  }
  
  ruiSymbol = uiSymbol;
  
  return;
}

UInt TDecCavlc::xGetBit()
{
  UInt ruiCode;
  m_pcBitstream->read( 1, ruiCode );
  return ruiCode;
}

Int TDecCavlc::xReadVlc( Int n )
{
#if CAVLC_COEF_LRG_BLK
  assert( n>=0 && n<=13 );
#else
  assert( n>=0 && n<=11 );
#endif
  
  UInt zeroes=0, done=0, tmp;
  UInt cw, bit;
  UInt val = 0;
  UInt first;
  UInt lead = 0;
  
  if (n < 5)
  {
    while (!done && zeroes < 6)
    {
      xReadFlag( bit );
      if (bit)
      {
        if (n)
        {
          xReadCode( n, cw );
        }
        else
        {
          cw = 0;
        }
        done = 1;
      }
      else
      {
        zeroes++;
      }
    }
    if ( done )
    {
      val = (zeroes<<n)+cw;
    }
    else
    {
      lead = n;
      while (!done)
      {
        xReadFlag( first );
        if ( !first )
        {
          lead++;
        }
        else
        {
          if ( lead )
          {
            xReadCode( lead, tmp );
          }
          else
          {
            tmp = 0;
          }
          val = 6 * (1 << n) + (1 << lead) + tmp - (1 << n);
          done = 1;
        }
      }
    }
  }
  else if (n < 8)
  {
    while (!done)
    {
      xReadFlag( bit );
      if ( bit )
      {
        xReadCode( n-4, cw );
        done = 1;
      }
      else
      {
        zeroes++;
      }
    }
    val = (zeroes<<(n-4))+cw;
  }
  else if (n == 8)
  {
    if ( xGetBit() )
    {
      val = 0;
    }
    else if ( xGetBit() )
    {
      val = 1;
    }
    else
    {
      val = 2;
    }
  }
  else if (n == 9)
  {
    if ( xGetBit() )
    {
      if ( xGetBit() )
      {
        xReadCode(3, val);
        val += 3;
      }
      else if ( xGetBit() )
      {
        val = xGetBit() + 1;
      }
      else
      {
        val = 0;
      }
    }
    else
    {
      while (!done)
      {
        xReadFlag( bit );
        if ( bit )
        {
          xReadCode(4, cw);
          done = 1;
        }
        else
        {
          zeroes++;
        }
      }
      val = (zeroes<<4)+cw+11;
    }
  }
  else if (n == 10)
  {
    while (!done)
    {
      xReadFlag( first );
      if ( !first )
      {
        lead++;
      }
      else
      {
        if ( !lead )
        {
          val = 0;
        }
        else
        {
          xReadCode(lead, val);
          val += (1<<lead);
          val--;
        }
        done = 1;
      }
    }
  }
  else if (n == 11)
  {
    UInt code;
    xReadCode(3, val);
    if(val)
    {
      xReadCode(1, code);
      val = (val<<1)|code;
      val--;
    }
  }
#if CAVLC_COEF_LRG_BLK
  else if (n == 12)
  {
    while (!done)
    {
      m_pcBitstream->pseudoRead(32,val);
      if (val)
      {
        tmp = 31;
        while(!done)
        {
          if( val>>tmp )
          {
            xReadCode(32-tmp, val);
            lead += 31-tmp;
            xReadCode(6, val);
            val += (lead<<6);
            done = 1;
          }
          tmp--;
        }    
      }
      else
      {
        xReadCode(32, val);
        lead += 32;
      }
    }
  }
  else if (n == 13)
  {
    while (!done)
    {
      m_pcBitstream->pseudoRead(32,val);
      if (val)
      {
        tmp = 31;
        while(!done)
        {
          if(val>>tmp)
          {
            xReadCode(32-tmp, cw);
            zeroes += 31-tmp;
            xReadCode(4, cw);
            done = 1;
          }
          tmp--;
        } 
      }
      else
      {
        xReadCode(32, val);
        zeroes += 32;
      }
    }
    val = (zeroes<<4)+cw;
  }
#endif
  
  return val;
}

#if !CAVLC_COEF_LRG_BLK
Void TDecCavlc::xParseCoeff4x4( TCoeff* scoeff, Int n )
{
  Int i;
  UInt sign;
  Int tmp;
  Int vlc,cn,this_pos;
  Int maxrun;
  Int last_position;
  Int atable[5] = {4,6,14,28,0xfffffff};
  Int vlc_adaptive=0;
  Int done;
  LastCoeffStruct combo;
  
  Int nTab;
  Int tr1;
  nTab=max(0,n-2);

  for (i = 0; i < 16; i++)
  {
    scoeff[i] = 0;
  }
  
  {
    /* Get the last nonzero coeff */
    Int x,y,cx,cy,vlcNum;
    Int vlcTable[8] = {2,2,2};
    
    /* Decode according to current LP table */
    vlcNum = vlcTable[nTab];
    tmp = xReadVlc( vlcNum );
    cn = m_uiLPTableD4[nTab][tmp];
    combo.level = (cn>15);
    combo.last_pos = cn&0x0f;
    
    /* Adapt LP table */
    cx = tmp;
    cy = max( 0, cx-1 );
    x = cn;
    y = m_uiLPTableD4[nTab][cy];
    m_uiLPTableD4[nTab][cy] = x;
    m_uiLPTableD4[nTab][cx] = y;
  }
  
  if ( combo.level == 1 )
  {
    tmp = xReadVlc( 0 );
    sign = tmp&1;
    tmp = (tmp>>1)+2;
  }
  else
  {
    tmp = 1;
    xReadFlag( sign );
  }
  
  if (tmp>1)
  {
    tr1=0;
  }
  else
  {
    tr1=1;
  }

  if ( sign )
  {
    tmp = -tmp;
  }
  
  last_position = combo.last_pos;
  this_pos = 15 - last_position;
  scoeff[this_pos] = tmp;
  i = this_pos;
  i++;
  
  done = 0;
  {
    while (!done && i < 16)
    {
      maxrun = 15-i;
      if(n==2)
        vlc = g_auiVlcTable8x8Intra[maxrun];
      else
        vlc = g_auiVlcTable8x8Inter[maxrun];
      
      /* Go into run mode */
      cn = xReadVlc( vlc );
      if(n==2)
      {
        xRunLevelIndInv(&combo, maxrun, g_auiLumaRunTr14x4[tr1][maxrun], cn);
      }
      else
      {
#if RUNLEVEL_TABLE_CUT
        xRunLevelIndInterInv(&combo, maxrun, cn);
#else
        combo = g_acstructLumaRun8x8[maxrun][cn];
#endif
      }
      i += combo.last_pos;
      /* No sign for last zeroes */
      if (i < 16)
      {
        if (combo.level == 1)
        {
          tmp = xReadVlc( 0 );
          sign = tmp&1;
          tmp = (tmp>>1)+2;
          done = 1;
        }
        else
        {
          tmp = 1;
          xReadFlag( sign );
        }
        if ( sign )
        {
          tmp = -tmp;
        }
        scoeff[i] = tmp;
      }
      i++;
      if (tr1>0 && tr1<MAX_TR1)
      {
        tr1++;
      }
    }
  }
  if (i < 16)
  {
    /* Get the rest in level mode */
    while ( i < 16 )
    {
      tmp = xReadVlc( vlc_adaptive );
      if ( tmp > atable[vlc_adaptive] )
      {
        vlc_adaptive++;
      }
      if ( tmp )
      {
        xReadFlag( sign );
        if ( sign )
        {
          tmp = -tmp;
        }
      }
      scoeff[i] = tmp;
      i++;
    }
  }
  
  return;
}
#endif

Void TDecCavlc::xRunLevelIndInv(LastCoeffStruct *combo, Int maxrun, UInt lrg1Pos, UInt cn)
{
  int lev, run;
  if (lrg1Pos>0)
  {
    if(cn < min(lrg1Pos, unsigned(maxrun+2)))
    {
      lev = 0; 
      run = cn; 
    }
    else if(cn < (maxrun<<1) + 4 - (Int)lrg1Pos)
    {
      if((cn+lrg1Pos)&1)
      {
        lev = 0;
        run = (cn + lrg1Pos - 1) >> 1;
      }
      else
      {
        lev = 1; 
        run = (cn - lrg1Pos)>>1;
      }
    }
    else
    {
      lev = 1;
      run = cn - maxrun - 2;
    }
  }
  else
  {
    if( cn & 1 )
    {
      lev = 0; run = (cn-1)>>1;
    }
    else
    {
      run = cn >> 1;
      lev = (run <= maxrun)?1:0;
    }
  }
  combo->level = lev;
  combo->last_pos = run;
}

#if RUNLEVEL_TABLE_CUT
/** Function for deriving run and level value in CAVLC run-level coding 
 * \param combo pointer to a struct of run and level
 * \param maxrun maximum length of run for a given coefficient location
 * \param cn codeword index
 * \returns
 * This function derives run and level value in CAVLC run-level coding based on codeword index and maximum run value.  
 */
#if CAVLC_RUNLEVEL_TABLE_REM
Void TDecCavlc::xRunLevelIndInterInv(LastCoeffStruct *combo, Int maxrun, UInt cn, UInt scale)
#else
Void TDecCavlc::xRunLevelIndInterInv(LastCoeffStruct *combo, Int maxrun, UInt cn)
#endif
{

#if !CAVLC_RUNLEVEL_TABLE_REM
  if (maxrun<28)
  {
    if(cn > maxrun+1)
    {
      combo->level = 1;
      combo->last_pos = g_acstructLumaRun8x8[maxrun][cn-maxrun-1];
    }
    else
    {
      combo->level = 0;
      combo->last_pos = g_acstructLumaRun8x8[maxrun][cn];
    }
  }
  else
#endif
  {
    if(cn<maxrun+2)
    {
      combo->level = 0;
      combo->last_pos = cn;
#if CAVLC_RUNLEVEL_TABLE_REM
      {
        int thr = (maxrun + 1) >> scale;
        if (combo->last_pos >= thr)
          combo->last_pos = (combo->last_pos == thr) ? (maxrun+1) : (combo->last_pos-1);
      }
#endif
    }
    else
    {
      combo->level = 1;
      combo->last_pos = cn-maxrun-2;
    }
  }
}
#endif

#if !CAVLC_COEF_LRG_BLK
Void TDecCavlc::xParseCoeff8x8(TCoeff* scoeff, int n)
{
  Int i;
  UInt sign;
  Int tmp;
  LastCoeffStruct combo;
  Int vlc,cn,this_pos;
  Int maxrun;
  Int last_position;
  Int atable[5] = {4,6,14,28,0xfffffff};
  Int vlc_adaptive=0;
  Int done;
  Int tr1;
  
  static const Int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};
  Int sum_big_coef = 0;
  
  for (i = 0; i < 64; i++)
  {
    scoeff[i] = 0;
  }
  
  /* Get the last nonzero coeff */
  {
    Int x,y,cx,cy,vlcNum;
    
    /* Decode according to current LP table */
    // ADAPT_VLC_NUM
    vlcNum = g_auiLastPosVlcNum[n][min(16,m_uiLastPosVlcIndex[n])];
    tmp = xReadVlc( vlcNum );
    cn = m_uiLPTableD8[n][tmp];
    combo.level = (cn>63);
    combo.last_pos = cn&0x3f;
    
    /* Adapt LP table */
    cx = tmp;
    cy = max(0,cx-1);
    x = cn;
    y = m_uiLPTableD8[n][cy];
    m_uiLPTableD8[n][cy] = x;
    m_uiLPTableD8[n][cx] = y;

    // ADAPT_VLC_NUM
    m_uiLastPosVlcIndex[n] += cx == m_uiLastPosVlcIndex[n] ? 0 : (cx < m_uiLastPosVlcIndex[n] ? -1 : 1);
  }
  
  if (combo.level == 1)
  {
    tmp = xReadVlc( 0 );
    sign = tmp&1;
    tmp = (tmp>>1)+2;
  }
  else
  {
    tmp = 1;
    xReadFlag( sign );
  }

  if (tmp>1)
  {
    tr1=0;
  }
  else
  {
    tr1=1;
  }

  if ( sign )
  {
    tmp = -tmp;
  }
  
  last_position = combo.last_pos;
  this_pos = 63 - last_position;
  scoeff[this_pos] = tmp;
  i = this_pos;
  i++;
  
  done = 0;
  {
    while (!done && i < 64)
    {
      maxrun = 63-i;
      if (n == 2 || n == 5)
        vlc = g_auiVlcTable8x8Intra[min(maxrun,28)];
      else
        vlc = g_auiVlcTable8x8Inter[min(maxrun,28)];
      
      /* Go into run mode */
      cn = xReadVlc( vlc );
      if (n == 2 || n == 5)
        xRunLevelIndInv(&combo, maxrun, g_auiLumaRunTr18x8[tr1][min(maxrun,28)], cn);
      else
#if RUNLEVEL_TABLE_CUT
        xRunLevelIndInterInv(&combo, maxrun, cn);
#else
        combo = g_acstructLumaRun8x8[min(maxrun,28)][cn];
#endif
      i += combo.last_pos;
      /* No sign for last zeroes */
      if (i < 64)
      {
        if (combo.level == 1)
        {
          tmp = xReadVlc( 0 );
          sign = tmp&1;
          tmp = (tmp>>1)+2;
          
          sum_big_coef += tmp;
          if (i > switch_thr[n] || sum_big_coef > 2)
          {
            done = 1;
          }
        }
        else
        {
          tmp = 1;
          xReadFlag( sign );
        }
        if ( sign )
        {
          tmp = -tmp;
        }
        scoeff[i] = tmp;
      }
      i++;
      if (tr1==0 || combo.level != 0)
      {
        tr1=0;
      }
      else if( tr1 < MAX_TR1)
      {
        tr1++;
      }
    }
  }
  if (i < 64)
  {
    /* Get the rest in level mode */
    while (i<64)
    {
      tmp = xReadVlc( vlc_adaptive );
      
      if (tmp>atable[vlc_adaptive])
      {
        vlc_adaptive++;
      }
      if (tmp)
      {
        xReadFlag( sign );
        if ( sign )
        {
          tmp = -tmp;
        }
      }
      scoeff[i] = tmp;
      i++;
    }
  }
  return;
}
#endif


#if CAVLC_COEF_LRG_BLK
/** Function for parsing a block of transform coefficients in CAVLC.
 * \param scoeff    pointer to transform coefficient buffer
 * \param blockType block type information, e.g. luma, chroma, intra, inter, etc. 
 * \param blSize    block size
 */
Void TDecCavlc::xParseCoeff(TCoeff* scoeff, Int blockType, Int blSize
#if CAVLC_RUNLEVEL_TABLE_REM
                            , Int isIntra
#endif
                            )
{
  static const Int switch_thr[10] = {49,49,0,49,49,0,49,49,49,49};
#if MOD_INTRA_TABLE
  static const int aiTableTr1[2][5] = {{0, 1, 1, 1, 0},{0, 1, 2, 3, 4}};
#endif
  Int i, noCoeff=blSize*blSize;;
  UInt sign;
  LastCoeffStruct combo;
  Int cn, maxrun, tmprun;
#if TBL_RUN_ADAPT
   Int vlc_adaptive = 0;
#else
  Int atable[5] = {4,6,14,28,0xfffffff};
#endif
  Int done, tr1, tmp;
  Int sum_big_coef = 0;

  memset(scoeff,0,sizeof(TCoeff)*noCoeff);

#if CAVLC_RUNLEVEL_TABLE_REM
  Int scale = (isIntra && blockType < 2) ? 0 : 3;
#endif

  /* Get the last nonzero coeff */
  if(blSize >=8 )
  {
    /* Decode according to current LP table */
    // ADAPT_VLC_NUM
    tmp = g_auiLastPosVlcNum[blockType][min(16u,m_uiLastPosVlcIndex[blockType])];
    cn = xReadVlc( tmp );
    xLastLevelIndInv(combo.level, combo.last_pos, blSize, cn);

    /* Adapt LP table */
#if CAVLC_COEF_LRG_BLK_CHROMA
    cn = (blSize==8 || blockType<2)?cn:(cn>>2);
#else
    cn = (blSize==8)?cn:(cn>>2);
#endif
    // ADAPT_VLC_NUM
    m_uiLastPosVlcIndex[blockType] += cn == m_uiLastPosVlcIndex[blockType] ? 0 : (cn < m_uiLastPosVlcIndex[blockType] ? -1 : 1);
  }
  else
  {
    /* Get the last nonzero coeff */
    Int y,cy;
    Int nTab = max(0,blockType-2);
    
    /* Decode according to current LP table */
    tmp = xReadVlc( 2 );
    cn = m_uiLPTableD4[nTab][tmp];
    combo.level = (cn>15)?1:0;
    combo.last_pos = cn&0x0f;
    
    /* Adapt LP table */
    cy = max( 0, tmp-1 );
    y = m_uiLPTableD4[nTab][cy];
    m_uiLPTableD4[nTab][cy] = cn;
    m_uiLPTableD4[nTab][tmp] = y;
  }

  if (combo.level == 1)
  {
    tmp = xReadVlc( 0 );
    sign = tmp&1;
    tmp = (tmp>>1)+2;
    tr1=0;
  }
  else
  {
    tmp = 1;
    xReadFlag( sign );
    tr1=1;
  }

  i = noCoeff - 1 - combo.last_pos;
  scoeff[i++] = sign? -tmp:tmp;

  done = 0;

#if MOD_INTRA_TABLE
  const UInt *vlcTable = (blockType == 2||blockType == 5)?  g_auiVlcTable8x8Intra:
    ((blSize<=8)? g_auiVlcTable8x8Inter:g_auiVlcTable16x16Inter);

  const UInt **pLumaRunTr1 = (blSize==4)? g_pLumaRunTr14x4: ((blSize==8)? g_pLumaRunTr18x8: g_pLumaRunTr116x16);
#else
  const UInt *vlcTable = (blockType == 2||blockType == 5)? ((blSize<=8)? g_auiVlcTable8x8Intra:g_auiVlcTable16x16Intra):
    ((blSize<=8)? g_auiVlcTable8x8Inter:g_auiVlcTable16x16Inter);
  const UInt **pLumaRunTr1 = (blSize==4)? g_pLumaRunTr14x4:g_pLumaRunTr18x8;
#endif
  while (!done && i < noCoeff)
  {
    maxrun = noCoeff - 1 -i;
    tmprun = min(maxrun,28);
#if MOD_INTRA_TABLE 
    if (tmprun < 28 || blSize<=8 || (blockType!=2&&blockType!=5))
    {
      tmp = vlcTable[tmprun];
    }
    else
    {
      tmp = 2;
    }
#else
    tmp = vlcTable[tmprun];
#endif


    /* Go into run mode */
    cn = xReadVlc( tmp );
    if (blockType == 2 || blockType == 5)
    {
#if MOD_INTRA_TABLE
      xRunLevelIndInv(&combo, maxrun, pLumaRunTr1[aiTableTr1[(blSize&4)>>2][tr1]][tmprun], cn);
#else
      xRunLevelIndInv(&combo, maxrun, pLumaRunTr1[tr1][tmprun], cn);
#endif
    }
    else
    {
#if CAVLC_RUNLEVEL_TABLE_REM
      xRunLevelIndInterInv(&combo, maxrun, cn, scale);
#else
      xRunLevelIndInterInv(&combo, maxrun, cn);
#endif
    }

    i += combo.last_pos;
    if (i < noCoeff)
    {
      if (combo.level == 1)
      {
        tmp = xReadVlc( 0 );
        sign = tmp&1;
        tmp = (tmp>>1)+2;

        sum_big_coef += tmp;
        if (blSize==4 ||i > switch_thr[blockType] || sum_big_coef > 2)
        {
#if TBL_RUN_ADAPT
        if (tmp > atable[vlc_adaptive])
        {
           vlc_adaptive++;
        }

#endif          
          done = 1;
        }
      }
      else
      {
        tmp = 1;
        xReadFlag( sign );
      }
      scoeff[i++] = sign? -tmp:tmp;
    }

    if (tr1==0 || combo.level != 0)
    {
      tr1=0;
    }
    else if( tr1 < MAX_TR1)
    {
      tr1++;
    }
  }

  if (i < noCoeff)
  {
    /* Get the rest in level mode */
#if !TBL_RUN_ADAPT
    Int vlc_adaptive = 0;
#endif 
    while (i < noCoeff)
    {
      tmp = xReadVlc( vlc_adaptive );

      if (tmp)
      {
        xReadFlag( sign );
        scoeff[i] = sign?-tmp:tmp;
        if (tmp > atable[vlc_adaptive])
        {
          vlc_adaptive++;
        }
      }
      i++;
    }
  }

  return;
}

#endif

//! \}
