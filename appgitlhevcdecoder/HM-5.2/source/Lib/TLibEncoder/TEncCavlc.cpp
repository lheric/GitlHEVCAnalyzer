/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.  
 *
 * Copyright (c) 2010-2012, ITU/ISO/IEC
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

/** \file     TEncCavlc.cpp
    \brief    CAVLC encoder class
*/

#include "../TLibCommon/CommonDef.h"
#include "TEncCavlc.h"
#include "SEIwrite.h"

//! \ingroup TLibEncoder
//! \{

#if ENC_DEC_TRACE

#define WRITE_CODE( value, length, name)    xWriteCodeTr ( value, length, name )
#define WRITE_UVLC( value,         name)    xWriteUvlcTr ( value,         name )
#define WRITE_SVLC( value,         name)    xWriteSvlcTr ( value,         name )
#define WRITE_FLAG( value,         name)    xWriteFlagTr ( value,         name )

Void  xWriteUvlcTr          ( UInt value,               const Char *pSymbolName);
Void  xWriteSvlcTr          ( Int  value,               const Char *pSymbolName);
Void  xWriteFlagTr          ( UInt value,               const Char *pSymbolName);

Void  xTraceSPSHeader (TComSPS *pSPS)
{
  fprintf( g_hTrace, "=========== Sequence Parameter Set ID: %d ===========\n", pSPS->getSPSId() );
}

Void  xTracePPSHeader (TComPPS *pPPS)
{
  fprintf( g_hTrace, "=========== Picture Parameter Set ID: %d ===========\n", pPPS->getPPSId() );
}

Void  xTraceAPSHeader (TComAPS *pAPS)
{
  fprintf( g_hTrace, "=========== Adaptation Parameter Set ===========\n");
}

Void  xTraceSliceHeader (TComSlice *pSlice)
{
  fprintf( g_hTrace, "=========== Slice ===========\n");
}


Void  TEncCavlc::xWriteCodeTr (UInt value, UInt  length, const Char *pSymbolName)
{
  xWriteCode (value,length);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(%d) : %d\n", pSymbolName, length, value ); 
}

