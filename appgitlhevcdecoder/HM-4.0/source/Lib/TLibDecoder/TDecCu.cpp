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

/** \file     TDecCu.cpp
    \brief    CU decoder class
*/

#include "TDecCu.h"
#if ENABLE_ANAYSIS_OUTPUT
#include "TLibSysuAnalyzer/TSysuAnalyzerOutput.h"
#endif
//! \ingroup TLibDecoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TDecCu::TDecCu()
{
  m_ppcYuvResi = NULL;
  m_ppcYuvReco = NULL;
  m_ppcCU      = NULL;
}

TDecCu::~TDecCu()
{
}

Void TDecCu::init( TDecEntropy* pcEntropyDecoder, TComTrQuant* pcTrQuant, TComPrediction* pcPrediction)
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcTrQuant         = pcTrQuant;
  m_pcPrediction      = pcPrediction;
}

/**
 \param    uiMaxDepth    total number of allowable depth
 \param    uiMaxWidth    largest CU width
 \param    uiMaxHeight   largest CU height
 */
Void TDecCu::create( UInt uiMaxDepth, UInt uiMaxWidth, UInt uiMaxHeight )
{
  m_uiMaxDepth = uiMaxDepth+1;
  
  m_ppcYuvResi = new TComYuv*[m_uiMaxDepth-1];
  m_ppcYuvReco = new TComYuv*[m_uiMaxDepth-1];
  m_ppcCU      = new TComDataCU*[m_uiMaxDepth-1];
  
  UInt uiNumPartitions;
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    uiNumPartitions = 1<<( ( m_uiMaxDepth - ui - 1 )<<1 );
    UInt uiWidth  = uiMaxWidth  >> ui;
    UInt uiHeight = uiMaxHeight >> ui;
    
    m_ppcYuvResi[ui] = new TComYuv;    m_ppcYuvResi[ui]->create( uiWidth, uiHeight );
    m_ppcYuvReco[ui] = new TComYuv;    m_ppcYuvReco[ui]->create( uiWidth, uiHeight );
    m_ppcCU     [ui] = new TComDataCU; m_ppcCU     [ui]->create( uiNumPartitions, uiWidth, uiHeight, true, uiMaxWidth >> (m_uiMaxDepth - 1) );
  }
  
  m_bDecodeDQP = false;

  // initialize partition order.
  UInt* piTmp = &g_auiZscanToRaster[0];
  initZscanToRaster(m_uiMaxDepth, 1, 0, piTmp);
  initRasterToZscan( uiMaxWidth, uiMaxHeight, m_uiMaxDepth );
  
  // initialize conversion matrix from partition index to pel
  initRasterToPelXY( uiMaxWidth, uiMaxHeight, m_uiMaxDepth );
#if REDUCE_UPPER_MOTION_DATA
  initMotionReferIdx ( uiMaxWidth, uiMaxHeight, m_uiMaxDepth );
#endif
}

Void TDecCu::destroy()
{
  for ( UInt ui = 0; ui < m_uiMaxDepth-1; ui++ )
  {
    m_ppcYuvResi[ui]->destroy(); delete m_ppcYuvResi[ui]; m_ppcYuvResi[ui] = NULL;
    m_ppcYuvReco[ui]->destroy(); delete m_ppcYuvReco[ui]; m_ppcYuvReco[ui] = NULL;
    m_ppcCU     [ui]->destroy(); delete m_ppcCU     [ui]; m_ppcCU     [ui] = NULL;
  }
  
  delete [] m_ppcYuvResi; m_ppcYuvResi = NULL;
  delete [] m_ppcYuvReco; m_ppcYuvReco = NULL;
  delete [] m_ppcCU     ; m_ppcCU      = NULL;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param    pcCU        pointer of CU data
 \param    ruiIsLast   last data?
 */
#if ENABLE_ANAYSIS_OUTPUT
Void TDecCu::decodeCU( TComInputBitstream* pcBitstream, TComDataCU* pcCU, UInt& ruiIsLast )
#elif
Void TDecCu::decodeCU( TComDataCU* pcCU, UInt& ruiIsLast )
#endif
{
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    setdQPFlag(true);
  }
  // start from the top level CU
#if FINE_GRANULARITY_SLICES
#if ENABLE_ANAYSIS_OUTPUT
  xDecodeCU( pcBitstream, pcCU, 0, 0, ruiIsLast);
#else
  xDecodeCU( pcCU, 0, 0, ruiIsLast);
#endif
#else
  xDecodeCU( pcCU, 0, 0 );
#endif
  
#if !SUB_LCU_DQP
  // dQP: only for LCU
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    if ( pcCU->isSkipped( 0 ) && pcCU->getDepth( 0 ) == 0 )
    {
    }
    else if ( getdQPFlag() )
    {
      m_pcEntropyDecoder->decodeQP( pcCU, 0, 0 );
      setdQPFlag(false);
    }
  }
#endif 
  
  //--- Read terminating bit ---
#if !FINE_GRANULARITY_SLICES
  m_pcEntropyDecoder->decodeTerminatingBit( ruiIsLast );
#endif
}

/** \param    pcCU        pointer of CU data
 */
