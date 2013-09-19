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

/** \file     TEncSlice.cpp
    \brief    slice encoder class
*/

#include "TEncTop.h"
#include "TEncSlice.h"
#include <math.h>

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSlice::TEncSlice()
{
  m_apcPicYuvPred = NULL;
  m_apcPicYuvResi = NULL;
  
  m_pdRdPicLambda = NULL;
  m_pdRdPicQp     = NULL;
  m_piRdPicQp     = NULL;
}

TEncSlice::~TEncSlice()
{
}

Void TEncSlice::create( Int iWidth, Int iHeight, UInt iMaxCUWidth, UInt iMaxCUHeight, UChar uhTotalDepth )
{
  // create prediction picture
  if ( m_apcPicYuvPred == NULL )
  {
    m_apcPicYuvPred  = new TComPicYuv;
    m_apcPicYuvPred->create( iWidth, iHeight, iMaxCUWidth, iMaxCUHeight, uhTotalDepth );
  }
  
  // create residual picture
  if( m_apcPicYuvResi == NULL )
  {
    m_apcPicYuvResi  = new TComPicYuv;
    m_apcPicYuvResi->create( iWidth, iHeight, iMaxCUWidth, iMaxCUHeight, uhTotalDepth );
  }
}

Void TEncSlice::destroy()
{
  // destroy prediction picture
  if ( m_apcPicYuvPred )
  {
    m_apcPicYuvPred->destroy();
    delete m_apcPicYuvPred;
    m_apcPicYuvPred  = NULL;
  }
  
  // destroy residual picture
  if ( m_apcPicYuvResi )
  {
    m_apcPicYuvResi->destroy();
    delete m_apcPicYuvResi;
    m_apcPicYuvResi  = NULL;
  }
  
  // free lambda and QP arrays
  if ( m_pdRdPicLambda ) { xFree( m_pdRdPicLambda ); m_pdRdPicLambda = NULL; }
  if ( m_pdRdPicQp     ) { xFree( m_pdRdPicQp     ); m_pdRdPicQp     = NULL; }
  if ( m_piRdPicQp     ) { xFree( m_piRdPicQp     ); m_piRdPicQp     = NULL; }
}