Void  TEncCavlc::xWriteUvlcTr (UInt value, const Char *pSymbolName)
{
  xWriteUvlc (value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(v) : %d\n", pSymbolName, value ); 
}

Void  TEncCavlc::xWriteSvlcTr (Int value, const Char *pSymbolName)
{
  xWriteSvlc(value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s s(v) : %d\n", pSymbolName, value ); 
}

Void  TEncCavlc::xWriteFlagTr(UInt value, const Char *pSymbolName)
{
  xWriteFlag(value);
  fprintf( g_hTrace, "%8lld  ", g_nSymbolCounter++ );
  fprintf( g_hTrace, "%-40s u(1) : %d\n", pSymbolName, value ); 
}

#else

#define WRITE_CODE( value, length, name)     xWriteCode ( value, length )
#define WRITE_UVLC( value,         name)     xWriteUvlc ( value )
#define WRITE_SVLC( value,         name)     xWriteSvlc ( value )
#define WRITE_FLAG( value,         name)     xWriteFlag ( value )

#endif



// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncCavlc::TEncCavlc()
{
  m_pcBitIf           = NULL;
  m_uiCoeffCost       = 0;
  m_bAlfCtrl = false;
  m_uiMaxAlfCtrlDepth = 0;
  
  m_iSliceGranularity = 0;
}

TEncCavlc::~TEncCavlc()
{
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncCavlc::resetEntropy()
{
}

/**
 * marshall the SEI message sei.
 */
void TEncCavlc::codeSEI(const SEI& sei)
{
  writeSEImessage(*m_pcBitIf, sei);
}

Void  TEncCavlc::codeAPSInitInfo(TComAPS* pcAPS)
{

#if ENC_DEC_TRACE  
  xTraceAPSHeader(pcAPS);
#endif
  //APS ID
  WRITE_UVLC( pcAPS->getAPSID(), "aps_id" );

  WRITE_FLAG( pcAPS->getScalingListEnabled()?1:0, "aps_scaling_list_data_present_flag");
  //DF flag
  WRITE_FLAG(pcAPS->getLoopFilterOffsetInAPS()?1:0, "aps_deblocking_filter_flag");
  //SAO flag
  WRITE_FLAG( pcAPS->getSaoEnabled()?1:0, "aps_sample_adaptive_offset_flag"); 

  //ALF flag
  WRITE_FLAG( pcAPS->getAlfEnabled()?1:0, "aps_adaptive_loop_filter_flag"); 

}

Void TEncCavlc::codeDFFlag(UInt uiCode, const Char *pSymbolName)
{
  WRITE_FLAG(uiCode, pSymbolName);
}
Void TEncCavlc::codeDFSvlc(Int iCode, const Char *pSymbolName)
{
  WRITE_SVLC(iCode, pSymbolName);
}

Void TEncCavlc::codeShortTermRefPicSet( TComPPS* pcPPS, TComReferencePictureSet* pcRPS )
{
#if PRINT_RPS_INFO
  int lastBits = getNumberOfWrittenBits();
#endif
  WRITE_FLAG( pcRPS->getInterRPSPrediction(), "inter_ref_pic_set_prediction_flag" ); // inter_RPS_prediction_flag
  if (pcRPS->getInterRPSPrediction()) 
  {
    //Int rIdx = i - pcRPS->getDeltaRIdxMinus1() - 1;
    Int deltaRPS = pcRPS->getDeltaRPS();
    //assert (rIdx <= i);
    WRITE_UVLC( pcRPS->getDeltaRIdxMinus1(), "delta_idx_minus1" ); // delta index of the Reference Picture Set used for prediction minus 1
    WRITE_CODE( (deltaRPS >=0 ? 0: 1), 1, "delta_rps_sign" ); //delta_rps_sign
    WRITE_UVLC( abs(deltaRPS) - 1, "abs_delta_rps_minus1"); // absolute delta RPS minus 1

    for(Int j=0; j < pcRPS->getNumRefIdc(); j++)
    {
      Int refIdc = pcRPS->getRefIdc(j);
      WRITE_CODE( (refIdc==1? 1: 0), 1, "ref_idc0" ); //first bit is "1" if Idc is 1 
      if (refIdc != 1) 
      {
        WRITE_CODE( refIdc>>1, 1, "ref_idc1" ); //second bit is "1" if Idc is 2, "0" otherwise.
      }
    }
  }
  else
  {
    WRITE_UVLC( pcRPS->getNumberOfNegativePictures(), "num_negative_pics" );
    WRITE_UVLC( pcRPS->getNumberOfPositivePictures(), "num_positive_pics" );
    Int prev = 0;
    for(Int j=0 ; j < pcRPS->getNumberOfNegativePictures(); j++)
    {
      WRITE_UVLC( prev-pcRPS->getDeltaPOC(j)-1, "delta_poc_s0_minus1" );
      prev = pcRPS->getDeltaPOC(j);
      WRITE_FLAG( pcRPS->getUsed(j), "used_by_curr_pic_s0_flag"); 
    }
    prev = 0;
    for(Int j=pcRPS->getNumberOfNegativePictures(); j < pcRPS->getNumberOfNegativePictures()+pcRPS->getNumberOfPositivePictures(); j++)
    {
      WRITE_UVLC( pcRPS->getDeltaPOC(j)-prev-1, "delta_poc_s1_minus1" );
      prev = pcRPS->getDeltaPOC(j);
      WRITE_FLAG( pcRPS->getUsed(j), "used_by_curr_pic_s1_flag" ); 
    }
  }

#if PRINT_RPS_INFO
  printf("irps=%d (%2d bits) ", pcRPS->getInterRPSPrediction(), getNumberOfWrittenBits() - lastBits);
  pcRPS->printDeltaPOC();
#endif
}


Void TEncCavlc::codePPS( TComPPS* pcPPS )
{
#if ENC_DEC_TRACE  
  xTracePPSHeader (pcPPS);
#endif
   TComRPS* pcRPSList = pcPPS->getRPSList();
  
  WRITE_UVLC( pcPPS->getPPSId(),                             "pic_parameter_set_id" );
  WRITE_UVLC( pcPPS->getSPSId(),                             "seq_parameter_set_id" );
  // RPS is put before entropy_coding_mode_flag
  // since entropy_coding_mode_flag will probably be removed from the WD
  TComReferencePictureSet*      pcRPS;

  WRITE_UVLC(pcRPSList->getNumberOfReferencePictureSets(), "num_short_term_ref_pic_sets" );
  for(UInt i=0; i < pcRPSList->getNumberOfReferencePictureSets(); i++)
  {
    pcRPS = pcRPSList->getReferencePictureSet(i);
    codeShortTermRefPicSet(pcPPS,pcRPS);
  }    
  WRITE_FLAG( pcPPS->getLongTermRefsPresent() ? 1 : 0,         "long_term_ref_pics_present_flag" );
  // entropy_coding_mode_flag
  // We code the entropy_coding_mode_flag, it's needed for tests.
  WRITE_FLAG( pcPPS->getEntropyCodingMode() ? 1 : 0,         "entropy_coding_mode_flag" );
  if (pcPPS->getEntropyCodingMode())
  {
    WRITE_UVLC( pcPPS->getEntropyCodingSynchro(),            "entropy_coding_synchro" );
    WRITE_FLAG( pcPPS->getCabacIstateReset() ? 1 : 0,        "cabac_istate_reset" );
    if ( pcPPS->getEntropyCodingSynchro() )
    {
      WRITE_UVLC( pcPPS->getNumSubstreams()-1,               "num_substreams_minus1" );
    }
  }
  WRITE_UVLC( pcPPS->getNumTLayerSwitchingFlags(),           "num_temporal_layer_switching_point_flags" );
  for( UInt i = 0; i < pcPPS->getNumTLayerSwitchingFlags(); i++ ) 
  {
    WRITE_FLAG( pcPPS->getTLayerSwitchingFlag( i ) ? 1 : 0 , "temporal_layer_switching_point_flag" ); 
  }
  //   num_ref_idx_l0_default_active_minus1
  //   num_ref_idx_l1_default_active_minus1
  WRITE_SVLC( pcPPS->getPicInitQPMinus26(),                  "pic_init_qp_minus26");
  WRITE_FLAG( pcPPS->getConstrainedIntraPred() ? 1 : 0,      "constrained_intra_pred_flag" );
  WRITE_FLAG( pcPPS->getEnableTMVPFlag() ? 1 : 0,            "enable_temporal_mvp_flag" );
  WRITE_CODE( pcPPS->getSliceGranularity(), 2,               "slice_granularity");
  WRITE_UVLC( pcPPS->getMaxCuDQPDepth() + pcPPS->getUseDQP(),                   "max_cu_qp_delta_depth" );

  WRITE_SVLC( pcPPS->getChromaQpOffset(),                   "chroma_qp_offset"     );
  WRITE_SVLC( pcPPS->getChromaQpOffset2nd(),                "chroma_qp_offset_2nd" );

  WRITE_FLAG( pcPPS->getUseWP() ? 1 : 0,  "weighted_pred_flag" );   // Use of Weighting Prediction (P_SLICE)
  WRITE_CODE( pcPPS->getWPBiPredIdc(), 2, "weighted_bipred_idc" );  // Use of Weighting Bi-Prediction (B_SLICE)

  WRITE_FLAG( pcPPS->getColumnRowInfoPresent(),           "tile_info_present_flag" );
  WRITE_FLAG( pcPPS->getTileBehaviorControlPresentFlag(),  "tile_control_present_flag");
  if( pcPPS->getColumnRowInfoPresent() == 1 )
  {
    WRITE_FLAG( pcPPS->getUniformSpacingIdr(),                                   "uniform_spacing_flag" );
    WRITE_UVLC( pcPPS->getNumColumnsMinus1(),                                    "num_tile_columns_minus1" );
    WRITE_UVLC( pcPPS->getNumRowsMinus1(),                                       "num_tile_rows_minus1" );
    if( pcPPS->getUniformSpacingIdr() == 0 )
    {
      for(UInt i=0; i<pcPPS->getNumColumnsMinus1(); i++)
      {
        WRITE_UVLC( pcPPS->getColumnWidth(i),                                    "column_width" );
      }
      for(UInt i=0; i<pcPPS->getNumRowsMinus1(); i++)
      {
        WRITE_UVLC( pcPPS->getRowHeight(i),                                      "row_height" );
      }
    }
  }
  if(pcPPS->getTileBehaviorControlPresentFlag() == 1)
  {
    Int iNumColTilesMinus1 = (pcPPS->getColumnRowInfoPresent() == 1)?(pcPPS->getNumColumnsMinus1()):(pcPPS->getSPS()->getNumColumnsMinus1());
    Int iNumRowTilesMinus1 = (pcPPS->getColumnRowInfoPresent() == 1)?(pcPPS->getNumColumnsMinus1()):(pcPPS->getSPS()->getNumRowsMinus1());

    if(iNumColTilesMinus1 !=0 || iNumRowTilesMinus1 !=0)
    {
      WRITE_FLAG( pcPPS->getTileBoundaryIndependenceIdr(),                         "tile_boundary_independence_flag" );
      if(pcPPS->getTileBoundaryIndependenceIdr() == 1)
      {
        WRITE_FLAG( pcPPS->getLFCrossTileBoundaryFlag()?1 : 0,            "loop_filter_across_tile_flag");
      }
    }
  }

  return;
}

Void TEncCavlc::codeSPS( TComSPS* pcSPS )
{
#if ENC_DEC_TRACE  
  xTraceSPSHeader (pcSPS);
#endif
  WRITE_CODE( pcSPS->getProfileIdc (),     8,       "profile_idc" );
  WRITE_CODE( 0,                           8,       "reserved_zero_8bits" );
  WRITE_CODE( pcSPS->getLevelIdc (),       8,       "level_idc" );
  WRITE_UVLC( pcSPS->getSPSId (),                   "seq_parameter_set_id" );
  WRITE_UVLC( pcSPS->getChromaFormatIdc (),         "chroma_format_idc" );
  WRITE_CODE( pcSPS->getMaxTLayers() - 1,  3,       "max_temporal_layers_minus1" );
  WRITE_UVLC( pcSPS->getWidth (),                   "pic_width_in_luma_samples" );
  WRITE_UVLC( pcSPS->getHeight(),                   "pic_height_in_luma_samples" );

#if FULL_NBIT
  WRITE_UVLC( pcSPS->getBitDepth() - 8,             "bit_depth_luma_minus8" );
#else
  WRITE_UVLC( pcSPS->getBitIncrement(),             "bit_depth_luma_minus8" );
#endif
#if FULL_NBIT
  WRITE_UVLC( pcSPS->getBitDepth() - 8,             "bit_depth_chroma_minus8" );
#else
  WRITE_UVLC( pcSPS->getBitIncrement(),             "bit_depth_chroma_minus8" );
#endif

  WRITE_FLAG( pcSPS->getUsePCM() ? 1 : 0,                   "pcm_enabled_flag");

  if( pcSPS->getUsePCM() )
  {
  WRITE_CODE( pcSPS->getPCMBitDepthLuma() - 1, 4,   "pcm_bit_depth_luma_minus1" );
  WRITE_CODE( pcSPS->getPCMBitDepthChroma() - 1, 4, "pcm_bit_depth_chroma_minus1" );
  }
  WRITE_UVLC( pcSPS->getBitsForPOC()-4,                 "log2_max_pic_order_cnt_lsb_minus4" );
  WRITE_UVLC( pcSPS->getMaxNumberOfReferencePictures(), "max_num_ref_pics" ); 
  WRITE_UVLC( pcSPS->getNumReorderFrames(),             "num_reorder_frames" ); 
  WRITE_UVLC(pcSPS->getMaxDecFrameBuffering(),          "max_dec_frame_buffering" );
  WRITE_UVLC(pcSPS->getMaxLatencyIncrease(),            "max_latency_increase"    );
  assert( pcSPS->getMaxCUWidth() == pcSPS->getMaxCUHeight() );
  
  UInt MinCUSize = pcSPS->getMaxCUWidth() >> ( pcSPS->getMaxCUDepth()-g_uiAddCUDepth );
  UInt log2MinCUSize = 0;
  while(MinCUSize > 1)
  {
    MinCUSize >>= 1;
    log2MinCUSize++;
  }

  WRITE_UVLC( log2MinCUSize - 3,                                                     "log2_min_coding_block_size_minus3" );
  WRITE_UVLC( pcSPS->getMaxCUDepth()-g_uiAddCUDepth,                                 "log2_diff_max_min_coding_block_size" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MinSize() - 2,                                 "log2_min_transform_block_size_minus2" );
  WRITE_UVLC( pcSPS->getQuadtreeTULog2MaxSize() - pcSPS->getQuadtreeTULog2MinSize(), "log2_diff_max_min_transform_block_size" );

  if(log2MinCUSize == 3)
  {
    xWriteFlag  ( (pcSPS->getDisInter4x4()) ? 1 : 0 );
  }

  if( pcSPS->getUsePCM() )
  {
  WRITE_UVLC( pcSPS->getPCMLog2MinSize() - 3,                                        "log2_min_pcm_coding_block_size_minus3" );

  WRITE_UVLC( pcSPS->getPCMLog2MaxSize() - pcSPS->getPCMLog2MinSize(),               "log2_diff_max_min_pcm_coding_block_size" );
  }
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthInter() - 1,                               "max_transform_hierarchy_depth_inter" );
  WRITE_UVLC( pcSPS->getQuadtreeTUMaxDepthIntra() - 1,                               "max_transform_hierarchy_depth_intra" );
  WRITE_FLAG( (pcSPS->getScalingListFlag()) ? 1 : 0,                                  "scaling_list_enabled_flag" ); 
  WRITE_FLAG  ( (pcSPS->getUseLMChroma ()) ? 1 : 0,                                  "chroma_pred_from_luma_enabled_flag" ); 
  WRITE_FLAG( pcSPS->getUseDF() ? 1 : 0,                                             "deblocking_filter_in_aps_enabled_flag");
  WRITE_FLAG( pcSPS->getLFCrossSliceBoundaryFlag()?1 : 0,                            "loop_filter_across_slice_flag");
  WRITE_FLAG( pcSPS->getUseSAO() ? 1 : 0,                                            "sample_adaptive_offset_enabled_flag");
  WRITE_FLAG( (pcSPS->getUseALF ()) ? 1 : 0,                                         "adaptive_loop_filter_enabled_flag");

  if( pcSPS->getUsePCM() )
  {
  WRITE_FLAG( pcSPS->getPCMFilterDisableFlag()?1 : 0,                                "pcm_loop_filter_disable_flag");
  }

  assert( pcSPS->getMaxTLayers() > 0 );         

  WRITE_FLAG( pcSPS->getTemporalIdNestingFlag() ? 1 : 0,                             "temporal_id_nesting_flag" );

  //!!!KS: Syntax not in WD !!!
  
  xWriteUvlc  ( pcSPS->getPad (0) );
  xWriteUvlc  ( pcSPS->getPad (1) );

  // Tools
  xWriteFlag  ( (pcSPS->getUseMRG ()) ? 1 : 0 ); // SOPH:
  
  // AMVP mode for each depth
  for (Int i = 0; i < pcSPS->getMaxCUDepth(); i++)
  {
    xWriteFlag( pcSPS->getAMVPMode(i) ? 1 : 0);
  }

  WRITE_FLAG( pcSPS->getUniformSpacingIdr(),                          "uniform_spacing_flag" );
  WRITE_UVLC( pcSPS->getNumColumnsMinus1(),                           "num_tile_columns_minus1" );
  WRITE_UVLC( pcSPS->getNumRowsMinus1(),                              "num_tile_rows_minus1" );
  if( pcSPS->getUniformSpacingIdr()==0 )
  {
    for(UInt i=0; i<pcSPS->getNumColumnsMinus1(); i++)
    {
      WRITE_UVLC( pcSPS->getColumnWidth(i),                           "column_width" );
    }
    for(UInt i=0; i<pcSPS->getNumRowsMinus1(); i++)
    {
      WRITE_UVLC( pcSPS->getRowHeight(i),                             "row_height" );
    }
  }

  if( pcSPS->getNumColumnsMinus1() !=0 || pcSPS->getNumRowsMinus1() != 0)
  {
    WRITE_FLAG( pcSPS->getTileBoundaryIndependenceIdr(),                "tile_boundary_independence_flag" );
    if(pcSPS->getTileBoundaryIndependenceIdr() == 1)
    {
      WRITE_FLAG( pcSPS->getLFCrossTileBoundaryFlag()?1 : 0,            "loop_filter_across_tile_flag");
    }
  }

  // Software-only flags
  WRITE_FLAG( pcSPS->getUseNSQT(), "enable_nsqt" );
  WRITE_FLAG( pcSPS->getUseAMP(), "enable_amp" );
}

Void TEncCavlc::writeTileMarker( UInt uiTileIdx, UInt uiBitsUsed )
{
  xWriteCode( uiTileIdx, uiBitsUsed );
}

Void TEncCavlc::codeSliceHeader         ( TComSlice* pcSlice )
{
#if ENC_DEC_TRACE  
  xTraceSliceHeader (pcSlice);
#endif

  // if( nal_ref_idc != 0 )
  //   dec_ref_pic_marking( )
  // if( entropy_coding_mode_flag  &&  slice_type  !=  I)
  //   cabac_init_idc
  // first_slice_in_pic_flag
  // if( first_slice_in_pic_flag == 0 )
  //    slice_address
  //calculate number of bits required for slice address
  Int maxAddrOuter = pcSlice->getPic()->getNumCUsInFrame();
  Int reqBitsOuter = 0;
  while(maxAddrOuter>(1<<reqBitsOuter)) 
  {
    reqBitsOuter++;
  }
  Int maxAddrInner = pcSlice->getPic()->getNumPartInCU()>>(2);
  maxAddrInner = (1<<(pcSlice->getPPS()->getSliceGranularity()<<1));
  Int reqBitsInner = 0;
  
  while(maxAddrInner>(1<<reqBitsInner))
  {
    reqBitsInner++;
  }
  Int lCUAddress;
  Int innerAddress;
  if (pcSlice->isNextSlice())
  {
    // Calculate slice address
    lCUAddress = (pcSlice->getSliceCurStartCUAddr()/pcSlice->getPic()->getNumPartInCU());
    innerAddress = (pcSlice->getSliceCurStartCUAddr()%(pcSlice->getPic()->getNumPartInCU()))>>((pcSlice->getSPS()->getMaxCUDepth()-pcSlice->getPPS()->getSliceGranularity())<<1);
  }
  else
  {
    // Calculate slice address
    lCUAddress = (pcSlice->getEntropySliceCurStartCUAddr()/pcSlice->getPic()->getNumPartInCU());
    innerAddress = (pcSlice->getEntropySliceCurStartCUAddr()%(pcSlice->getPic()->getNumPartInCU()))>>((pcSlice->getSPS()->getMaxCUDepth()-pcSlice->getPPS()->getSliceGranularity())<<1);
    
  }
  //write slice address
  Int address = (pcSlice->getPic()->getPicSym()->getCUOrderMap(lCUAddress) << reqBitsInner) + innerAddress;
  WRITE_FLAG( address==0, "first_slice_in_pic_flag" );
  if(address>0) 
  {
    WRITE_CODE( address, reqBitsOuter+reqBitsInner, "slice_address" );
  }

  WRITE_UVLC( pcSlice->getSliceType(),       "slice_type" );
  Bool bEntropySlice = (!pcSlice->isNextSlice());
  WRITE_FLAG( bEntropySlice ? 1 : 0, "lightweight_slice_flag" );
  
  if (!bEntropySlice)
  {
    WRITE_UVLC( pcSlice->getPPS()->getPPSId(), "pic_parameter_set_id" );
    if(pcSlice->getNalUnitType()==NAL_UNIT_CODED_SLICE_IDR) 
    {
      WRITE_UVLC( 0, "idr_pic_id" );
      WRITE_FLAG( 0, "no_output_of_prior_pics_flag" );
    }
    else
    {
      WRITE_CODE( (pcSlice->getPOC()-pcSlice->getLastIDR()+(1<<pcSlice->getSPS()->getBitsForPOC()))%(1<<pcSlice->getSPS()->getBitsForPOC()), pcSlice->getSPS()->getBitsForPOC(), "pic_order_cnt_lsb");
      TComReferencePictureSet* pcRPS = pcSlice->getRPS();
      if(pcSlice->getRPSidx() < 0)
      {
        WRITE_FLAG( 0, "short_term_ref_pic_set_pps_flag");
        codeShortTermRefPicSet(pcSlice->getPPS(), pcRPS);
      }
      else
      {
        WRITE_FLAG( 1, "short_term_ref_pic_set_pps_flag");
        WRITE_UVLC( pcSlice->getRPSidx(), "short_term_ref_pic_set_idx" );
      }
      if(pcSlice->getPPS()->getLongTermRefsPresent())
      {
        WRITE_UVLC( pcRPS->getNumberOfLongtermPictures(), "num_long_term_pics");
        Int maxPocLsb = 1<<pcSlice->getSPS()->getBitsForPOC();
        Int prev = 0;
        for(Int i=pcRPS->getNumberOfPictures()-1 ; i > pcRPS->getNumberOfPictures()-pcRPS->getNumberOfLongtermPictures()-1; i--)
        {
          WRITE_UVLC((maxPocLsb-pcRPS->getDeltaPOC(i)+prev-1)%maxPocLsb, "delta_poc_lsb_lt_minus1");
          prev = pcRPS->getDeltaPOC(i);
          WRITE_FLAG( pcRPS->getUsed(i), "used_by_curr_pic_lt_flag"); 
        }
      }
    }
    if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF()|| pcSlice->getSPS()->getScalingListFlag() || pcSlice->getSPS()->getUseDF())
    {
      if (pcSlice->getSPS()->getUseALF())
      {
         if (pcSlice->getAlfEnabledFlag())
         {
           assert (pcSlice->getAPS()->getAlfEnabled());
         }
         WRITE_FLAG( pcSlice->getAlfEnabledFlag(), "ALF on/off flag in slice header" );
      }
      if (pcSlice->getSPS()->getUseSAO())
      {
         assert (pcSlice->getSaoEnabledFlag() == pcSlice->getAPS()->getSaoEnabled());
         WRITE_FLAG( pcSlice->getSaoEnabledFlag(), "SAO on/off flag in slice header" );
      }
      WRITE_UVLC( pcSlice->getAPS()->getAPSID(), "aps_id");
    }

    // we always set num_ref_idx_active_override_flag equal to one. this might be done in a more intelligent way 
    if (!pcSlice->isIntra())
    {
      WRITE_FLAG( 1 ,                                             "num_ref_idx_active_override_flag");
      WRITE_CODE( pcSlice->getNumRefIdx( REF_PIC_LIST_0 ) - 1, 3, "num_ref_idx_l0_active_minus1" );
    }
    else
    {
      pcSlice->setNumRefIdx(REF_PIC_LIST_0, 0);
    }
    if (pcSlice->isInterB())
    {
      WRITE_CODE( pcSlice->getNumRefIdx( REF_PIC_LIST_1 ) - 1, 3, "num_ref_idx_l1_active_minus1" );
    }
    else
    {
      pcSlice->setNumRefIdx(REF_PIC_LIST_1, 0);
    }
    TComRefPicListModification* refPicListModification = pcSlice->getRefPicListModification();
    if(!pcSlice->isIntra())
    {
      WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0() ? 1 : 0,       "ref_pic_list_modification_flag" );    
      for(Int i = 0; i < refPicListModification->getNumberOfRefPicListModificationsL0(); i++)
      {
        WRITE_UVLC( refPicListModification->getListIdcL0(i), "ref_pic_list_modification_idc");
        WRITE_UVLC( refPicListModification->getRefPicSetIdxL0(i), "ref_pic_set_idx");
      }
      if(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL0())
        WRITE_UVLC( 3, "ref_pic_list_modification_idc");
    }
    if(pcSlice->isInterB())
    {    
      WRITE_FLAG(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1() ? 1 : 0,       "ref_pic_list_modification_flag" );
      for(Int i = 0; i < refPicListModification->getNumberOfRefPicListModificationsL1(); i++)
      {
        WRITE_UVLC( refPicListModification->getListIdcL1(i), "ref_pic_list_modification_idc");
        WRITE_UVLC( refPicListModification->getRefPicSetIdxL1(i), "ref_pic_set_idx");
      }
      if(pcSlice->getRefPicListModification()->getRefPicListModificationFlagL1())
        WRITE_UVLC( 3, "ref_pic_list_modification_idc");
    }
  }
  // ref_pic_list_combination( )
  // maybe move to own function?
  if (pcSlice->isInterB())
  {
    WRITE_FLAG(pcSlice->getRefPicListCombinationFlag() ? 1 : 0,       "ref_pic_list_combination_flag" );
    if(pcSlice->getRefPicListCombinationFlag())
    {
      WRITE_UVLC( pcSlice->getNumRefIdx(REF_PIC_LIST_C) - 1,          "num_ref_idx lc_active_minus1");
      
      WRITE_FLAG( pcSlice->getRefPicListModificationFlagLC() ? 1 : 0, "ref_pic_list_modification_flag_lc" );
      if(pcSlice->getRefPicListModificationFlagLC())
      {
        for (UInt i=0;i<pcSlice->getNumRefIdx(REF_PIC_LIST_C);i++)
        {
          WRITE_FLAG( pcSlice->getListIdFromIdxOfLC(i),               "pic_from_list_0_flag" );
          WRITE_UVLC( pcSlice->getRefIdxFromIdxOfLC(i),               "ref_idx_list_curr" );
        }
      }
    }
  }
    
  if(pcSlice->getPPS()->getEntropyCodingMode() && !pcSlice->isIntra())
  {
    WRITE_UVLC(pcSlice->getCABACinitIDC(),  "cabac_init_idc");
  }

  // if( !lightweight_slice_flag ) {
  if (!bEntropySlice)
  {
    Int iCode = pcSlice->getSliceQp() - ( pcSlice->getPPS()->getPicInitQPMinus26() + 26 );
    WRITE_SVLC( iCode, "slice_qp_delta" ); 
  //   if( sample_adaptive_offset_enabled_flag )
  //     sao_param()
  //   if( deblocking_filter_control_present_flag ) {
  //     disable_deblocking_filter_idc
    WRITE_FLAG(pcSlice->getInheritDblParamFromAPS(), "inherit_dbl_param_from_APS_flag");
    if (!pcSlice->getInheritDblParamFromAPS())
    {
      WRITE_FLAG(pcSlice->getLoopFilterDisable(), "loop_filter_disable");  // should be an IDC
      if(!pcSlice->getLoopFilterDisable())
      {
        WRITE_SVLC (pcSlice->getLoopFilterBetaOffset(), "beta_offset_div2");
        WRITE_SVLC (pcSlice->getLoopFilterTcOffset(), "tc_offset_div2");
      }
    }
  //     if( disable_deblocking_filter_idc  !=  1 ) {
  //       slice_alpha_c0_offset_div2
  //       slice_beta_offset_div2
  //     }
  //   }
  //   if( slice_type = = B )
  //   collocated_from_l0_flag
    if ( pcSlice->getSliceType() == B_SLICE )
    {
      WRITE_FLAG( pcSlice->getColDir(), "collocated_from_l0_flag" );
    }
    //   if( adaptive_loop_filter_enabled_flag ) {
  //     if( !shared_pps_info_enabled_flag )
  //       alf_param( )
  //     alf_cu_control_param( )
  //   }
  // }
  
    if ( (pcSlice->getPPS()->getUseWP() && pcSlice->getSliceType()==P_SLICE) || (pcSlice->getPPS()->getWPBiPredIdc()==1 && pcSlice->getSliceType()==B_SLICE) )
    {
      xCodePredWeightTable( pcSlice );
    }
  }

  // !!!! sytnax elements not in the WD !!!!
  
  assert(pcSlice->getMaxNumMergeCand()<=MRG_MAX_NUM_CANDS_SIGNALED);
  assert(MRG_MAX_NUM_CANDS_SIGNALED<=MRG_MAX_NUM_CANDS);
  WRITE_UVLC(MRG_MAX_NUM_CANDS - pcSlice->getMaxNumMergeCand(), "maxNumMergeCand");
}