Void TDecCu::decompressCU( TComDataCU* pcCU )
{
  xDecompressCU( pcCU, pcCU, 0,  0 );

#if ENABLE_ANAYSIS_OUTPUT
  TSysuAnalyzerOutput::getInstance()->writeOutCUInfo( pcCU );
#endif
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

#if FINE_GRANULARITY_SLICES
/**decode end-of-slice flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth 
 * \returns Bool
 */
Bool TDecCu::xDecodeSliceEnd( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) {
  UInt uiIsLast;
  TComPic* pcPic = pcCU->getPic();
  TComSlice * pcSlice = pcPic->getSlice(pcPic->getCurrSliceIdx());
  UInt uiCurNumParts    = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiWidth = pcSlice->getSPS()->getWidth();
  UInt uiHeight = pcSlice->getSPS()->getHeight();
  UInt uiGranularityWidth = g_uiMaxCUWidth>>(pcSlice->getPPS()->getSliceGranularity());
  UInt uiPosX = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiPosY = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];

  if(((uiPosX+pcCU->getWidth(uiAbsPartIdx))%uiGranularityWidth==0||(uiPosX+pcCU->getWidth(uiAbsPartIdx)==uiWidth))
    &&((uiPosY+pcCU->getHeight(uiAbsPartIdx))%uiGranularityWidth==0||(uiPosY+pcCU->getHeight(uiAbsPartIdx)==uiHeight)))
  {
    m_pcEntropyDecoder->decodeTerminatingBit( uiIsLast );
  }
  else
    uiIsLast=0;
  if(uiIsLast) 
  {
    if(pcSlice->isNextEntropySlice()&&!pcSlice->isNextSlice()) 
    {
      pcSlice->setEntropySliceCurEndCUAddr(pcCU->getSCUAddr()+uiAbsPartIdx+uiCurNumParts);
    }
    else 
    {
      pcSlice->setSliceCurEndCUAddr(pcCU->getSCUAddr()+uiAbsPartIdx+uiCurNumParts);
      pcSlice->setEntropySliceCurEndCUAddr(pcCU->getSCUAddr()+uiAbsPartIdx+uiCurNumParts);
    }
  }

  return uiIsLast>0;
}
#endif
/** decode CU block recursively
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth 
 * \returns Void
 */

#if FINE_GRANULARITY_SLICES
#if ENABLE_ANAYSIS_OUTPUT
Void TDecCu::xDecodeCU( TComInputBitstream* pcBitstream,TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& ruiIsLast)
#else
Void TDecCu::xDecodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& ruiIsLast)
#endif
#else
Void TDecCu::xDecodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
#endif
{
  TComPic* pcPic = pcCU->getPic();
  UInt uiCurNumParts    = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts      = uiCurNumParts>>2;
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
#if FINE_GRANULARITY_SLICES
  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());
  Bool bStartInCU = pcCU->getSCUAddr()+uiAbsPartIdx+uiCurNumParts>pcSlice->getEntropySliceCurStartCUAddr()&&pcCU->getSCUAddr()+uiAbsPartIdx<pcSlice->getEntropySliceCurStartCUAddr();
  if((!bStartInCU) && ( uiRPelX < pcSlice->getSPS()->getWidth() ) && ( uiBPelY < pcSlice->getSPS()->getHeight() ) )
#else
  if( ( uiRPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiBPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#endif
  {
    m_pcEntropyDecoder->decodeSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  else
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth ) ) || bBoundary )
  {
    UInt uiIdx = uiAbsPartIdx;
#if SUB_LCU_DQP
    if( (g_uiMaxCUWidth>>uiDepth) == pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
    {
      setdQPFlag(true);
      pcCU->setQPSubParts( pcCU->getRefQP(uiAbsPartIdx), uiAbsPartIdx, uiDepth ); // set QP to default QP
    }
#endif
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++ )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      
#if FINE_GRANULARITY_SLICES
      Bool bSubInSlice = pcCU->getSCUAddr()+uiIdx+uiQNumParts>pcSlice->getEntropySliceCurStartCUAddr();
      if ( bSubInSlice )
#endif
      {
        if ( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        {
#if FINE_GRANULARITY_SLICES
#if ENABLE_ANAYSIS_OUTPUT
          xDecodeCU( pcBitstream, pcCU, uiIdx, uiDepth+1, ruiIsLast );
#else
          xDecodeCU( pcCU, uiIdx, uiDepth+1, ruiIsLast );
#endif

#else
          xDecodeCU( pcCU, uiIdx, uiDepth+1 );
#endif
        }
        else
        {
          pcCU->setOutsideCUPart( uiIdx, uiDepth+1 );
        }
      }
#if FINE_GRANULARITY_SLICES
      if(ruiIsLast)
      {
        break;
      }
#endif
      
      uiIdx += uiQNumParts;
    }
#if SUB_LCU_DQP
    if( (g_uiMaxCUWidth>>uiDepth) == pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
    {
      if ( getdQPFlag() )
      {
        UInt uiQPSrcPartIdx;
#if FINE_GRANULARITY_SLICES
        if ( pcPic->getCU( pcCU->getAddr() )->getEntropySliceStartCU(uiAbsPartIdx) != pcSlice->getEntropySliceCurStartCUAddr() )
        {
          uiQPSrcPartIdx = pcSlice->getEntropySliceCurStartCUAddr() % pcPic->getNumPartInCU();
        }
        else
#endif
        {
          uiQPSrcPartIdx = uiAbsPartIdx;
        }
        pcCU->setQPSubParts( pcCU->getRefQP( uiQPSrcPartIdx ), uiAbsPartIdx, uiDepth ); // set QP to default QP
      }
    }
#endif
    return;
  }

#if ENABLE_ANAYSIS_OUTPUT
    UInt uiByteLocation = pcBitstream->getNumBitsLeft();
#endif
  
#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uiDepth) >= pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
  {
    setdQPFlag(true);
    pcCU->setQPSubParts( pcCU->getRefQP(uiAbsPartIdx), uiAbsPartIdx, uiDepth ); // set QP to default QP
  }
#endif

  // decode CU mode and the partition size
  if( !pcCU->getSlice()->isIntra() )
  {
    m_pcEntropyDecoder->decodeSkipFlag( pcCU, uiAbsPartIdx, uiDepth );
  }
  

  if( pcCU->isSkipped(uiAbsPartIdx) )
  {
#if HHI_MRG_SKIP
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
      m_ppcCU[uiDepth]->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_0 );
      m_ppcCU[uiDepth]->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_1 );
      TComMvField cMvFieldNeighbours[MRG_MAX_NUM_CANDS << 1]; // double length for mv of both lists
      UChar uhInterDirNeighbours[MRG_MAX_NUM_CANDS];