Void TEncSlice::init( TEncTop* pcEncTop )
{
  m_pcCfg             = pcEncTop;
  m_pcListPic         = pcEncTop->getListPic();
  
  m_pcGOPEncoder      = pcEncTop->getGOPEncoder();
  m_pcCuEncoder       = pcEncTop->getCuEncoder();
  m_pcPredSearch      = pcEncTop->getPredSearch();
  
  m_pcEntropyCoder    = pcEncTop->getEntropyCoder();
  m_pcCavlcCoder      = pcEncTop->getCavlcCoder();
  m_pcSbacCoder       = pcEncTop->getSbacCoder();
  m_pcBinCABAC        = pcEncTop->getBinCABAC();
  m_pcTrQuant         = pcEncTop->getTrQuant();
  
  m_pcBitCounter      = pcEncTop->getBitCounter();
  m_pcRdCost          = pcEncTop->getRdCost();
  m_pppcRDSbacCoder   = pcEncTop->getRDSbacCoder();
  m_pcRDGoOnSbacCoder = pcEncTop->getRDGoOnSbacCoder();
  
  // create lambda and QP arrays
  m_pdRdPicLambda     = (Double*)xMalloc( Double, m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_pdRdPicQp         = (Double*)xMalloc( Double, m_pcCfg->getDeltaQpRD() * 2 + 1 );
  m_piRdPicQp         = (Int*   )xMalloc( Int,    m_pcCfg->getDeltaQpRD() * 2 + 1 );
}

/**
 - non-referenced frame marking
 - QP computation based on temporal structure
 - lambda computation based on QP
 - set temporal layer ID and the parameter sets
 .
 \param pcPic         picture class
 \param iPOCLast      POC of last picture
 \param uiPOCCurr     current POC
 \param iNumPicRcvd   number of received pictures
 \param iTimeOffset   POC offset for hierarchical structure
 \param iDepth        temporal layer depth
 \param rpcSlice      slice header class
 \param pSPS          SPS associated with the slice
 \param pPPS          PPS associated with the slice
 */
Void TEncSlice::initEncSlice( TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iTimeOffset, Int iDepth, TComSlice*& rpcSlice, TComSPS* pSPS, TComPPS *pPPS )
{
  Double dQP;
  Double dLambda;
  
  rpcSlice = pcPic->getSlice(0);
  rpcSlice->setSliceBits(0);
  rpcSlice->setPic( pcPic );
  rpcSlice->initSlice();
  rpcSlice->setPOC( iPOCLast - iNumPicRcvd + iTimeOffset );
  
  // depth re-computation based on rate GOP size
  if ( m_pcCfg->getGOPSize() != m_pcCfg->getRateGOPSize() )
  {
    Int i, j;
    Int iPOC = rpcSlice->getPOC()%m_pcCfg->getRateGOPSize();
    if ( iPOC == 0 ) iDepth = 0;
    else
    {
      Int iStep = m_pcCfg->getRateGOPSize();
      iDepth    = 0;
      for( i=iStep>>1; i>=1; i>>=1 )
      {
        for ( j=i; j<m_pcCfg->getRateGOPSize(); j+=iStep )
        {
          if ( j == iPOC )
          {
            i=0;
            break;
          }
        }
        iStep>>=1;
        iDepth++;
      }
    }
  }
  
  // slice type
  SliceType eSliceType;
  
#if !HB_LAMBDA_FOR_LDC
  if ( m_pcCfg->getUseLDC() )
  {
    eSliceType = P_SLICE;
  }
  else
#endif
  {
    eSliceType = ( uiPOCCurr % m_pcCfg->getGOPSize() != 0 || iDepth > 0) ? B_SLICE : P_SLICE;
  }
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType    ( eSliceType );
  
  // ------------------------------------------------------------------------------------------------------------------
  // Non-referenced frame marking
  // ------------------------------------------------------------------------------------------------------------------
  
  if ( m_pcCfg->getUseNRF() )
  {
    if ( ( m_pcCfg->getRateGOPSize() != 1) && (m_pcCfg->getRateGOPSize() >> (iDepth+1)) == 0 )
    {
      rpcSlice->setReferenced(false);
    }
    else
    {
      rpcSlice->setReferenced(true);
    }
  }
  else
  {
    rpcSlice->setReferenced(true);
  }
  
  // ------------------------------------------------------------------------------------------------------------------
  // QP setting
  // ------------------------------------------------------------------------------------------------------------------
  
  dQP = m_pcCfg->getQP();
  if ( iDepth < MAX_TLAYER && m_pcCfg->getTemporalLayerQPOffset(iDepth) != ( MAX_QP + 1 ) )
  {
    dQP += m_pcCfg->getTemporalLayerQPOffset(iDepth);
  }
  else
  {
    if ( ( iPOCLast != 0 ) && ( ( uiPOCCurr % m_pcCfg->getIntraPeriod() ) != 0 ) && ( m_pcGOPEncoder->getGOPSize() != 0 ) ) // P or B-slice
    {
      if ( m_pcCfg->getUseLDC() && !m_pcCfg->getUseBQP() )
      {
        if ( iDepth == 0 ) dQP += 1.0;
        else
        {
          dQP += iDepth+3;
        }
      }
      else
      {
        dQP += iDepth+1;
      }
    }
  }
  
  // modify QP
  Int* pdQPs = m_pcCfg->getdQPs();
  if ( pdQPs )
  {
    dQP += pdQPs[ rpcSlice->getPOC() ];
  }
  
  // ------------------------------------------------------------------------------------------------------------------
  // Lambda computation
  // ------------------------------------------------------------------------------------------------------------------
  
  Int iQP;
  Double dOrigQP = dQP;

  // pre-compute lambda and QP values for all possible QP candidates
  if (pcPic->getSlice(0)->isIntra())
  {
    m_pcTrQuant->setRDOQOffset(1);
  }
  else
  {
    if (m_pcCfg->getHierarchicalCoding())
      m_pcTrQuant->setRDOQOffset(1);
    else
      m_pcTrQuant->setRDOQOffset(0);
  }

  for ( Int iDQpIdx = 0; iDQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; iDQpIdx++ )
  {
    // compute QP value
    dQP = dOrigQP + ((iDQpIdx+1)>>1)*(iDQpIdx%2 ? -1 : 1);
    
    // compute lambda value
    Int    NumberBFrames = ( m_pcCfg->getRateGOPSize() - 1 );
    Int    SHIFT_QP = 12;
    Double dLambda_scale = 1.0 - Clip3( 0.0, 0.5, 0.05*(Double)NumberBFrames );
#if FULL_NBIT
    Int    bitdepth_luma_qp_scale = 6 * (g_uiBitDepth - 8);
#else
    Int    bitdepth_luma_qp_scale = 0;
#endif
    Double qp_temp = (double) dQP + bitdepth_luma_qp_scale - SHIFT_QP;
#if FULL_NBIT
    Double qp_temp_orig = (double) dQP - SHIFT_QP;
#endif
    // Case #1: I or P-slices (key-frame)
    if ( iDepth == 0 )
    {
      if ( m_pcCfg->getUseRDOQ() && rpcSlice->isIntra() && dQP == dOrigQP )
      {
        dLambda = 0.57 * pow( 2.0, qp_temp/3.0 );
      }
      else
      {
        if ( NumberBFrames > 0 ) // HB structure or HP structure
        {
          dLambda = 0.68 * pow( 2.0, qp_temp/3.0 );
        }
        else                     // IPP structure
        {
          dLambda = 0.85 * pow( 2.0, qp_temp/3.0 );
        }
      }
      dLambda *= dLambda_scale;
    }
    else // P or B slices for HB or HP structure
    {
      dLambda = 0.68 * pow( 2.0, qp_temp/3.0 );
      if ( pcPic->getSlice(0)->isInterB () )
      {
#if FULL_NBIT
        dLambda *= Clip3( 2.00, 4.00, (qp_temp_orig / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#else
        dLambda *= Clip3( 2.00, 4.00, (qp_temp / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#endif
        if ( rpcSlice->isReferenced() ) // HB structure and referenced
        {
          dLambda *= 0.80;
          dLambda *= dLambda_scale;
        }
      }
      else
      {
        dLambda *= dLambda_scale;
      }
    }
    // if hadamard is used in ME process
    if ( !m_pcCfg->getUseHADME() ) dLambda *= 0.95;
    
    iQP = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );
    
    m_pdRdPicLambda[iDQpIdx] = dLambda;
    m_pdRdPicQp    [iDQpIdx] = dQP;
    m_piRdPicQp    [iDQpIdx] = iQP;
  }
  
  // obtain dQP = 0 case
  dLambda = m_pdRdPicLambda[0];
  dQP     = m_pdRdPicQp    [0];
  iQP     = m_piRdPicQp    [0];
  
  // store lambda
  m_pcRdCost ->setLambda( dLambda );
#if WEIGHTED_CHROMA_DISTORTION
// for RDO
  // in RdCost there is only one lambda because the luma and chroma bits are not separated, instead we weight the distortion of chroma.
  double weight = pow( 2.0, (iQP-g_aucChromaScale[iQP])/3.0 );  // takes into account of the chroma qp mapping without chroma qp Offset
  m_pcRdCost ->setChromaDistortionWeight( weight );     
#endif

#if RDOQ_CHROMA_LAMBDA 
// for RDOQ
  m_pcTrQuant->setLambda( dLambda, dLambda / weight );    
#else
  m_pcTrQuant->setLambda( dLambda );
#endif

#if ALF_CHROMA_LAMBDA || SAO_CHROMA_LAMBDA
// For ALF or SAO
  rpcSlice   ->setLambda( dLambda, dLambda / weight );  
#else
  rpcSlice   ->setLambda( dLambda );
#endif
  
#if HB_LAMBDA_FOR_LDC
  // restore original slice type
  if ( m_pcCfg->getUseLDC() )
  {
    eSliceType = P_SLICE;
  }
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType        ( eSliceType );
#endif
  
  rpcSlice->setSliceQp          ( iQP );
  rpcSlice->setSliceQpDelta     ( 0 );
  rpcSlice->setNumRefIdx        ( REF_PIC_LIST_0, eSliceType == P_SLICE ? m_pcCfg->getNumOfReference() : (eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L0()) : 0 ) );
  rpcSlice->setNumRefIdx        ( REF_PIC_LIST_1, eSliceType == B_SLICE ? (m_pcCfg->getNumOfReferenceB_L1()) : 0 );
  
  rpcSlice->setSymbolMode       ( m_pcCfg->getSymbolMode());
  rpcSlice->setLoopFilterDisable( m_pcCfg->getLoopFilterDisable() );
  
  rpcSlice->setDepth            ( iDepth );
  
  if ( pSPS->getMaxTLayers() > 1 )
  {
    assert( iDepth < pSPS->getMaxTLayers() );
    pcPic->setTLayer( iDepth );
  }
  else 
  {
    pcPic->setTLayer( 0 );
  }
  rpcSlice->setTLayer( pcPic->getTLayer() );
  rpcSlice->setTLayerSwitchingFlag( pPPS->getTLayerSwitchingFlag( pcPic->getTLayer() ) );

  rpcSlice->setSPS( pSPS );
  rpcSlice->setPPS( pPPS );

  // reference picture usage indicator for next frames
  rpcSlice->setDRBFlag          ( true );
  rpcSlice->setERBIndex         ( ERB_NONE );
  
  assert( m_apcPicYuvPred );
  assert( m_apcPicYuvResi );
  
  pcPic->setPicYuvPred( m_apcPicYuvPred );
  pcPic->setPicYuvResi( m_apcPicYuvResi );
  rpcSlice->setSliceMode            ( m_pcCfg->getSliceMode()            );
  rpcSlice->setSliceArgument        ( m_pcCfg->getSliceArgument()        );
  rpcSlice->setEntropySliceMode     ( m_pcCfg->getEntropySliceMode()     );
  rpcSlice->setEntropySliceArgument ( m_pcCfg->getEntropySliceArgument() );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSlice::setSearchRange( TComSlice* pcSlice )
{
  Int iCurrPOC = pcSlice->getPOC();
  Int iRefPOC;
  Int iRateGOPSize = m_pcCfg->getRateGOPSize();
  Int iOffset = (iRateGOPSize >> 1);
  Int iMaxSR = m_pcCfg->getSearchRange();
  Int iNumPredDir = pcSlice->isInterP() ? 1 : 2;
  
  for (Int iDir = 0; iDir <= iNumPredDir; iDir++)
  {
    RefPicList e = (RefPicList)iDir;
    for (Int iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(e); iRefIdx++)
    {
      iRefPOC = pcSlice->getRefPic(e, iRefIdx)->getPOC();
      Int iNewSR = Clip3(8, iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC)+iOffset)/iRateGOPSize);
      m_pcPredSearch->setAdaptiveSearchRange(iDir, iRefIdx, iNewSR);
    }
  }
}

/**
 - multi-loop slice encoding for different slice QP
 .
 \param rpcPic    picture class
 */
Void TEncSlice::precompressSlice( TComPic*& rpcPic )
{
  // if deltaQP RD is not used, simply return
  if ( m_pcCfg->getDeltaQpRD() == 0 ) return;
  
  TComSlice* pcSlice        = rpcPic->getSlice(getSliceIdx());
  Double     dPicRdCostBest = MAX_DOUBLE;
  UInt       uiQpIdxBest = 0;
  
  Double dFrameLambda;
#if FULL_NBIT
  Int    SHIFT_QP = 12 + 6 * (g_uiBitDepth - 8);
#else
  Int    SHIFT_QP = 12;
#endif
  
  // set frame lambda
  if (m_pcCfg->getGOPSize() > 1)
  {
    dFrameLambda = 0.68 * pow (2, (m_piRdPicQp[0]  - SHIFT_QP) / 3.0) * (pcSlice->isInterB()? 2 : 1);
  }
  else
  {
    dFrameLambda = 0.68 * pow (2, (m_piRdPicQp[0] - SHIFT_QP) / 3.0);
  }
  m_pcRdCost      ->setFrameLambda(dFrameLambda);
  
  // for each QP candidate
  for ( UInt uiQpIdx = 0; uiQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; uiQpIdx++ )
  {
    pcSlice       ->setSliceQp             ( m_piRdPicQp    [uiQpIdx] );
    m_pcRdCost    ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
#if WEIGHTED_CHROMA_DISTORTION
    // for RDO
    // in RdCost there is only one lambda because the luma and chroma bits are not separated, instead we weight the distortion of chroma.
    int iQP = m_piRdPicQp    [uiQpIdx];
    double weight = pow( 2.0, (iQP-g_aucChromaScale[iQP])/3.0 );  // takes into account of the chroma qp mapping without chroma qp Offset
    m_pcRdCost    ->setChromaDistortionWeight( weight );     
#endif

#if RDOQ_CHROMA_LAMBDA 
    // for RDOQ
    m_pcTrQuant   ->setLambda( m_pdRdPicLambda[uiQpIdx], m_pdRdPicLambda[uiQpIdx] / weight );
#else
    m_pcTrQuant   ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
#endif
#if ALF_CHROMA_LAMBDA || SAO_CHROMA_LAMBDA
    // For ALF or SAO
    pcSlice       ->setLambda              ( m_pdRdPicLambda[uiQpIdx], m_pdRdPicLambda[uiQpIdx] / weight ); 
#else
    pcSlice       ->setLambda              ( m_pdRdPicLambda[uiQpIdx] );
#endif
    
    // try compress
    compressSlice   ( rpcPic );
    
    Double dPicRdCost;
    UInt64 uiPicDist        = m_uiPicDist;
    UInt64 uiALFBits        = 0;
    
    m_pcGOPEncoder->preLoopFilterPicAll( rpcPic, uiPicDist, uiALFBits );
    
    // compute RD cost and choose the best
    dPicRdCost = m_pcRdCost->calcRdCost64( m_uiPicTotalBits + uiALFBits, uiPicDist, true, DF_SSE_FRAME);
    
    if ( dPicRdCost < dPicRdCostBest )
    {
      uiQpIdxBest    = uiQpIdx;
      dPicRdCostBest = dPicRdCost;
    }
  }
  
  // set best values
  pcSlice       ->setSliceQp             ( m_piRdPicQp    [uiQpIdxBest] );
  m_pcRdCost    ->setLambda              ( m_pdRdPicLambda[uiQpIdxBest] );
#if WEIGHTED_CHROMA_DISTORTION
  // in RdCost there is only one lambda because the luma and chroma bits are not separated, instead we weight the distortion of chroma.
  int iQP = m_piRdPicQp    [uiQpIdxBest];
  double weight = pow( 2.0, (iQP-g_aucChromaScale[iQP])/3.0 );  // takes into account of the chroma qp mapping without chroma qp Offset
  m_pcRdCost ->setChromaDistortionWeight( weight );     
#endif

#if RDOQ_CHROMA_LAMBDA 
  // for RDOQ 
  m_pcTrQuant   ->setLambda( m_pdRdPicLambda[uiQpIdxBest], m_pdRdPicLambda[uiQpIdxBest] / weight ); 
#else
  m_pcTrQuant   ->setLambda              ( m_pdRdPicLambda[uiQpIdxBest] );
#endif
#if ALF_CHROMA_LAMBDA || SAO_CHROMA_LAMBDA
  // For ALF or SAO
  pcSlice       ->setLambda              ( m_pdRdPicLambda[uiQpIdxBest], m_pdRdPicLambda[uiQpIdxBest] / weight ); 
#else
  pcSlice       ->setLambda              ( m_pdRdPicLambda[uiQpIdxBest] );
#endif
}

/** \param rpcPic   picture class
 */
Void TEncSlice::compressSlice( TComPic*& rpcPic )
{
  UInt  uiCUAddr;
  UInt   uiStartCUAddr;
  UInt   uiBoundingCUAddr;
#if FINE_GRANULARITY_SLICES
  rpcPic->getSlice(getSliceIdx())->setEntropySliceCounter(0);
#else
  UInt64 uiBitsCoded            = 0;
#endif
  TEncBinCABAC* pppcRDSbacCoder = NULL;
  TComSlice* pcSlice            = rpcPic->getSlice(getSliceIdx());
  xDetermineStartAndBoundingCUAddr ( uiStartCUAddr, uiBoundingCUAddr, rpcPic, false );
  
  // initialize cost values
  m_uiPicTotalBits  = 0;
  m_dPicRdCost      = 0;
  m_uiPicDist       = 0;
  
  // set entropy coder
  if( m_pcCfg->getUseSBACRD() )
  {
    m_pcSbacCoder->init( m_pcBinCABAC );
    m_pcEntropyCoder->setEntropyCoder   ( m_pcSbacCoder, pcSlice );
    m_pcEntropyCoder->resetEntropy      ();
    m_pppcRDSbacCoder[0][CI_CURR_BEST]->load(m_pcSbacCoder);
    pppcRDSbacCoder = (TEncBinCABAC *) m_pppcRDSbacCoder[0][CI_CURR_BEST]->getEncBinIf();
    pppcRDSbacCoder->setBinCountingEnableFlag( false );
    pppcRDSbacCoder->setBinsCoded( 0 );
  }
  else
  {
    m_pcCavlcCoder  ->setAdaptFlag    ( false );
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
    m_pcEntropyCoder->resetEntropy      ();
    m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  }
  
  // initialize ALF parameters
  m_pcEntropyCoder->setAlfCtrl(false);
  m_pcEntropyCoder->setMaxAlfCtrlDepth(0); //unnecessary
  
  // for every CU in slice
#if FINE_GRANULARITY_SLICES
  for(  uiCUAddr = uiStartCUAddr/rpcPic->getNumPartInCU(); uiCUAddr < (uiBoundingCUAddr+(rpcPic->getNumPartInCU()-1))/rpcPic->getNumPartInCU(); uiCUAddr++  )
#else
  for(  uiCUAddr = uiStartCUAddr; uiCUAddr < uiBoundingCUAddr; uiCUAddr++  )
#endif
  {
    // initialize CU encoder
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
    pcCU->initCU( rpcPic, uiCUAddr );

    // if RD based on SBAC is used
    if( m_pcCfg->getUseSBACRD() )
    {
      // set go-on entropy coder
      m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, pcSlice );
      m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
#if FINE_GRANULARITY_SLICES
      ((TEncBinCABAC*)m_pcRDGoOnSbacCoder->getEncBinIf())->setBinCountingEnableFlag(true);
#endif
      // run CU encoder
      m_pcCuEncoder->compressCU( pcCU );
      
      // restore entropy coder to an initial stage
      m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST], pcSlice );
      m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
      pppcRDSbacCoder->setBinCountingEnableFlag( true );
#if FINE_GRANULARITY_SLICES
      m_pcBitCounter->resetBits();
      pppcRDSbacCoder->setBinsCoded( 0 );
#endif
      m_pcCuEncoder->encodeCU( pcCU );

      pppcRDSbacCoder->setBinCountingEnableFlag( false );
#if FINE_GRANULARITY_SLICES
      if (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE && ( ( pcSlice->getSliceBits() + m_pcBitCounter->getNumberOfWrittenBits() ) ) > m_pcCfg->getSliceArgument()<<3)
      {
#else
      uiBitsCoded += m_pcBitCounter->getNumberOfWrittenBits();
      if (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE && ( ( pcSlice->getSliceBits() + uiBitsCoded ) >> 3 ) > m_pcCfg->getSliceArgument())
      {
        if (uiCUAddr==uiStartCUAddr && pcSlice->getSliceBits()==0)
        {
          // Could not fit even a single LCU within the slice under the defined byte-constraint. Display a warning message and code 1 LCU in the slice.
          fprintf(stdout,"Slice overflow warning! codedBits=%6d, limitBytes=%6d\n", m_pcBitCounter->getNumberOfWrittenBits(), m_pcCfg->getSliceArgument() );
          uiCUAddr = uiCUAddr + 1;
        }
#endif
        pcSlice->setNextSlice( true );
        break;
      }
#if FINE_GRANULARITY_SLICES
      if (m_pcCfg->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE && pcSlice->getEntropySliceCounter()+pppcRDSbacCoder->getBinsCoded() > m_pcCfg->getEntropySliceArgument()&&pcSlice->getSliceCurEndCUAddr()!=pcSlice->getEntropySliceCurEndCUAddr())
      {
#else
      
      UInt uiBinsCoded = pppcRDSbacCoder->getBinsCoded();
      if (m_pcCfg->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE && uiBinsCoded > m_pcCfg->getEntropySliceArgument())
      {
        if (uiCUAddr == uiStartCUAddr)
        {
          // Could not fit even a single LCU within the entropy slice under the defined byte-constraint. Display a warning message and code 1 LCU in the entropy slice.
          fprintf(stdout,"Entropy Slice overflow warning! codedBins=%6d, limitBins=%6d\n", uiBinsCoded, m_pcCfg->getEntropySliceArgument() );
          uiCUAddr = uiCUAddr + 1;
        }
#endif
#if !FINE_GRANULARITY_SLICES
        uiBitsCoded -= m_pcBitCounter->getNumberOfWrittenBits();
#endif
        pcSlice->setNextEntropySlice( true );
        break;
      }
    }
    // other case: encodeCU is not called
    else
    {
      m_pcCuEncoder->compressCU( pcCU );
      m_pcCavlcCoder ->setAdaptFlag(true);
      m_pcCuEncoder->encodeCU( pcCU );
      
#if FINE_GRANULARITY_SLICES
      if (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE && ( ( pcSlice->getSliceBits()+ m_pcBitCounter->getNumberOfWrittenBits() ) ) > m_pcCfg->getSliceArgument()<<3)
      {
#else
      uiBitsCoded += m_pcBitCounter->getNumberOfWrittenBits();
      if (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE && ( ( pcSlice->getSliceBits() + uiBitsCoded ) >> 3 ) > m_pcCfg->getSliceArgument())
      {
        if (uiCUAddr==uiStartCUAddr && pcSlice->getSliceBits()==0)
        {
          // Could not fit even a single LCU within the slice under the defined byte-constraint. Display a warning message and code 1 LCU in the slice.
          fprintf(stdout,"Slice overflow warning! codedBits=%6d, limitBytes=%6d\n", m_pcBitCounter->getNumberOfWrittenBits(), m_pcCfg->getSliceArgument() );
          uiCUAddr = uiCUAddr + 1;
        }
#endif
        pcSlice->setNextSlice( true );
        break;
      }
#if FINE_GRANULARITY_SLICES
      if (m_pcCfg->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE && pcSlice->getEntropySliceCounter()+ m_pcBitCounter->getNumberOfWrittenBits()> m_pcCfg->getEntropySliceArgument()&&pcSlice->getSliceCurEndCUAddr()!=pcSlice->getEntropySliceCurEndCUAddr())
      {

#else
      if (m_pcCfg->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE && uiBitsCoded > m_pcCfg->getEntropySliceArgument())
      {
        if (uiCUAddr == uiStartCUAddr)
        {
          // Could not fit even a single LCU within the entropy slice under the defined bit/bin-constraint. Display a warning message and code 1 LCU in the entropy slice.
          fprintf(stdout,"Entropy Slice overflow warning! codedBits=%6d, limitBits=%6d\n", m_pcBitCounter->getNumberOfWrittenBits(), m_pcCfg->getEntropySliceArgument() );
          uiCUAddr = uiCUAddr + 1;
        }
#endif

#if !FINE_GRANULARITY_SLICES
        uiBitsCoded -= m_pcBitCounter->getNumberOfWrittenBits();
#endif
        pcSlice->setNextEntropySlice( true );
        break;
      }
      m_pcCavlcCoder ->setAdaptFlag(false);
    }
    
    m_uiPicTotalBits += pcCU->getTotalBits();
    m_dPicRdCost     += pcCU->getTotalCost();
    m_uiPicDist      += pcCU->getTotalDistortion();
  }
#if !FINE_GRANULARITY_SLICES
  pcSlice->setSliceCurEndCUAddr( uiCUAddr );
  pcSlice->setEntropySliceCurEndCUAddr( uiCUAddr );
  pcSlice->setSliceBits( (UInt)(pcSlice->getSliceBits() + uiBitsCoded) );
#endif
}

/**
 \param  rpcPic        picture class
 \retval rpcBitstream  bitstream class
 */
Void TEncSlice::encodeSlice   ( TComPic*& rpcPic, TComOutputBitstream* pcBitstream )
{
  UInt       uiCUAddr;
  UInt       uiStartCUAddr;
  UInt       uiBoundingCUAddr;
#if !FINE_GRANULARITY_SLICES
  xDetermineStartAndBoundingCUAddr  ( uiStartCUAddr, uiBoundingCUAddr, rpcPic, true );
#endif
  TComSlice* pcSlice = rpcPic->getSlice(getSliceIdx());

#if FINE_GRANULARITY_SLICES
  uiStartCUAddr=pcSlice->getEntropySliceCurStartCUAddr();
  uiBoundingCUAddr=pcSlice->getEntropySliceCurEndCUAddr();
#endif
  // choose entropy coder
  Int iSymbolMode = pcSlice->getSymbolMode();
  if (iSymbolMode)
  {
    m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
    m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
  }
  else
  {
    m_pcCavlcCoder  ->setAdaptFlag( true );
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
    m_pcEntropyCoder->resetEntropy();
  }
  
  // set bitstream
  m_pcEntropyCoder->setBitstream( pcBitstream );
  // for every CU
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceEnable;
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tPOC: " );
  DTRACE_CABAC_V( rpcPic->getPOC() );
  DTRACE_CABAC_T( "\n" );
#if ENC_DEC_TRACE
  g_bJustDoIt = g_bEncDecTraceDisable;
#endif

#if FINE_GRANULARITY_SLICES
  for(  uiCUAddr = uiStartCUAddr/rpcPic->getNumPartInCU(); uiCUAddr<(uiBoundingCUAddr+rpcPic->getNumPartInCU()-1)/rpcPic->getNumPartInCU(); uiCUAddr++  )
#else
  for(  uiCUAddr = uiStartCUAddr; uiCUAddr<uiBoundingCUAddr; uiCUAddr++  )
#endif

  {
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceEnable;
#endif
    if ( (m_pcCfg->getSliceMode()!=0 || m_pcCfg->getEntropySliceMode()!=0) && uiCUAddr==uiBoundingCUAddr-1 )
    {
      m_pcCuEncoder->encodeCU( pcCU, true );
    }
    else
    {
      m_pcCuEncoder->encodeCU( pcCU );
    }
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceDisable;
#endif    
  }
}

/** Determines the starting and bounding LCU address of current slice / entropy slice
 * \param bEncodeSlice Identifies if the calling function is compressSlice() [false] or encodeSlice() [true]
 * \returns Updates uiStartCUAddr, uiBoundingCUAddr with appropriate LCU address
 */
Void TEncSlice::xDetermineStartAndBoundingCUAddr  ( UInt& uiStartCUAddr, UInt& uiBoundingCUAddr, TComPic*& rpcPic, Bool bEncodeSlice )
{
  TComSlice* pcSlice = rpcPic->getSlice(getSliceIdx());
  UInt uiStartCUAddrSlice, uiBoundingCUAddrSlice;
  uiStartCUAddrSlice        = pcSlice->getSliceCurStartCUAddr();
  UInt uiNumberOfCUsInFrame = rpcPic->getNumCUsInFrame();
  uiBoundingCUAddrSlice     = uiNumberOfCUsInFrame;
  if (bEncodeSlice) 
  {
    UInt uiCUAddrIncrement;
    switch (m_pcCfg->getSliceMode())
    {
    case AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE:
      uiCUAddrIncrement        = m_pcCfg->getSliceArgument();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrSlice    = ((uiStartCUAddrSlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU()) ? (uiStartCUAddrSlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrSlice    = ((uiStartCUAddrSlice + uiCUAddrIncrement     ) < uiNumberOfCUsInFrame ) ? (uiStartCUAddrSlice + uiCUAddrIncrement     ) : uiNumberOfCUsInFrame;
#endif
      break;
    case AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE:
      uiCUAddrIncrement        = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrSlice    = pcSlice->getSliceCurEndCUAddr();
      break;
    default:
      uiCUAddrIncrement        = rpcPic->getNumCUsInFrame();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrSlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrSlice    = uiNumberOfCUsInFrame;
#endif
      break;
    } 
    pcSlice->setSliceCurEndCUAddr( uiBoundingCUAddrSlice );
  }
  else
  {
    UInt uiCUAddrIncrement     ;
    switch (m_pcCfg->getSliceMode())
    {
    case AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE:
      uiCUAddrIncrement        = m_pcCfg->getSliceArgument();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrSlice    = ((uiStartCUAddrSlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU()) ? (uiStartCUAddrSlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrSlice    = ((uiStartCUAddrSlice + uiCUAddrIncrement     ) < uiNumberOfCUsInFrame ) ? (uiStartCUAddrSlice + uiCUAddrIncrement     ) : uiNumberOfCUsInFrame;
#endif
      break;
    default:
      uiCUAddrIncrement        = rpcPic->getNumCUsInFrame();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrSlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrSlice    = uiNumberOfCUsInFrame;
#endif
      break;
    } 
    pcSlice->setSliceCurEndCUAddr( uiBoundingCUAddrSlice );
  }

  // Entropy slice
  UInt uiStartCUAddrEntropySlice, uiBoundingCUAddrEntropySlice;
  uiStartCUAddrEntropySlice    = pcSlice->getEntropySliceCurStartCUAddr();
  uiBoundingCUAddrEntropySlice = uiNumberOfCUsInFrame;
  if (bEncodeSlice) 
  {
    UInt uiCUAddrIncrement;
    switch (m_pcCfg->getEntropySliceMode())
    {
    case SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE:
      uiCUAddrIncrement               = m_pcCfg->getEntropySliceArgument();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrEntropySlice    = ((uiStartCUAddrEntropySlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU() ) ? (uiStartCUAddrEntropySlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrEntropySlice    = ((uiStartCUAddrEntropySlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame ) ? (uiStartCUAddrEntropySlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame;
#endif
      break;
    case SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE:
      uiCUAddrIncrement               = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrEntropySlice    = pcSlice->getEntropySliceCurEndCUAddr();
      break;
    default:
      uiCUAddrIncrement               = rpcPic->getNumCUsInFrame();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrEntropySlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrEntropySlice    = uiNumberOfCUsInFrame;
#endif
      break;
    } 
    pcSlice->setEntropySliceCurEndCUAddr( uiBoundingCUAddrEntropySlice );
  }
  else
  {
    UInt uiCUAddrIncrement;
    switch (m_pcCfg->getEntropySliceMode())
    {
    case SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE:
      uiCUAddrIncrement               = m_pcCfg->getEntropySliceArgument();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrEntropySlice    = ((uiStartCUAddrEntropySlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU() ) ? (uiStartCUAddrEntropySlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrEntropySlice    = ((uiStartCUAddrEntropySlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame ) ? (uiStartCUAddrEntropySlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame;
#endif
      break;
    default:
      uiCUAddrIncrement               = rpcPic->getNumCUsInFrame();
#if FINE_GRANULARITY_SLICES
      uiBoundingCUAddrEntropySlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
#else
      uiBoundingCUAddrEntropySlice    = uiNumberOfCUsInFrame;
#endif
      break;
    } 
    pcSlice->setEntropySliceCurEndCUAddr( uiBoundingCUAddrEntropySlice );
  }
#if FINE_GRANULARITY_SLICES
  if(uiBoundingCUAddrEntropySlice>uiBoundingCUAddrSlice)
  {
    uiBoundingCUAddrEntropySlice = uiBoundingCUAddrSlice;
    pcSlice->setEntropySliceCurEndCUAddr(uiBoundingCUAddrSlice);
  }
  //calculate real entropy slice start address
  UInt uiInternalAddress = (pcSlice->getEntropySliceCurStartCUAddr()) % rpcPic->getNumPartInCU();
  UInt uiExternalAddress = (pcSlice->getEntropySliceCurStartCUAddr()) / rpcPic->getNumPartInCU();
  UInt uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiWidth = pcSlice->getSPS()->getWidth();
  UInt uiHeight = pcSlice->getSPS()->getHeight();
  while((uiPosX>=uiWidth||uiPosY>=uiHeight)&&!(uiPosX>=uiWidth&&uiPosY>=uiHeight)) {
    uiInternalAddress++;
    if(uiInternalAddress>=rpcPic->getNumPartInCU())
    {
      uiInternalAddress=0;
      uiExternalAddress++;
    }
    uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
    uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  }
  UInt uiRealStartAddress = uiExternalAddress*rpcPic->getNumPartInCU()+uiInternalAddress;
  
  pcSlice->setEntropySliceCurStartCUAddr(uiRealStartAddress);
  uiStartCUAddrEntropySlice=uiRealStartAddress;
  
  //calculate real slice start address
  uiInternalAddress = (pcSlice->getSliceCurStartCUAddr()) % rpcPic->getNumPartInCU();
  uiExternalAddress = (pcSlice->getSliceCurStartCUAddr()) / rpcPic->getNumPartInCU();
  uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
  uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  uiWidth = pcSlice->getSPS()->getWidth();
  uiHeight = pcSlice->getSPS()->getHeight();
  while((uiPosX>=uiWidth||uiPosY>=uiHeight)&&!(uiPosX>=uiWidth&&uiPosY>=uiHeight)) {
    uiInternalAddress++;
    if(uiInternalAddress>=rpcPic->getNumPartInCU())
    {
      uiInternalAddress=0;
      uiExternalAddress++;
    }
    uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
    uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  }
  uiRealStartAddress = uiExternalAddress*rpcPic->getNumPartInCU()+uiInternalAddress;
  
  pcSlice->setSliceCurStartCUAddr(uiRealStartAddress);
  uiStartCUAddrSlice=uiRealStartAddress;
  
  
#endif
  // Make a joint decision based on reconstruction and entropy slice bounds
  uiStartCUAddr    = max(uiStartCUAddrSlice   , uiStartCUAddrEntropySlice   );
  uiBoundingCUAddr = min(uiBoundingCUAddrSlice, uiBoundingCUAddrEntropySlice);


  if (!bEncodeSlice)
  {
    // For fixed number of LCU within an entropy and reconstruction slice we already know whether we will encounter end of entropy and/or reconstruction slice
    // first. Set the flags accordingly.
    if ( (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE && m_pcCfg->getEntropySliceMode()==SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE)
      || (m_pcCfg->getSliceMode()==0 && m_pcCfg->getEntropySliceMode()==SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE)
      || (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE && m_pcCfg->getEntropySliceMode()==0) )
    {
      if (uiBoundingCUAddrSlice < uiBoundingCUAddrEntropySlice)
      {
        pcSlice->setNextSlice       ( true );
        pcSlice->setNextEntropySlice( false );
      }
      else if (uiBoundingCUAddrSlice > uiBoundingCUAddrEntropySlice)
      {
        pcSlice->setNextSlice       ( false );
        pcSlice->setNextEntropySlice( true );
      }
      else
      {
        pcSlice->setNextSlice       ( true );
        pcSlice->setNextEntropySlice( true );
      }
    }
    else
    {
      pcSlice->setNextSlice       ( false );
      pcSlice->setNextEntropySlice( false );
    }
  }
}
//! \}