Void TEncCavlc::codeTileMarkerFlag(TComSlice* pcSlice) 
{
  Bool bEntropySlice = (!pcSlice->isNextSlice());
  if (!bEntropySlice)
  {
    xWriteFlag  (pcSlice->getTileMarkerFlag() ? 1 : 0 );
  }
}

/**
 - write wavefront substreams sizes for the slice header.
 .
 \param pcSlice Where we find the substream size information.
 */
Void TEncCavlc::codeSliceHeaderSubstreamTable( TComSlice* pcSlice )
{
  UInt uiNumSubstreams = pcSlice->getPPS()->getNumSubstreams();
  UInt*puiSubstreamSizes = pcSlice->getSubstreamSizes();

  // Write header information for all substreams except the last.
  for (UInt ui = 0; ui+1 < uiNumSubstreams; ui++)
  {
    UInt uiNumbits = puiSubstreamSizes[ui];

    //the 2 first bits are used to give the size of the header
    if ( uiNumbits < (1<<8) )
    {
      xWriteCode(0,         2  );
      xWriteCode(uiNumbits, 8  );
    }
    else if ( uiNumbits < (1<<16) )
    {
      xWriteCode(1,         2  );
      xWriteCode(uiNumbits, 16 );
    }
    else if ( uiNumbits < (1<<24) )
    {
      xWriteCode(2,         2  );
      xWriteCode(uiNumbits, 24 );
    }
    else if ( uiNumbits < (1<<31) )
    {
      xWriteCode(3,         2  );
      xWriteCode(uiNumbits, 32 );
    }
    else
    {
      printf("Error in codeSliceHeaderTable\n");
      exit(-1);
    }
  }
}