#if MRG_AMVP_FIXED_IDX_F470
      Int numValidMergeCand = 0;
#else
      UInt uiNeighbourCandIdx[MRG_MAX_NUM_CANDS]; //MVs with same idx => same cand
#endif
      for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ++ui )
      {
        uhInterDirNeighbours[ui] = 0;
#if !MRG_AMVP_FIXED_IDX_F470
        uiNeighbourCandIdx[ui] = 0;
#endif
      }
#if MRG_AMVP_FIXED_IDX_F470
      m_pcEntropyDecoder->decodeMergeIndex( pcCU, 0, uiAbsPartIdx, SIZE_2Nx2N, uhInterDirNeighbours, cMvFieldNeighbours, uiDepth );
      m_ppcCU[uiDepth]->getInterMergeCandidates( 0, 0, uiDepth, cMvFieldNeighbours, uhInterDirNeighbours, numValidMergeCand );

      UInt uiMergeIndex = pcCU->getMergeIndex(uiAbsPartIdx);
      pcCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeIndex], uiAbsPartIdx, 0, uiDepth );

      TComMv cTmpMv( 0, 0 );
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0  ) //if ( ref. frame list0 has at least 1 entry )
      {
        pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_0, uiAbsPartIdx, 0, uiDepth);
        pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_0, uiAbsPartIdx, 0, uiDepth);
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cTmpMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ], SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      }
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
      {
        pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_1, uiAbsPartIdx, 0, uiDepth);
        pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_1, uiAbsPartIdx, 0, uiDepth);
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cTmpMv, SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ], SIZE_2Nx2N, uiAbsPartIdx, uiDepth );
      }
#else
      m_ppcCU[uiDepth]->getInterMergeCandidates( 0, 0, uiDepth, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourCandIdx );
      for(UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
      {
        pcCU->setNeighbourCandIdxSubParts( uiIter, uiNeighbourCandIdx[uiIter], uiAbsPartIdx, 0, uiDepth );
      }
      m_pcEntropyDecoder->decodeMergeIndex( pcCU, 0, uiAbsPartIdx, SIZE_2Nx2N, uhInterDirNeighbours, cMvFieldNeighbours, uiDepth );
#endif
    }
    else
#endif
    {
      pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_0, uiAbsPartIdx, 0, uiDepth);
      pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_0, uiAbsPartIdx, 0, uiDepth);
      
      pcCU->setMVPIdxSubParts( -1, REF_PIC_LIST_1, uiAbsPartIdx, 0, uiDepth);
      pcCU->setMVPNumSubParts( -1, REF_PIC_LIST_1, uiAbsPartIdx, 0, uiDepth);
      
      for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
      {        
        if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
        {
          m_ppcCU[uiDepth]->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, RefPicList( uiRefListIdx ) );
          m_pcEntropyDecoder->decodeMVPIdxPU( m_ppcCU[uiDepth], 0, uiDepth, 0, RefPicList( uiRefListIdx ));
        }
      }
    }
    xFinishDecodeCU( pcCU, uiAbsPartIdx, uiDepth, ruiIsLast );
#if ENABLE_ANAYSIS_OUTPUT
    UInt iByteConsumed = uiByteLocation - pcBitstream->getNumBitsLeft();
    TSysuAnalyzerOutput::getInstance()->aiCUBits.push_back(iByteConsumed);
#endif
    return;
  }
  m_pcEntropyDecoder->decodePredMode( pcCU, uiAbsPartIdx, uiDepth );
  m_pcEntropyDecoder->decodePartSize( pcCU, uiAbsPartIdx, uiDepth );
  
#if E057_INTRA_PCM
  if (pcCU->isIntra( uiAbsPartIdx ) && pcCU->getPartitionSize( uiAbsPartIdx ) == SIZE_2Nx2N )
  {
    m_pcEntropyDecoder->decodeIPCMInfo( pcCU, uiAbsPartIdx, uiDepth );

    if(pcCU->getIPCMFlag(uiAbsPartIdx))
    {
      xFinishDecodeCU( pcCU, uiAbsPartIdx, uiDepth, ruiIsLast );
#if ENABLE_ANAYSIS_OUTPUT
    UInt iByteConsumed = uiByteLocation - pcBitstream->getNumBitsLeft();
    TSysuAnalyzerOutput::getInstance()->aiCUBits.push_back(iByteConsumed);
#endif
      return;
    }
  }
#endif

  UInt uiCurrWidth      = pcCU->getWidth ( uiAbsPartIdx );
  UInt uiCurrHeight     = pcCU->getHeight( uiAbsPartIdx );
  
  // prediction mode ( Intra : direction mode, Inter : Mv, reference idx )
  m_pcEntropyDecoder->decodePredInfo( pcCU, uiAbsPartIdx, uiDepth, m_ppcCU[uiDepth]);
  
  // Coefficient decoding
  Bool bCodeDQP = getdQPFlag();
  m_pcEntropyDecoder->decodeCoeff( pcCU, uiAbsPartIdx, uiDepth, uiCurrWidth, uiCurrHeight, bCodeDQP );
  setdQPFlag( bCodeDQP );
  
  xFinishDecodeCU( pcCU, uiAbsPartIdx, uiDepth, ruiIsLast );
#if ENABLE_ANAYSIS_OUTPUT
    UInt iByteConsumed = uiByteLocation - pcBitstream->getNumBitsLeft();
    TSysuAnalyzerOutput::getInstance()->aiCUBits.push_back(iByteConsumed);
#endif
}

#if FINE_GRANULARITY_SLICES
Void TDecCu::xFinishDecodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& ruiIsLast)
#else
Void TDecCu::xFinishDecodeCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
#endif
{
#if SUB_LCU_DQP
  if( (g_uiMaxCUWidth>>uiDepth) >= pcCU->getSlice()->getPPS()->getMinCuDQPSize() && pcCU->getSlice()->getSPS()->getUseDQP())
  {
    if( getdQPFlag() )
    {
      pcCU->setQPSubParts( pcCU->getRefQP(uiAbsPartIdx), uiAbsPartIdx, uiDepth ); // set QP to default QP
    }
  }
#endif
#if FINE_GRANULARITY_SLICES
  ruiIsLast = xDecodeSliceEnd( pcCU, uiAbsPartIdx, uiDepth);
#endif
}

Void TDecCu::xDecompressCU( TComDataCU* pcCU, TComDataCU* pcCUCur, UInt uiAbsPartIdx,  UInt uiDepth )
{
  TComPic* pcPic = pcCU->getPic();
  
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
#if FINE_GRANULARITY_SLICES
  UInt uiCurNumParts    = pcPic->getNumPartInCU() >> (uiDepth<<1);
  TComSlice * pcSlice = pcCU->getPic()->getSlice(pcCU->getPic()->getCurrSliceIdx());
  Bool bStartInCU = pcCU->getSCUAddr()+uiAbsPartIdx+uiCurNumParts>pcSlice->getEntropySliceCurStartCUAddr()&&pcCU->getSCUAddr()+uiAbsPartIdx<pcSlice->getEntropySliceCurStartCUAddr();
  if(bStartInCU||( uiRPelX >= pcSlice->getSPS()->getWidth() ) || ( uiBPelY >= pcSlice->getSPS()->getHeight() ) )
#else
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
#endif
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < g_uiMaxCUDepth - g_uiAddCUDepth ) ) || bBoundary )
  {
    UInt uiNextDepth = uiDepth + 1;
    UInt uiQNumParts = pcCU->getTotalNumPart() >> (uiNextDepth<<1);
    UInt uiIdx = uiAbsPartIdx;
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++ )
    {
      uiLPelX = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiIdx] ];
      uiTPelY = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiIdx] ];
      
#if FINE_GRANULARITY_SLICES
      Bool binSlice = (pcCU->getSCUAddr()+uiIdx+uiQNumParts>pcSlice->getEntropySliceCurStartCUAddr())&&(pcCU->getSCUAddr()+uiIdx<pcSlice->getEntropySliceCurEndCUAddr());
      if(binSlice&&( uiLPelX < pcSlice->getSPS()->getWidth() ) && ( uiTPelY < pcSlice->getSPS()->getHeight() ) )
#else
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#endif
        xDecompressCU(pcCU, m_ppcCU[uiNextDepth], uiIdx, uiNextDepth );
      
      uiIdx += uiQNumParts;
    }
    return;
  }
  
  // Residual reconstruction
  m_ppcYuvResi[uiDepth]->clear();
  
  m_ppcCU[uiDepth]->copySubCU( pcCU, uiAbsPartIdx, uiDepth );
  
  switch( m_ppcCU[uiDepth]->getPredictionMode(0) )
  {
    case MODE_SKIP:
    case MODE_INTER:
      xReconInter( m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth );
      break;
    case MODE_INTRA:
      xReconIntraQT( m_ppcCU[uiDepth], uiAbsPartIdx, uiDepth );
      break;
    default:
      assert(0);
      break;
  }
  
  xCopyToPic( m_ppcCU[uiDepth], pcPic, uiAbsPartIdx, uiDepth );
}

Void TDecCu::xReconInter( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  
  // inter prediction
  m_pcPrediction->motionCompensation( pcCU, m_ppcYuvReco[uiDepth] );
  
  // inter recon
  xDecodeInterTexture( pcCU, 0, uiDepth );
  
  // clip for only non-zero cbp case
  if  ( ( pcCU->getCbf( 0, TEXT_LUMA ) ) || ( pcCU->getCbf( 0, TEXT_CHROMA_U ) ) || ( pcCU->getCbf(0, TEXT_CHROMA_V ) ) )
  {
    m_ppcYuvReco[uiDepth]->addClip( m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth], 0, pcCU->getWidth( 0 ) );
  }
  else
  {
    m_ppcYuvReco[uiDepth]->copyPartToPartYuv( m_ppcYuvReco[uiDepth],0, pcCU->getWidth( 0 ),pcCU->getHeight( 0 ));
  }
}