Void TEncCavlc::codeTerminatingBit      ( UInt uilsLast )
{
}

Void TEncCavlc::codeSliceFinish ()
{
}

Void TEncCavlc::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert(0);
}

Void TEncCavlc::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TEncCavlc::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeMergeFlag    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{  
  if (!m_bAlfCtrl)
  {
    return;
  }
  
  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }
  
  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;
  
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfCtrlDepth()
{  
  if (!m_bAlfCtrl)
  {
    return;
  }
  
  UInt uiDepth = m_uiMaxAlfCtrlDepth;
  
#if !PARAMSET_VLC_CLEANUP
  xWriteUnaryMaxSymbol(uiDepth, g_uiMaxCUDepth-1);
#else
  xWriteUvlc(uiDepth);
#endif
}

Void TEncCavlc::codeInterModeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiEncMode )
{
  assert(0);
}

Void TEncCavlc::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert(0);
}

Void TEncCavlc::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  assert(0);
}

Void TEncCavlc::codeQtCbf( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  assert(0);
}

Void TEncCavlc::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

/** Code I_PCM information. 
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \returns Void
 *
 * If I_PCM flag indicates that the CU is I_PCM, code its PCM alignment bits and codes.  
 */
Void TEncCavlc::codeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx)
{
  UInt uiIPCM = (pcCU->getIPCMFlag(uiAbsPartIdx) == true)? 1 : 0;

  xWriteFlag(uiIPCM);

  if (uiIPCM)
  {
    xWritePCMAlignZero();

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
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        xWriteCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCb() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        xWriteCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }

    piPCMSample = pcCU->getPCMSampleCr() + uiChromaOffset;
    uiWidth = pcCU->getWidth(uiAbsPartIdx)/2;
    uiHeight = pcCU->getHeight(uiAbsPartIdx)/2;
    uiSampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();

    for(uiY = 0; uiY < uiHeight; uiY++)
    {
      for(uiX = 0; uiX < uiWidth; uiX++)
      {
        UInt uiSample = piPCMSample[uiX];

        xWriteCode(uiSample, uiSampleBits);
      }
      piPCMSample += uiWidth;
    }
  }
}