Void
TDecCu::xIntraRecLumaBlk( TComDataCU* pcCU,
                         UInt        uiTrDepth,
                         UInt        uiAbsPartIdx,
                         TComYuv*    pcRecoYuv,
                         TComYuv*    pcPredYuv, 
                         TComYuv*    pcResiYuv )
{
  UInt    uiWidth           = pcCU     ->getWidth   ( 0 ) >> uiTrDepth;
  UInt    uiHeight          = pcCU     ->getHeight  ( 0 ) >> uiTrDepth;
  UInt    uiStride          = pcRecoYuv->getStride  ();
  Pel*    piReco            = pcRecoYuv->getLumaAddr( uiAbsPartIdx );
  Pel*    piPred            = pcPredYuv->getLumaAddr( uiAbsPartIdx );
  Pel*    piResi            = pcResiYuv->getLumaAddr( uiAbsPartIdx );
  
  UInt    uiNumCoeffInc     = ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 );
  TCoeff* pcCoeff           = pcCU->getCoeffY() + ( uiNumCoeffInc * uiAbsPartIdx );
  
  UInt    uiLumaPredMode    = pcCU->getLumaIntraDir     ( uiAbsPartIdx );
  
  UInt    uiZOrder          = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  Pel*    piRecIPred        = pcCU->getPic()->getPicYuvRec()->getLumaAddr( pcCU->getAddr(), uiZOrder );
  UInt    uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getStride  ();
  
  //===== init availability pattern =====
  Bool  bAboveAvail = false;
  Bool  bLeftAvail  = false;
  pcCU->getPattern()->initPattern   ( pcCU, uiTrDepth, uiAbsPartIdx );
  pcCU->getPattern()->initAdiPattern( pcCU, uiAbsPartIdx, uiTrDepth, 
                                     m_pcPrediction->getPredicBuf       (),
                                     m_pcPrediction->getPredicBufWidth  (),
                                     m_pcPrediction->getPredicBufHeight (),
                                     bAboveAvail, bLeftAvail );
  
  //===== get prediction signal =====
  m_pcPrediction->predIntraLumaAng( pcCU->getPattern(), uiLumaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
  
  //===== inverse transform =====
  m_pcTrQuant->setQPforQuant  ( pcCU->getQP(0), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
#if INTRA_DST_TYPE_7
  m_pcTrQuant->invtransformNxN( TEXT_LUMA, pcCU->getLumaIntraDir( uiAbsPartIdx ), piResi, uiStride, pcCoeff, uiWidth, uiHeight );
#else
  m_pcTrQuant->invtransformNxN( piResi, uiStride, pcCoeff, uiWidth, uiHeight );
#endif  

  
  //===== reconstruction =====
  {
    Pel* pPred      = piPred;
    Pel* pResi      = piResi;
    Pel* pReco      = piReco;
    Pel* pRecIPred  = piRecIPred;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pReco    [ uiX ] = Clip( pPred[ uiX ] + pResi[ uiX ] );
        pRecIPred[ uiX ] = pReco[ uiX ];
      }
      pPred     += uiStride;
      pResi     += uiStride;
      pReco     += uiStride;
      pRecIPred += uiRecIPredStride;
    }
  }
}


Void
TDecCu::xIntraRecChromaBlk( TComDataCU* pcCU,
                           UInt        uiTrDepth,
                           UInt        uiAbsPartIdx,
                           TComYuv*    pcRecoYuv,
                           TComYuv*    pcPredYuv, 
                           TComYuv*    pcResiYuv,
                           UInt        uiChromaId )
{
  UInt uiFullDepth  = pcCU->getDepth( 0 ) + uiTrDepth;
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiFullDepth ] + 2;
  if( uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    assert( uiTrDepth > 0 );
    uiTrDepth--;
    UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( pcCU->getDepth( 0 ) + uiTrDepth ) << 1 );
    Bool bFirstQ = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
    if( !bFirstQ )
    {
      return;
    }
  }
  
  TextType  eText             = ( uiChromaId > 0 ? TEXT_CHROMA_V : TEXT_CHROMA_U );
  UInt      uiWidth           = pcCU     ->getWidth   ( 0 ) >> ( uiTrDepth + 1 );
  UInt      uiHeight          = pcCU     ->getHeight  ( 0 ) >> ( uiTrDepth + 1 );
  UInt      uiStride          = pcRecoYuv->getCStride ();
  Pel*      piReco            = ( uiChromaId > 0 ? pcRecoYuv->getCrAddr( uiAbsPartIdx ) : pcRecoYuv->getCbAddr( uiAbsPartIdx ) );
  Pel*      piPred            = ( uiChromaId > 0 ? pcPredYuv->getCrAddr( uiAbsPartIdx ) : pcPredYuv->getCbAddr( uiAbsPartIdx ) );
  Pel*      piResi            = ( uiChromaId > 0 ? pcResiYuv->getCrAddr( uiAbsPartIdx ) : pcResiYuv->getCbAddr( uiAbsPartIdx ) );
  
  UInt      uiNumCoeffInc     = ( ( pcCU->getSlice()->getSPS()->getMaxCUWidth() * pcCU->getSlice()->getSPS()->getMaxCUHeight() ) >> ( pcCU->getSlice()->getSPS()->getMaxCUDepth() << 1 ) ) >> 2;
  TCoeff*   pcCoeff           = ( uiChromaId > 0 ? pcCU->getCoeffCr() : pcCU->getCoeffCb() ) + ( uiNumCoeffInc * uiAbsPartIdx );
  
  UInt      uiChromaPredMode  = pcCU->getChromaIntraDir( 0 );
  