Void TEncCavlc::codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  assert(0);
}

Void TEncCavlc::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert(0);
}

Void TEncCavlc::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  assert(0);
}

Void TEncCavlc::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getRefQP( uiAbsPartIdx );

  xWriteSvlc( iDQp );
  
  return;
}

Void TEncCavlc::codeCoeffNxN    ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType )
{
  assert(0);
}

Void TEncCavlc::codeAlfFlag( UInt uiCode )
{  
  xWriteFlag( uiCode );
}

Void TEncCavlc::codeAlfCtrlFlag( UInt uiSymbol )
{
  xWriteFlag( uiSymbol );
}

Void TEncCavlc::codeAlfUvlc( UInt uiCode )
{
  xWriteUvlc( uiCode );
}

Void TEncCavlc::codeAlfSvlc( Int iCode )
{
  xWriteSvlc( iCode );
}

Void TEncCavlc::codeSaoFlag( UInt uiCode )
{
  xWriteFlag( uiCode );
}

Void TEncCavlc::codeSaoUvlc( UInt uiCode )
{
    xWriteUvlc( uiCode );
}

Void TEncCavlc::codeSaoSvlc( Int iCode )
{
    xWriteSvlc( iCode );
}

Void TEncCavlc::estBit( estBitsSbacStruct* pcEstBitsCabac, Int width, Int height, TextType eTType )
{
  // printf("error : no VLC mode support in this version\n");
  return;
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncCavlc::xWriteCode     ( UInt uiCode, UInt uiLength )
{
  assert ( uiLength > 0 );
  m_pcBitIf->write( uiCode, uiLength );
}

Void TEncCavlc::xWriteUvlc     ( UInt uiCode )
{
  UInt uiLength = 1;
  UInt uiTemp = ++uiCode;
  
  assert ( uiTemp );
  
  while( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }
  
  //m_pcBitIf->write( uiCode, uiLength );
  // Take care of cases where uiLength > 32
  m_pcBitIf->write( 0, uiLength >> 1);
  m_pcBitIf->write( uiCode, (uiLength+1) >> 1);
}

Void TEncCavlc::xWriteSvlc     ( Int iCode )
{
  UInt uiCode;
  
  uiCode = xConvertToUInt( iCode );
  xWriteUvlc( uiCode );
}

Void TEncCavlc::xWriteFlag( UInt uiCode )
{
  m_pcBitIf->write( uiCode, 1 );
}

/** Write PCM alignment bits. 
 * \returns Void
 */
Void  TEncCavlc::xWritePCMAlignZero    ()
{
  m_pcBitIf->writeAlignZero();
}

Void TEncCavlc::xWriteUnaryMaxSymbol( UInt uiSymbol, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    return;
  }
  xWriteFlag( uiSymbol ? 1 : 0 );
  if ( uiSymbol == 0 )
  {
    return;
  }
  
  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );
  
  while( --uiSymbol )
  {
    xWriteFlag( 1 );
  }
  if( bCodeLast )
  {
    xWriteFlag( 0 );
  }
  return;
}