#if !LM_CHROMA
  if( uiChromaPredMode == 4 )
  {
    uiChromaPredMode          = pcCU->getLumaIntraDir( 0 );
  }
#endif
  
  UInt      uiZOrder          = pcCU->getZorderIdxInCU() + uiAbsPartIdx;
  Pel*      piRecIPred        = ( uiChromaId > 0 ? pcCU->getPic()->getPicYuvRec()->getCrAddr( pcCU->getAddr(), uiZOrder ) : pcCU->getPic()->getPicYuvRec()->getCbAddr( pcCU->getAddr(), uiZOrder ) );
  UInt      uiRecIPredStride  = pcCU->getPic()->getPicYuvRec()->getCStride();
  
  //===== init availability pattern =====
  Bool  bAboveAvail = false;
  Bool  bLeftAvail  = false;
  pcCU->getPattern()->initPattern         ( pcCU, uiTrDepth, uiAbsPartIdx );

#if LM_CHROMA
  if( uiChromaPredMode == LM_CHROMA_IDX && uiChromaId == 0 )
  {
    pcCU->getPattern()->initAdiPattern( pcCU, uiAbsPartIdx, uiTrDepth, 
                                     m_pcPrediction->getPredicBuf       (),
                                     m_pcPrediction->getPredicBufWidth  (),
                                     m_pcPrediction->getPredicBufHeight (),
                                     bAboveAvail, bLeftAvail, 
#if LM_CHROMA_SIMPLIFICATION
                                     true );
#else
                                     2 );
#endif

    m_pcPrediction->getLumaRecPixels( pcCU->getPattern(), uiWidth, uiHeight );
  }
#endif
  
  pcCU->getPattern()->initAdiPatternChroma( pcCU, uiAbsPartIdx, uiTrDepth, 
                                           m_pcPrediction->getPredicBuf       (),
                                           m_pcPrediction->getPredicBufWidth  (),
                                           m_pcPrediction->getPredicBufHeight (),
                                           bAboveAvail, bLeftAvail );
  Int* pPatChroma   = ( uiChromaId > 0 ? pcCU->getPattern()->getAdiCrBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() ) : pcCU->getPattern()->getAdiCbBuf( uiWidth, uiHeight, m_pcPrediction->getPredicBuf() ) );
  
  //===== get prediction signal =====
#if LM_CHROMA
  if( uiChromaPredMode == LM_CHROMA_IDX )
  {
    m_pcPrediction->predLMIntraChroma( pcCU->getPattern(), pPatChroma, piPred, uiStride, uiWidth, uiHeight, uiChromaId );
  }
  else
  {
#if FIXED_MPM
    if( uiChromaPredMode == DM_CHROMA_IDX )
#else
    if( uiChromaPredMode == 4 )
#endif
    {
      uiChromaPredMode = pcCU->getLumaIntraDir( 0 );
    }
    m_pcPrediction->predIntraChromaAng( pcCU->getPattern(), pPatChroma, uiChromaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );  
  }
#else // LM_CHROMA
  m_pcPrediction->predIntraChromaAng( pcCU->getPattern(), pPatChroma, uiChromaPredMode, piPred, uiStride, uiWidth, uiHeight, pcCU, bAboveAvail, bLeftAvail );
#endif

  //===== inverse transform =====
  m_pcTrQuant->setQPforQuant  ( pcCU->getQP(0), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), eText );
#if INTRA_DST_TYPE_7 
  m_pcTrQuant->invtransformNxN( eText, REG_DCT, piResi, uiStride, pcCoeff, uiWidth, uiHeight );
#else
  m_pcTrQuant->invtransformNxN( piResi, uiStride, pcCoeff, uiWidth, uiHeight );
#endif

  //===== reconstruction =====
  {
    Pel* pPred      = piPred;
    Pel* pResi      = piResi;
    Pel* pReco      = piReco;
    Pel* pRecIPred  = piRecIPred;
    for( UInt uiY = 0; uiY < uiHeight; uiY++ )
    {
      for( UInt uiX = 0; uiX < uiWidth; uiX++ )
      {
        pReco    [ uiX ] = Clip( pPred[ uiX ] + pResi[ uiX ] );
        pRecIPred[ uiX ] = pReco[ uiX ];
      }
      pPred     += uiStride;
      pResi     += uiStride;
      pReco     += uiStride;
      pRecIPred += uiRecIPredStride;
    }
  }
}

Void
TDecCu::xIntraRecQT( TComDataCU* pcCU,
                    UInt        uiTrDepth,
                    UInt        uiAbsPartIdx,
                    TComYuv*    pcRecoYuv,
                    TComYuv*    pcPredYuv, 
                    TComYuv*    pcResiYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if( uiTrMode == uiTrDepth )
  {
    xIntraRecLumaBlk  ( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv );
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 0 );
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 1 );
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xIntraRecQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, pcRecoYuv, pcPredYuv, pcResiYuv );
    }
  }
}

Void
TDecCu::xReconIntraQT( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt  uiInitTrDepth = ( pcCU->getPartitionSize(0) == SIZE_2Nx2N ? 0 : 1 );
  UInt  uiNumPart     = pcCU->getNumPartInter();
  UInt  uiNumQParts   = pcCU->getTotalNumPart() >> 2;
  
#if E057_INTRA_PCM
  if (pcCU->getIPCMFlag(0))
  {
    xReconPCM( pcCU, uiAbsPartIdx, uiDepth );
    return;
  }
#endif

#if LM_CHROMA
  for( UInt uiPU = 0; uiPU < uiNumPart; uiPU++ )
  {
    xIntraLumaRecQT( pcCU, uiInitTrDepth, uiPU * uiNumQParts, m_ppcYuvReco[uiDepth], m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth] );
  }  

  for( UInt uiPU = 0; uiPU < uiNumPart; uiPU++ )
  {
    xIntraChromaRecQT( pcCU, uiInitTrDepth, uiPU * uiNumQParts, m_ppcYuvReco[uiDepth], m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth] );
  }
#else

  for( UInt uiPU = 0; uiPU < uiNumPart; uiPU++ )
  {
    xIntraRecQT( pcCU, uiInitTrDepth, uiPU * uiNumQParts, m_ppcYuvReco[uiDepth], m_ppcYuvReco[uiDepth], m_ppcYuvResi[uiDepth] );
  }
#endif

}

#if LM_CHROMA

/** Function for deriving recontructed PU/CU Luma sample with QTree structure
 * \param pcCU pointer of current CU
 * \param uiTrDepth current tranform split depth
 * \param uiAbsPartIdx  part index
 * \param pcRecoYuv pointer to reconstructed sample arrays
 * \param pcPredYuv pointer to prediction sample arrays
 * \param pcResiYuv pointer to residue sample arrays
 * 
 \ This function dervies recontructed PU/CU Luma sample with recursive QTree structure
 */
Void
TDecCu::xIntraLumaRecQT( TComDataCU* pcCU,
                     UInt        uiTrDepth,
                     UInt        uiAbsPartIdx,
                     TComYuv*    pcRecoYuv,
                     TComYuv*    pcPredYuv, 
                     TComYuv*    pcResiYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if( uiTrMode == uiTrDepth )
  {
    xIntraRecLumaBlk  ( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv );
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xIntraLumaRecQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, pcRecoYuv, pcPredYuv, pcResiYuv );
    }
  }
}

/** Function for deriving recontructed PU/CU chroma samples with QTree structure
 * \param pcCU pointer of current CU
 * \param uiTrDepth current tranform split depth
 * \param uiAbsPartIdx  part index
 * \param pcRecoYuv pointer to reconstructed sample arrays
 * \param pcPredYuv pointer to prediction sample arrays
 * \param pcResiYuv pointer to residue sample arrays
 * 
 \ This function dervies recontructed PU/CU chroma samples with QTree recursive structure
 */
Void
TDecCu::xIntraChromaRecQT( TComDataCU* pcCU,
                     UInt        uiTrDepth,
                     UInt        uiAbsPartIdx,
                     TComYuv*    pcRecoYuv,
                     TComYuv*    pcPredYuv, 
                     TComYuv*    pcResiYuv )
{
  UInt uiFullDepth  = pcCU->getDepth(0) + uiTrDepth;
  UInt uiTrMode     = pcCU->getTransformIdx( uiAbsPartIdx );
  if( uiTrMode == uiTrDepth )
  {
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 0 );
    xIntraRecChromaBlk( pcCU, uiTrDepth, uiAbsPartIdx, pcRecoYuv, pcPredYuv, pcResiYuv, 1 );
  }
  else
  {
    UInt uiNumQPart  = pcCU->getPic()->getNumPartInCU() >> ( ( uiFullDepth + 1 ) << 1 );
    for( UInt uiPart = 0; uiPart < 4; uiPart++ )
    {
      xIntraChromaRecQT( pcCU, uiTrDepth + 1, uiAbsPartIdx + uiPart * uiNumQPart, pcRecoYuv, pcPredYuv, pcResiYuv );
    }
  }
}
#endif

Void TDecCu::xCopyToPic( TComDataCU* pcCU, TComPic* pcPic, UInt uiZorderIdx, UInt uiDepth )
{
  UInt uiCUAddr = pcCU->getAddr();
  
  m_ppcYuvReco[uiDepth]->copyToPicYuv  ( pcPic->getPicYuvRec (), uiCUAddr, uiZorderIdx );

#if ENABLE_ANAYSIS_OUTPUT
  m_ppcYuvResi[uiDepth]->copyToPicYuv  ( pcPic->getPicYuvResi(), uiCUAddr, uiZorderIdx );
#endif 
  
  return;
}