Void TEncCavlc::xWriteExGolombLevel( UInt uiSymbol )
{
  if( uiSymbol )
  {
    xWriteFlag( 1 );
    UInt uiCount = 0;
    Bool bNoExGo = (uiSymbol < 13);
    
    while( --uiSymbol && ++uiCount < 13 )
    {
      xWriteFlag( 1 );
    }
    if( bNoExGo )
    {
      xWriteFlag( 0 );
    }
    else
    {
      xWriteEpExGolomb( uiSymbol, 0 );
    }
  }
  else
  {
    xWriteFlag( 0 );
  }
  return;
}

Void TEncCavlc::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    xWriteFlag( 1 );
    uiSymbol -= 1<<uiCount;
    uiCount  ++;
  }
  xWriteFlag( 0 );
  while( uiCount-- )
  {
    xWriteFlag( (uiSymbol>>uiCount) & 1 );
  }
  return;
}

/** code explicit wp tables
 * \param TComSlice* pcSlice
 * \returns Void
 */
Void TEncCavlc::xCodePredWeightTable( TComSlice* pcSlice )
{
  wpScalingParam  *wp;
  Bool            bChroma     = true; // color always present in HEVC ?
  Int             iNbRef       = (pcSlice->getSliceType() == B_SLICE ) ? (2) : (1);
  Bool            bDenomCoded  = false;

  UInt            uiMode = 0;
  if ( (pcSlice->getSliceType()==P_SLICE && pcSlice->getPPS()->getUseWP()) || (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==1 && pcSlice->getRefPicListCombinationFlag()==0 ) )
    uiMode = 1; // explicit
  else if ( pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==2 )
    uiMode = 2; // implicit (does not use this mode in this syntax)
  if (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==1 && pcSlice->getRefPicListCombinationFlag())
    uiMode = 3; // combined explicit
  if(uiMode == 1)
  {
    for ( Int iNumRef=0 ; iNumRef<iNbRef ; iNumRef++ ) 
    {
      RefPicList  eRefPicList = ( iNumRef ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
      for ( Int iRefIdx=0 ; iRefIdx<pcSlice->getNumRefIdx(eRefPicList) ; iRefIdx++ ) 
      {
        pcSlice->getWpScaling(eRefPicList, iRefIdx, wp);
        if ( !bDenomCoded ) 
        {
          Int iDeltaDenom;
          WRITE_UVLC( wp[0].uiLog2WeightDenom, "luma_log2_weight_denom" );     // ue(v): luma_log2_weight_denom

          if( bChroma )
          {
            iDeltaDenom = (wp[1].uiLog2WeightDenom - wp[0].uiLog2WeightDenom);
            WRITE_SVLC( iDeltaDenom, "delta_chroma_log2_weight_denom" );       // se(v): delta_chroma_log2_weight_denom
          }
          bDenomCoded = true;
        }

        WRITE_FLAG( wp[0].bPresentFlag, "luma_weight_lX_flag" );               // u(1): luma_weight_lX_flag

        if ( wp[0].bPresentFlag ) 
        {
          Int iDeltaWeight = (wp[0].iWeight - (1<<wp[0].uiLog2WeightDenom));
          WRITE_SVLC( iDeltaWeight, "delta_luma_weight_lX" );                  // se(v): delta_luma_weight_lX
          WRITE_SVLC( wp[0].iOffset, "luma_offset_lX" );                       // se(v): luma_offset_lX
        }

        if ( bChroma ) 
        {
          WRITE_FLAG( wp[1].bPresentFlag, "chroma_weight_lX_flag" );           // u(1): chroma_weight_lX_flag

          if ( wp[1].bPresentFlag )
          {
            for ( Int j=1 ; j<3 ; j++ ) 
            {
              Int iDeltaWeight = (wp[j].iWeight - (1<<wp[1].uiLog2WeightDenom));
              WRITE_SVLC( iDeltaWeight, "delta_chroma_weight_lX" );            // se(v): delta_chroma_weight_lX

              Int iDeltaChroma = (wp[j].iOffset + ( ( (g_uiIBDI_MAX>>1)*wp[j].iWeight)>>(wp[j].uiLog2WeightDenom) ) - (g_uiIBDI_MAX>>1));
              WRITE_SVLC( iDeltaChroma, "delta_chroma_offset_lX" );            // se(v): delta_chroma_offset_lX
            }
          }
        }
      }
    }
  }
  else if (uiMode == 3)
  {
    for ( Int iRefIdx=0 ; iRefIdx<pcSlice->getNumRefIdx(REF_PIC_LIST_C) ; iRefIdx++ ) 
    {
      RefPicList  eRefPicList = (RefPicList)pcSlice->getListIdFromIdxOfLC(iRefIdx);
      Int iCombRefIdx = pcSlice->getRefIdxFromIdxOfLC(iRefIdx);

      pcSlice->getWpScaling(eRefPicList, iCombRefIdx, wp);
      if ( !bDenomCoded ) 
      {
        Int iDeltaDenom;
        WRITE_UVLC( wp[0].uiLog2WeightDenom, "luma_log2_weight_denom" );       // ue(v): luma_log2_weight_denom

        if( bChroma )
        {
          iDeltaDenom = (wp[1].uiLog2WeightDenom - wp[0].uiLog2WeightDenom);
          WRITE_SVLC( iDeltaDenom, "delta_chroma_log2_weight_denom" );         // se(v): delta_chroma_log2_weight_denom
        }
        bDenomCoded = true;
      }

      WRITE_FLAG( wp[0].bPresentFlag, "luma_weight_lc_flag" );                 // u(1): luma_weight_lc_flag

      if ( wp[0].bPresentFlag ) 
      {
        Int iDeltaWeight = (wp[0].iWeight - (1<<wp[0].uiLog2WeightDenom));
        WRITE_SVLC( iDeltaWeight, "delta_luma_weight_lc" );                    // se(v): delta_luma_weight_lc
        WRITE_SVLC( wp[0].iOffset, "luma_offset_lc" );                         // se(v): luma_offset_lc
      }
      if ( bChroma ) 
      {
        WRITE_FLAG( wp[1].bPresentFlag, "chroma_weight_lc_flag" );             // u(1): luma_weight_lc_flag

        if ( wp[1].bPresentFlag )
        {
          for ( Int j=1 ; j<3 ; j++ ) 
          {
            Int iDeltaWeight = (wp[j].iWeight - (1<<wp[1].uiLog2WeightDenom));
            WRITE_SVLC( iDeltaWeight, "delta_chroma_weight_lc" );              // se(v): delta_chroma_weight_lc

            Int iDeltaChroma = (wp[j].iOffset + ( ( (g_uiIBDI_MAX>>1)*wp[j].iWeight)>>(wp[j].uiLog2WeightDenom) ) - (g_uiIBDI_MAX>>1));
            WRITE_SVLC( iDeltaChroma, "delta_chroma_offset_lc" );              // se(v): delta_chroma_offset_lc
          }
        }
      }
    }
  }
}

/** code quantization matrix
 *  \param scalingList quantization matrix information
 */
Void TEncCavlc::codeScalingList( TComScalingList* scalingList )
{
  UInt listId,sizeId;
  Int *dst=0;
#if SCALING_LIST_OUTPUT_RESULT
  Int *org=0;
  Int avg_error = 0;
  Int max_error = 0;
  Int startBit;
  Int startTotalBit;
  startBit = m_pcBitIf->getNumberOfWrittenBits();
  startTotalBit = m_pcBitIf->getNumberOfWrittenBits();
#endif

  WRITE_FLAG( scalingList->getUseDefaultOnlyFlag (), "use_default_scaling_list_flag" );

  if(scalingList->getUseDefaultOnlyFlag () == false)
  {
#if SCALING_LIST_OUTPUT_RESULT
    printf("Header Bit %d\n",m_pcBitIf->getNumberOfWrittenBits()-startBit);
#endif
    //for each size
    for(sizeId = 0; sizeId < SCALING_LIST_SIZE_NUM; sizeId++)
    {
      for(listId = 0; listId < g_auiScalingListNum[sizeId]; listId++)
      {
        dst = scalingList->getScalingListAddress(sizeId,listId);
#if SCALING_LIST_OUTPUT_RESULT
        org = scalingList->getScalingListOrgAddress(sizeId,listId);
        for(int i=0;i<g_scalingListSize[sizeId];i++)
        {
          avg_error += abs(dst[i] - org[i]);
          if(abs(max_error) < abs(dst[i] - org[i]))
          {
            max_error = dst[i] - org[i];
          }
        }
        startBit = m_pcBitIf->getNumberOfWrittenBits();
#endif
        WRITE_FLAG( scalingList->getPredMode (sizeId,listId), "pred_mode_flag" );
        if(scalingList->getPredMode (sizeId,listId) == SCALING_LIST_PRED_COPY)//Copy Mode
        {
          WRITE_UVLC( (Int)listId - (Int)scalingList->getPredMatrixId (sizeId,listId) - 1, "pred_matrix_id_delta");
          scalingList->xPredScalingListMatrix( scalingList, dst, sizeId, listId,sizeId, scalingList->getPredMatrixId (sizeId,listId));       
        }
        else if(scalingList->getPredMode (sizeId,listId) == SCALING_LIST_PRED_DPCM)//DPCM Mode
        {
          xCodeDPCMScalingListMatrix(scalingList, dst,sizeId);
        }
#if SCALING_LIST_OUTPUT_RESULT
        printf("Matrix [%d][%d] Bit %d\n",sizeId,listId,m_pcBitIf->getNumberOfWrittenBits() - startBit);
#endif
      }
    }
  }
#if SCALING_LIST_OUTPUT_RESULT
  else
  {
    printf("Header Bit %d\n",m_pcBitIf->getNumberOfWrittenBits()-startTotalBit);
  }
  printf("Total Bit %d\n",m_pcBitIf->getNumberOfWrittenBits()-startTotalBit);
  printf("MaxError %d\n",abs(max_error));
  printf("AvgError %lf\n",(double)avg_error/(double)((16+64+256)*6+(1024*2)));
#endif
  return;
}
/** code DPCM with quantization matrix
 * \param scalingList quantization matrix information
 * \param piData matrix data
 * \param uiSizeId size index
 */
Void TEncCavlc::xCodeDPCMScalingListMatrix(TComScalingList* scalingList, Int* piData, UInt uiSizeId)
{
  Int dpcm[1024];
  UInt uiDataCounter = g_scalingListSize[uiSizeId];

  //make DPCM
  scalingList->xMakeDPCM(piData, piData, dpcm, uiSizeId);
  xWriteResidualCode(uiDataCounter,dpcm);
}
/** write resiidual code
 * \param uiSize side index
 * \param data residual coefficient
 */
Void TEncCavlc::xWriteResidualCode(UInt uiSize, Int *data)
{
  for(UInt i=0;i<uiSize;i++)
  {
    WRITE_SVLC( data[i],  "delta_coef");
  }
}

//! \}