Void TDecCu::xDecodeInterTexture ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt    uiWidth    = pcCU->getWidth ( uiAbsPartIdx );
  UInt    uiHeight   = pcCU->getHeight( uiAbsPartIdx );
  TCoeff* piCoeff;
  
  Pel*    pResi;
  UInt    uiLumaTrMode, uiChromaTrMode;
  
  pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
  
  // Y
  piCoeff = pcCU->getCoeffY();
  pResi = m_ppcYuvResi[uiDepth]->getLumaAddr();
  m_pcTrQuant->setQPforQuant( pcCU->getQP( uiAbsPartIdx ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_LUMA );
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_LUMA, pResi, 0, m_ppcYuvResi[uiDepth]->getStride(), uiWidth, uiHeight, uiLumaTrMode, 0, piCoeff );
  
  // Cb and Cr
  m_pcTrQuant->setQPforQuant( pcCU->getQP( uiAbsPartIdx ), !pcCU->getSlice()->getDepth(), pcCU->getSlice()->getSliceType(), TEXT_CHROMA );
  
  uiWidth  >>= 1;
  uiHeight >>= 1;
  piCoeff = pcCU->getCoeffCb(); pResi = m_ppcYuvResi[uiDepth]->getCbAddr();
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_U, pResi, 0, m_ppcYuvResi[uiDepth]->getCStride(), uiWidth, uiHeight, uiChromaTrMode, 0, piCoeff );
  piCoeff = pcCU->getCoeffCr(); pResi = m_ppcYuvResi[uiDepth]->getCrAddr();
  m_pcTrQuant->invRecurTransformNxN ( pcCU, 0, TEXT_CHROMA_V, pResi, 0, m_ppcYuvResi[uiDepth]->getCStride(), uiWidth, uiHeight, uiChromaTrMode, 0, piCoeff );
}

#if E057_INTRA_PCM
/** Function for deriving reconstructed luma/chroma samples of a PCM mode CU.
 * \param pcCU pointer to current CU
 * \param uiPartIdx part index
 * \param piPCM pointer to PCM code arrays
 * \param piReco pointer to reconstructed sample arrays
 * \param uiStride stride of reconstructed sample arrays
 * \param uiWidth CU width
 * \param uiHeight CU height
 * \param ttText texture component type
 * \returns Void
 */
Void TDecCu::xDecodePCMTexture( TComDataCU* pcCU, UInt uiPartIdx, Pel *piPCM, Pel* piReco, UInt uiStride, UInt uiWidth, UInt uiHeight, TextType ttText)
{
  UInt uiX, uiY;
  Pel* piPicReco;
  UInt uiPicStride;
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
  UInt uiPcmLeftShiftBit; 
#endif

  if( ttText == TEXT_LUMA )
  {
    uiPicStride   = pcCU->getPic()->getPicYuvRec()->getStride();
    piPicReco = pcCU->getPic()->getPicYuvRec()->getLumaAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiPartIdx);
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiPcmLeftShiftBit = g_uiBitDepth + g_uiBitIncrement - pcCU->getSlice()->getSPS()->getPCMBitDepthLuma();
#endif
  }
  else
  {
    uiPicStride = pcCU->getPic()->getPicYuvRec()->getCStride();

    if( ttText == TEXT_CHROMA_U )
    {
      piPicReco = pcCU->getPic()->getPicYuvRec()->getCbAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiPartIdx);
    }
    else
    {
      piPicReco = pcCU->getPic()->getPicYuvRec()->getCrAddr(pcCU->getAddr(), pcCU->getZorderIdxInCU()+uiPartIdx);
    }
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
    uiPcmLeftShiftBit = g_uiBitDepth + g_uiBitIncrement - pcCU->getSlice()->getSPS()->getPCMBitDepthChroma();
#endif
  }

  for( uiY = 0; uiY < uiHeight; uiY++ )
  {
    for( uiX = 0; uiX < uiWidth; uiX++ )
    {
#if E192_SPS_PCM_BIT_DEPTH_SYNTAX
      piReco[uiX] = (piPCM[uiX] << uiPcmLeftShiftBit);
#else
      if(g_uiBitIncrement > 0)
        piReco[uiX] = (piPCM[uiX] << g_uiBitIncrement);
      else
        piReco[uiX] = piPCM[uiX];
#endif
      piPicReco[uiX] = piReco[uiX];
    }
    piPCM += uiWidth;
    piReco += uiStride;
    piPicReco += uiPicStride;
  }
}

/** Function for reconstructing a PCM mode CU.
 * \param pcCU pointer to current CU
 * \param uiAbsPartIdx CU index
 * \param uiDepth CU Depth
 * \returns Void
 */
Void TDecCu::xReconPCM( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  // Luma
  UInt uiWidth  = (g_uiMaxCUWidth >> uiDepth);
  UInt uiHeight = (g_uiMaxCUHeight >> uiDepth);

  Pel* piPcmY = pcCU->getPCMSampleY();
  Pel* piRecoY = m_ppcYuvReco[uiDepth]->getLumaAddr(0, uiWidth);

  UInt uiStride = m_ppcYuvResi[uiDepth]->getStride();

  xDecodePCMTexture( pcCU, 0, piPcmY, piRecoY, uiStride, uiWidth, uiHeight, TEXT_LUMA);

  // Cb and Cr
  UInt uiCWidth  = (uiWidth>>1);
  UInt uiCHeight = (uiHeight>>1);

  Pel* piPcmCb = pcCU->getPCMSampleCb();
  Pel* piPcmCr = pcCU->getPCMSampleCr();
  Pel* pRecoCb = m_ppcYuvReco[uiDepth]->getCbAddr();
  Pel* pRecoCr = m_ppcYuvReco[uiDepth]->getCrAddr();

  UInt uiCStride = m_ppcYuvReco[uiDepth]->getCStride();

  xDecodePCMTexture( pcCU, 0, piPcmCb, pRecoCb, uiCStride, uiCWidth, uiCHeight, TEXT_CHROMA_U);
  xDecodePCMTexture( pcCU, 0, piPcmCr, pRecoCr, uiCStride, uiCWidth, uiCHeight, TEXT_CHROMA_V);
}
#endif

//! \}
