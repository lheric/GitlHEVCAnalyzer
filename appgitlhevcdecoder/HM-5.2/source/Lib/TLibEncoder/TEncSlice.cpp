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
  m_pcBufferSbacCoders    = NULL;
  m_pcBufferBinCoderCABACs  = NULL;
  m_pcBufferLowLatSbacCoders    = NULL;
  m_pcBufferLowLatBinCoderCABACs  = NULL;
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

  if ( m_pcBufferSbacCoders )
  {
    delete[] m_pcBufferSbacCoders;
  }
  if ( m_pcBufferBinCoderCABACs )
  {
    delete[] m_pcBufferBinCoderCABACs;
  }
  if ( m_pcBufferLowLatSbacCoders )
    delete[] m_pcBufferLowLatSbacCoders;
  if ( m_pcBufferLowLatBinCoderCABACs )
    delete[] m_pcBufferLowLatBinCoderCABACs;
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
Void TEncSlice::initEncSlice( TComPic* pcPic, Int iPOCLast, UInt uiPOCCurr, Int iNumPicRcvd, Int iGOPid, TComSlice*& rpcSlice, TComSPS* pSPS, TComPPS *pPPS )
{
  Double dQP;
  Double dLambda;
  
  rpcSlice = pcPic->getSlice(0);
  rpcSlice->setSPS( pSPS );
  rpcSlice->setPPS( pPPS );
  rpcSlice->setSliceBits(0);
  rpcSlice->setPic( pcPic );
  rpcSlice->initSlice();
#if PARAMSET_VLC_CLEANUP
  rpcSlice->initTiles();
#endif

  rpcSlice->setPOC( uiPOCCurr );
  
  // depth computation based on GOP size
  int iDepth;
  {
    Int i, j;
    Int iPOC = rpcSlice->getPOC()%m_pcCfg->getGOPSize();
    if ( iPOC == 0 )
    {
      iDepth = 0;
    }
    else
    {
      Int iStep = m_pcCfg->getGOPSize();
      iDepth    = 0;
      for( i=iStep>>1; i>=1; i>>=1 )
      {
        for ( j=i; j<m_pcCfg->getGOPSize(); j+=iStep )
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
  
  eSliceType=B_SLICE;
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType    ( eSliceType );
  
  // ------------------------------------------------------------------------------------------------------------------
  // Non-referenced frame marking
  // ------------------------------------------------------------------------------------------------------------------
  
  rpcSlice->setReferenced(m_pcCfg->getGOPEntry(iGOPid).m_bRefPic);
  if(eSliceType==I_SLICE)
  {
    rpcSlice->setReferenced(true);
  }
  
  // ------------------------------------------------------------------------------------------------------------------
  // QP setting
  // ------------------------------------------------------------------------------------------------------------------
  
  dQP = m_pcCfg->getQP();
  if(eSliceType!=I_SLICE)
  {
    dQP += m_pcCfg->getGOPEntry(iGOPid).m_iQPOffset;
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
  for ( Int iDQpIdx = 0; iDQpIdx < 2 * m_pcCfg->getDeltaQpRD() + 1; iDQpIdx++ )
  {
    // compute QP value
    dQP = dOrigQP + ((iDQpIdx+1)>>1)*(iDQpIdx%2 ? -1 : 1);
    
    // compute lambda value
    Int    NumberBFrames = ( m_pcCfg->getGOPSize() - 1 );
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
    Double dQPFactor = m_pcCfg->getGOPEntry(iGOPid).m_iQPFactor;
    if ( eSliceType==I_SLICE )
    {
      dQPFactor=0.57*dLambda_scale;
    }
    dLambda = dQPFactor*pow( 2.0, qp_temp/3.0 );

    if ( iDepth>0 )
    {
#if FULL_NBIT
        dLambda *= Clip3( 2.00, 4.00, (qp_temp_orig / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#else
        dLambda *= Clip3( 2.00, 4.00, (qp_temp / 6.0) ); // (j == B_SLICE && p_cur_frm->layer != 0 )
#endif
    }
    
    // if hadamard is used in ME process
    if ( !m_pcCfg->getUseHADME() )
    {
      dLambda *= 0.95;
    }
    
    iQP = max( MIN_QP, min( MAX_QP, (Int)floor( dQP + 0.5 ) ) );
    
    m_pdRdPicLambda[iDQpIdx] = dLambda;
    m_pdRdPicQp    [iDQpIdx] = dQP;
    m_piRdPicQp    [iDQpIdx] = iQP;
  }
  
  // obtain dQP = 0 case
  dLambda = m_pdRdPicLambda[0];
  dQP     = m_pdRdPicQp    [0];
  iQP     = m_piRdPicQp    [0];
  
  if( rpcSlice->getSliceType( ) != I_SLICE )
  {
    dLambda *= m_pcCfg->getLambdaModifier( iDepth );
  }

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
  eSliceType = (iPOCLast == 0 || uiPOCCurr % m_pcCfg->getIntraPeriod() == 0 || m_pcGOPEncoder->getGOPSize() == 0) ? I_SLICE : eSliceType;
  
  rpcSlice->setSliceType        ( eSliceType );
#endif
  
  rpcSlice->setSliceQp          ( iQP );
#if ADAPTIVE_QP_SELECTION
  rpcSlice->setSliceQpBase      ( iQP );
#endif
  rpcSlice->setSliceQpDelta     ( 0 );
  rpcSlice->setNumRefIdx(REF_PIC_LIST_0,m_pcCfg->getGOPEntry(iGOPid).m_iRefBufSize);
  rpcSlice->setNumRefIdx(REF_PIC_LIST_1,m_pcCfg->getGOPEntry(iGOPid).m_iRefBufSize);
  
  rpcSlice->setLoopFilterOffsetInAPS( m_pcCfg->getLoopFilterOffsetInAPS() );
  rpcSlice->setInheritDblParamFromAPS( m_pcCfg->getLoopFilterOffsetInAPS() ? 1 : 0 );
  rpcSlice->setLoopFilterDisable( m_pcCfg->getLoopFilterDisable() );
  if ( !rpcSlice->getLoopFilterDisable())
  {
    rpcSlice->setLoopFilterBetaOffset( m_pcCfg->getLoopFilterBetaOffset() );
    rpcSlice->setLoopFilterTcOffset( m_pcCfg->getLoopFilterTcOffset() );
  }

  rpcSlice->setDepth            ( iDepth );
  
  pcPic->setTLayer( m_pcCfg->getGOPEntry(iGOPid).m_iTemporalId );
  if(eSliceType==I_SLICE)
  {
    pcPic->setTLayer(0);
  }
  rpcSlice->setTLayer( pcPic->getTLayer() );
  rpcSlice->setTLayerSwitchingFlag( pPPS->getTLayerSwitchingFlag( pcPic->getTLayer() ) );

  assert( m_apcPicYuvPred );
  assert( m_apcPicYuvResi );
  
  pcPic->setPicYuvPred( m_apcPicYuvPred );
  pcPic->setPicYuvResi( m_apcPicYuvResi );
  rpcSlice->setSliceMode            ( m_pcCfg->getSliceMode()            );
  rpcSlice->setSliceArgument        ( m_pcCfg->getSliceArgument()        );
  rpcSlice->setEntropySliceMode     ( m_pcCfg->getEntropySliceMode()     );
  rpcSlice->setEntropySliceArgument ( m_pcCfg->getEntropySliceArgument() );
  rpcSlice->setMaxNumMergeCand      (MRG_MAX_NUM_CANDS_SIGNALED);
  xStoreWPparam( pPPS->getUseWP(), pPPS->getWPBiPredIdc() );
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSlice::setSearchRange( TComSlice* pcSlice )
{
  Int iCurrPOC = pcSlice->getPOC();
  Int iRefPOC;
  Int iGOPSize = m_pcCfg->getGOPSize();
  Int iOffset = (iGOPSize >> 1);
  Int iMaxSR = m_pcCfg->getSearchRange();
  Int iNumPredDir = pcSlice->isInterP() ? 1 : 2;
  
  for (Int iDir = 0; iDir <= iNumPredDir; iDir++)
  {
    RefPicList e = (RefPicList)iDir;
    for (Int iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(e); iRefIdx++)
    {
      iRefPOC = pcSlice->getRefPic(e, iRefIdx)->getPOC();
      Int iNewSR = Clip3(8, iMaxSR, (iMaxSR*ADAPT_SR_SCALE*abs(iCurrPOC - iRefPOC)+iOffset)/iGOPSize);
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
  if ( m_pcCfg->getDeltaQpRD() == 0 )
  {
    return;
  }
  
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
#if ADAPTIVE_QP_SELECTION
    pcSlice       ->setSliceQpBase         ( m_piRdPicQp    [uiQpIdx] );
#endif
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
#if ADAPTIVE_QP_SELECTION
  pcSlice       ->setSliceQpBase         ( m_piRdPicQp    [uiQpIdxBest] );
#endif
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
  rpcPic->getSlice(getSliceIdx())->setEntropySliceCounter(0);
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
    m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );
    m_pcEntropyCoder->resetEntropy      ();
    m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  }
  
  //------------------------------------------------------------------------------
  //  Weighted Prediction parameters estimation.
  //------------------------------------------------------------------------------
  // calculate AC/DC values for current picture
  if( pcSlice->getPPS()->getUseWP() || pcSlice->getPPS()->getWPBiPredIdc() )
  {
    xCalcACDCParamSlice(pcSlice);
  }

  Bool bWp_explicit = (pcSlice->getSliceType()==P_SLICE && pcSlice->getPPS()->getUseWP()) || (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==1);
  Bool bWp_implicit = (pcSlice->getSliceType()==B_SLICE && pcSlice->getPPS()->getWPBiPredIdc()==2);

  if ( bWp_explicit || bWp_implicit )
  {
    //------------------------------------------------------------------------------
    //  Weighted Prediction implemented at Slice level. SliceMode=2 is not supported yet.
    //------------------------------------------------------------------------------
    if ( pcSlice->getSliceMode()==2 || pcSlice->getEntropySliceMode()==2 )
    {
      printf("Weighted Prediction is not supported with slice mode determined by max number of bins.\n"); exit(0);
    }

    if( bWp_explicit )
    {
      xEstimateWPParamSlice( pcSlice );
    }
    
    pcSlice->initWpScaling();

    // check WP on/off
    if( bWp_explicit )
    {
      xCheckWPEnable( pcSlice );
    }
  }

#if ADAPTIVE_QP_SELECTION
  if( m_pcCfg->getUseAdaptQpSelect() )
  {
    m_pcTrQuant->clearSliceARLCnt();
    if(pcSlice->getSliceType()!=I_SLICE)
    {
      Int qpBase = pcSlice->getSliceQpBase();
      pcSlice->setSliceQp(qpBase + m_pcTrQuant->getQpDelta(qpBase));
    }
  }
#endif
  // initialize ALF parameters
  m_pcEntropyCoder->setAlfCtrl(false);
  m_pcEntropyCoder->setMaxAlfCtrlDepth(0); //unnecessary
  
  TEncTop* pcEncTop = (TEncTop*) m_pcCfg;
  TEncSbac**** ppppcRDSbacCoders    = pcEncTop->getRDSbacCoders();
  TComBitCounter* pcBitCounters     = pcEncTop->getBitCounters();
  Int  iNumSubstreams = 1;
  UInt uiTilesAcross  = 0;

  if( m_pcCfg->getUseSBACRD() )
  {
    iNumSubstreams = pcSlice->getPPS()->getNumSubstreams();
    uiTilesAcross = rpcPic->getPicSym()->getNumColumnsMinus1()+1;
    delete[] m_pcBufferSbacCoders;
    delete[] m_pcBufferBinCoderCABACs;
    m_pcBufferSbacCoders     = new TEncSbac    [uiTilesAcross];
    m_pcBufferBinCoderCABACs = new TEncBinCABAC[uiTilesAcross];
    for (int ui = 0; ui < uiTilesAcross; ui++)
    {
      m_pcBufferSbacCoders[ui].init( &m_pcBufferBinCoderCABACs[ui] );
    }
    for (UInt ui = 0; ui < uiTilesAcross; ui++)
    {
      m_pcBufferSbacCoders[ui].load(m_pppcRDSbacCoder[0][CI_CURR_BEST]);  //init. state
    }

    for ( UInt ui = 0 ; ui < iNumSubstreams ; ui++ ) //init all sbac coders for RD optimization
    {
      ppppcRDSbacCoders[ui][0][CI_CURR_BEST]->load(m_pppcRDSbacCoder[0][CI_CURR_BEST]);
    }
  }
  //if( m_pcCfg->getUseSBACRD() )
  {
    delete[] m_pcBufferLowLatSbacCoders;
    delete[] m_pcBufferLowLatBinCoderCABACs;
    m_pcBufferLowLatSbacCoders     = new TEncSbac    [uiTilesAcross];
    m_pcBufferLowLatBinCoderCABACs = new TEncBinCABAC[uiTilesAcross];
    for (int ui = 0; ui < uiTilesAcross; ui++)
    {
      m_pcBufferLowLatSbacCoders[ui].init( &m_pcBufferLowLatBinCoderCABACs[ui] );
    }
    for (UInt ui = 0; ui < uiTilesAcross; ui++)
      m_pcBufferLowLatSbacCoders[ui].load(m_pppcRDSbacCoder[0][CI_CURR_BEST]);  //init. state
  }
  UInt uiWidthInLCUs  = rpcPic->getPicSym()->getFrameWidthInCU();
  //UInt uiHeightInLCUs = rpcPic->getPicSym()->getFrameHeightInCU();
  UInt uiCol=0, uiLin=0, uiSubStrm=0;
  Int  iBreakDep      = 0;
  UInt uiTileCol      = 0;
  UInt uiTileStartLCU = 0;
  UInt uiTileLCUX     = 0;

  // for every CU in slice
  UInt uiEncCUOrder;
  uiCUAddr = rpcPic->getPicSym()->getCUOrderMap( uiStartCUAddr /rpcPic->getNumPartInCU()); 
  for( uiEncCUOrder = uiStartCUAddr/rpcPic->getNumPartInCU();
       uiEncCUOrder < (uiBoundingCUAddr+(rpcPic->getNumPartInCU()-1))/rpcPic->getNumPartInCU();
       uiCUAddr = rpcPic->getPicSym()->getCUOrderMap(++uiEncCUOrder) )
  {
    // initialize CU encoder
    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );
    pcCU->initCU( rpcPic, uiCUAddr );

    // inherit from TR if necessary, select substream to use.
    if( m_pcCfg->getUseSBACRD() )
    {
      iBreakDep = rpcPic->getPicSym()->getTileBoundaryIndependenceIdr();
      uiTileCol = rpcPic->getPicSym()->getTileIdxMap(uiCUAddr) % (rpcPic->getPicSym()->getNumColumnsMinus1()+1); // what column of tiles are we in?
      uiTileStartLCU = rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr();
      uiTileLCUX = uiTileStartLCU % uiWidthInLCUs;
      //UInt uiSliceStartLCU = pcSlice->getSliceCurStartCUAddr();
      uiCol     = uiCUAddr % uiWidthInLCUs;
      uiLin     = uiCUAddr / uiWidthInLCUs;
      if (iBreakDep && pcSlice->getPPS()->getEntropyCodingSynchro())
      {
        // independent tiles => substreams are "per tile".  iNumSubstreams has already been multiplied.
        Int iNumSubstreamsPerTile = iNumSubstreams/rpcPic->getPicSym()->getNumTiles();
        uiSubStrm = rpcPic->getPicSym()->getTileIdxMap(uiCUAddr)*iNumSubstreamsPerTile
                      + uiLin%iNumSubstreamsPerTile;
      }
      else
      {
        // dependent tiles => substreams are "per frame".
        uiSubStrm = uiLin % iNumSubstreams;
      }

      if ( pcSlice->getPPS()->getEntropyCodingSynchro() && (uiCol == uiTileLCUX) )
      {
        // We'll sync if the TR is available.
        TComDataCU *pcCUUp = pcCU->getCUAbove();
        UInt uiWidthInCU = rpcPic->getFrameWidthInCU();
        UInt uiMaxParts = 1<<(pcSlice->getSPS()->getMaxCUDepth()<<1);
        TComDataCU *pcCUTR = NULL;
        if ( pcCUUp && ((uiCUAddr%uiWidthInCU+pcSlice->getPPS()->getEntropyCodingSynchro()) < uiWidthInCU)  )
        {
          pcCUTR = rpcPic->getCU( uiCUAddr - uiWidthInCU + pcSlice->getPPS()->getEntropyCodingSynchro() );
        }
        if ( (true/*bEnforceSliceRestriction*/ &&
             ((pcCUTR==NULL) || (pcCUTR->getSlice()==NULL) || 
             (pcCUTR->getSCUAddr()+uiMaxParts-1 < pcSlice->getSliceCurStartCUAddr()) ||
             (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && (rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(uiCUAddr)))
             ))||
             (true/*bEnforceEntropySliceRestriction*/ &&
             ((pcCUTR==NULL) || (pcCUTR->getSlice()==NULL) || 
             (pcCUTR->getSCUAddr()+uiMaxParts-1 < pcSlice->getEntropySliceCurStartCUAddr()) ||
             (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && (rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(uiCUAddr)))
             ))
           )
        {
          // TR not available.
        }
        else
        {
          // TR is available, we use it.
          ppppcRDSbacCoders[uiSubStrm][0][CI_CURR_BEST]->loadContexts( &m_pcBufferSbacCoders[uiTileCol] );
        }
      }
      m_pppcRDSbacCoder[0][CI_CURR_BEST]->load( ppppcRDSbacCoders[uiSubStrm][0][CI_CURR_BEST] ); //this load is used to simplify the code
  }

    // reset the entropy coder
    if( uiCUAddr == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr() &&                                   // must be first CU of tile
        uiCUAddr!=0 &&                                                                                                                                    // cannot be first CU of picture
        uiCUAddr!=rpcPic->getPicSym()->getPicSCUAddr(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr())/rpcPic->getNumPartInCU())     // cannot be first CU of slice
    {
      {
        m_pcEntropyCoder->updateContextTables ( pcSlice->getSliceType(), pcSlice->getSliceQp(), false );
        m_pcEntropyCoder->setEntropyCoder     ( m_pppcRDSbacCoder[0][CI_CURR_BEST], pcSlice );
        m_pcEntropyCoder->updateContextTables ( pcSlice->getSliceType(), pcSlice->getSliceQp() );
        m_pcEntropyCoder->setEntropyCoder     ( m_pcSbacCoder, pcSlice );
      }
    }

    if( (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()==0) && (rpcPic->getPicSym()->getNumColumnsMinus1()!=0) )
    {
      // Synchronize cabac probabilities with LCU among Tiles
      if( (uiTileLCUX != 0) &&
          (uiCUAddr == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr()) )
      { 
        TComDataCU *pcCULeft = pcCU->getCULeft();
        UInt uiMaxParts = 1<<(pcSlice->getSPS()->getMaxCUDepth()<<1);

        if ( (true/*bEnforceSliceRestriction*/ &&
              ((pcCULeft==NULL) || (pcCULeft->getSlice()==NULL) || 
               ((pcCULeft->getSCUAddr()+uiMaxParts-1) < pcSlice->getSliceCurStartCUAddr()) 
              )
             )||
             (true/*bEnforceEntropySliceRestriction*/ &&
              ((pcCULeft==NULL) || (pcCULeft->getSlice()==NULL) || 
               ((pcCULeft->getSCUAddr()+uiMaxParts-1) < pcSlice->getEntropySliceCurStartCUAddr())
              )
             )
           )
        {
          // Left not available.
        }
        else
        {
          // Left is available, we use it.
          ppppcRDSbacCoders[uiSubStrm][0][CI_CURR_BEST]->loadContexts( &m_pcBufferLowLatSbacCoders[uiTileCol-1] );
          m_pppcRDSbacCoder[0][CI_CURR_BEST]->loadContexts( ppppcRDSbacCoders[uiSubStrm][0][CI_CURR_BEST] ); //this load is used to simplify the code
        }
      }
    }

    // if RD based on SBAC is used
    if( m_pcCfg->getUseSBACRD() )
    {
      // set go-on entropy coder
      m_pcEntropyCoder->setEntropyCoder ( m_pcRDGoOnSbacCoder, pcSlice );
      m_pcEntropyCoder->setBitstream( &pcBitCounters[uiSubStrm] );
      
      ((TEncBinCABAC*)m_pcRDGoOnSbacCoder->getEncBinIf())->setBinCountingEnableFlag(true);
      // run CU encoder
      m_pcCuEncoder->compressCU( pcCU );
      
      // restore entropy coder to an initial stage
      m_pcEntropyCoder->setEntropyCoder ( m_pppcRDSbacCoder[0][CI_CURR_BEST], pcSlice );
      m_pcEntropyCoder->setBitstream( &pcBitCounters[uiSubStrm] );
      m_pcCuEncoder->setBitCounter( &pcBitCounters[uiSubStrm] );
      m_pcBitCounter = &pcBitCounters[uiSubStrm];
      pppcRDSbacCoder->setBinCountingEnableFlag( true );
      m_pcBitCounter->resetBits();
      pppcRDSbacCoder->setBinsCoded( 0 );
      m_pcCuEncoder->encodeCU( pcCU );

      pppcRDSbacCoder->setBinCountingEnableFlag( false );
      if (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE && ( ( pcSlice->getSliceBits() + m_pcEntropyCoder->getNumberOfWrittenBits() ) ) > m_pcCfg->getSliceArgument()<<3)
      {
        pcSlice->setNextSlice( true );
        break;
      }
      if (m_pcCfg->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE && pcSlice->getEntropySliceCounter()+pppcRDSbacCoder->getBinsCoded() > m_pcCfg->getEntropySliceArgument()&&pcSlice->getSliceCurEndCUAddr()!=pcSlice->getEntropySliceCurEndCUAddr())
      {
        pcSlice->setNextEntropySlice( true );
        break;
      }
      if( m_pcCfg->getUseSBACRD() )
      {
         ppppcRDSbacCoders[uiSubStrm][0][CI_CURR_BEST]->load( m_pppcRDSbacCoder[0][CI_CURR_BEST] );
       
         //Store probabilties of second LCU in line into buffer
        if (pcSlice->getPPS()->getEntropyCodingSynchro() && uiCol == uiTileLCUX+pcSlice->getPPS()->getEntropyCodingSynchro())
        {
          m_pcBufferSbacCoders[uiTileCol].loadContexts(ppppcRDSbacCoders[uiSubStrm][0][CI_CURR_BEST]);
        }
      }
      if( (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()==0) && (rpcPic->getPicSym()->getNumColumnsMinus1()!=0) )
      {
         //Store probabilties for next tile
        if( (uiLin == (rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr() / uiWidthInLCUs )) && 
            (uiCol == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getRightEdgePosInCU()) )
        {
          m_pcBufferLowLatSbacCoders[uiTileCol].loadContexts(ppppcRDSbacCoders[uiSubStrm][0][CI_CURR_BEST]);
        }
      }
      
    }
    // other case: encodeCU is not called
    else
    {
      m_pcCuEncoder->compressCU( pcCU );
      m_pcCuEncoder->encodeCU( pcCU );
      if (m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE && ( ( pcSlice->getSliceBits()+ m_pcEntropyCoder->getNumberOfWrittenBits() ) ) > m_pcCfg->getSliceArgument()<<3)
      {
        pcSlice->setNextSlice( true );
        break;
      }
      if (m_pcCfg->getEntropySliceMode()==SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE && pcSlice->getEntropySliceCounter()+ m_pcEntropyCoder->getNumberOfWrittenBits()> m_pcCfg->getEntropySliceArgument()&&pcSlice->getSliceCurEndCUAddr()!=pcSlice->getEntropySliceCurEndCUAddr())
      {
        pcSlice->setNextEntropySlice( true );
        break;
      }
    }
    
    m_uiPicTotalBits += pcCU->getTotalBits();
    m_dPicRdCost     += pcCU->getTotalCost();
    m_uiPicDist      += pcCU->getTotalDistortion();
  }
  xRestoreWPparam( pcSlice );
}

/**
 \param  rpcPic        picture class
 \retval rpcBitstream  bitstream class
 */
Void TEncSlice::encodeSlice   ( TComPic*& rpcPic, TComOutputBitstream* pcBitstream, TComOutputBitstream* pcSubstreams )
{
  UInt       uiCUAddr;
  UInt       uiStartCUAddr;
  UInt       uiBoundingCUAddr;
  TComSlice* pcSlice = rpcPic->getSlice(getSliceIdx());

  uiStartCUAddr=pcSlice->getEntropySliceCurStartCUAddr();
  uiBoundingCUAddr=pcSlice->getEntropySliceCurEndCUAddr();
  // choose entropy coder
  {
    m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
    m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
  }
  
  m_pcCuEncoder->setBitCounter( NULL );
  m_pcBitCounter = NULL;
  // Appropriate substream bitstream is switched later.
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

  TEncTop* pcEncTop = (TEncTop*) m_pcCfg;
  TEncSbac* pcSbacCoders = pcEncTop->getSbacCoders(); //coder for each substream
  Int iNumSubstreams = pcSlice->getPPS()->getNumSubstreams();
  UInt uiBitsOriginallyInSubstreams = 0;
  {
    UInt uiTilesAcross = rpcPic->getPicSym()->getNumColumnsMinus1()+1;
    for (UInt ui = 0; ui < uiTilesAcross; ui++)
    {
      m_pcBufferSbacCoders[ui].load(m_pcSbacCoder); //init. state
    }
    
    for (Int iSubstrmIdx=0; iSubstrmIdx < iNumSubstreams; iSubstrmIdx++)
    {
      uiBitsOriginallyInSubstreams += pcSubstreams[iSubstrmIdx].getNumberOfWrittenBits();
    }

    for (UInt ui = 0; ui < uiTilesAcross; ui++)
    {
      m_pcBufferLowLatSbacCoders[ui].load(m_pcSbacCoder);  //init. state
    }
  }

  UInt uiWidthInLCUs  = rpcPic->getPicSym()->getFrameWidthInCU();
  UInt uiCol=0, uiLin=0, uiSubStrm=0;
  Int  iBreakDep      = 0;
  UInt uiTileCol      = 0;
  UInt uiTileStartLCU = 0;
  UInt uiTileLCUX     = 0;

  UInt uiEncCUOrder;
  uiCUAddr = rpcPic->getPicSym()->getCUOrderMap( uiStartCUAddr /rpcPic->getNumPartInCU());  /*for tiles, uiStartCUAddr is NOT the real raster scan address, it is actually
                                                                                              an encoding order index, so we need to convert the index (uiStartCUAddr)
                                                                                              into the real raster scan address (uiCUAddr) via the CUOrderMap*/
  for( uiEncCUOrder = uiStartCUAddr /rpcPic->getNumPartInCU();
       uiEncCUOrder < (uiBoundingCUAddr+rpcPic->getNumPartInCU()-1)/rpcPic->getNumPartInCU();
       uiCUAddr = rpcPic->getPicSym()->getCUOrderMap(++uiEncCUOrder) )
  {
    if( m_pcCfg->getUseSBACRD() )
    {
      iBreakDep = rpcPic->getPicSym()->getTileBoundaryIndependenceIdr();
      uiTileCol = rpcPic->getPicSym()->getTileIdxMap(uiCUAddr) % (rpcPic->getPicSym()->getNumColumnsMinus1()+1); // what column of tiles are we in?
      uiTileStartLCU = rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr();
      uiTileLCUX = uiTileStartLCU % uiWidthInLCUs;
      //UInt uiSliceStartLCU = pcSlice->getSliceCurStartCUAddr();
      uiCol     = uiCUAddr % uiWidthInLCUs;
      uiLin     = uiCUAddr / uiWidthInLCUs;
      if (iBreakDep && pcSlice->getPPS()->getEntropyCodingSynchro())
      {
        // independent tiles => substreams are "per tile".  iNumSubstreams has already been multiplied.
        Int iNumSubstreamsPerTile = iNumSubstreams/rpcPic->getPicSym()->getNumTiles();
        uiSubStrm = rpcPic->getPicSym()->getTileIdxMap(uiCUAddr)*iNumSubstreamsPerTile
                      + uiLin%iNumSubstreamsPerTile;
      }
      else
      {
        // dependent tiles => substreams are "per frame".
        uiSubStrm = uiLin % iNumSubstreams;
      }

      m_pcEntropyCoder->setBitstream( &pcSubstreams[uiSubStrm] );

      // Synchronize cabac probabilities with upper-right LCU if it's available and we're at the start of a line.
      if (pcSlice->getPPS()->getEntropyCodingSynchro() && (uiCol == uiTileLCUX))
      {
        // We'll sync if the TR is available.
        TComDataCU *pcCUUp = rpcPic->getCU( uiCUAddr )->getCUAbove();
        UInt uiWidthInCU = rpcPic->getFrameWidthInCU();
        UInt uiMaxParts = 1<<(pcSlice->getSPS()->getMaxCUDepth()<<1);
        TComDataCU *pcCUTR = NULL;
        if ( pcCUUp && ((uiCUAddr%uiWidthInCU+pcSlice->getPPS()->getEntropyCodingSynchro()) < uiWidthInCU)  )
        {
          pcCUTR = rpcPic->getCU( uiCUAddr - uiWidthInCU + pcSlice->getPPS()->getEntropyCodingSynchro() );
        }
        if ( (true/*bEnforceSliceRestriction*/ &&
             ((pcCUTR==NULL) || (pcCUTR->getSlice()==NULL) || 
             (pcCUTR->getSCUAddr()+uiMaxParts-1 < pcSlice->getSliceCurStartCUAddr()) ||
             (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && (rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(uiCUAddr)))
             ))||
             (true/*bEnforceEntropySliceRestriction*/ &&
             ((pcCUTR==NULL) || (pcCUTR->getSlice()==NULL) || 
             (pcCUTR->getSCUAddr()+uiMaxParts-1 < pcSlice->getEntropySliceCurStartCUAddr()) ||
             (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && (rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(uiCUAddr)))
             ))
           )
        {
          // TR not available.
        }
        else
        {
          // TR is available, we use it.
          pcSbacCoders[uiSubStrm].loadContexts( &m_pcBufferSbacCoders[uiTileCol] );
        }
      }
      m_pcSbacCoder->load(&pcSbacCoders[uiSubStrm]);  //this load is used to simplify the code (avoid to change all the call to m_pcSbacCoder)
    }
    // reset the entropy coder
    if( uiCUAddr == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr() &&                                   // must be first CU of tile
        uiCUAddr!=0 &&                                                                                                                                    // cannot be first CU of picture
        uiCUAddr!=rpcPic->getPicSym()->getPicSCUAddr(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr())/rpcPic->getNumPartInCU())     // cannot be first CU of slice
    {
      Int iTileIdx            = rpcPic->getPicSym()->getTileIdxMap(uiCUAddr);
      Bool bWriteTileMarker   = false;
      // check if current iTileIdx should have a marker
      for (Int iEntryIdx=0; iEntryIdx<m_pcCfg->getMaxTileMarkerEntryPoints()-1; iEntryIdx++)
      {
        bWriteTileMarker = ( (((Int)((iEntryIdx+1)*m_pcCfg->getMaxTileMarkerOffset()+0.5)) == iTileIdx ) && iEntryIdx < (m_pcCfg->getMaxTileMarkerEntryPoints()-1)) ? true : false;
        if (bWriteTileMarker)
        {
          break;
        }
      }
      {
        // We're crossing into another tile, tiles are independent.
        // When tiles are independent, we have "substreams per tile".  Each substream has already been terminated, and we no longer
        // have to perform it here.
        if (pcSlice->getPPS()->getEntropyCodingSynchro())
        {
          ; // do nothing.
        }
        else
        {
          m_pcEntropyCoder->updateContextTables( pcSlice->getSliceType(), pcSlice->getSliceQp() );
          pcSubstreams[uiSubStrm].write( 1, 1 );
          pcSubstreams[uiSubStrm].writeAlignZero();
        }
      }
      {
        // Write TileMarker into the appropriate substream (nothing has been written to it yet).
        if (m_pcCfg->getTileMarkerFlag() && bWriteTileMarker)
        {
          // Log locations where tile markers are to be inserted during emulation prevention
          UInt uiMarkerCount = pcSubstreams[uiSubStrm].getTileMarkerLocationCount();
          pcSubstreams[uiSubStrm].setTileMarkerLocation     ( uiMarkerCount, pcSubstreams[uiSubStrm].getNumberOfWrittenBits() >> 3 );
          pcSubstreams[uiSubStrm].setTileMarkerLocationCount( uiMarkerCount + 1 );
          // Write tile index
          m_pcEntropyCoder->writeTileMarker(iTileIdx, rpcPic->getPicSym()->getBitsUsedByTileIdx()); // Tile index
        }

        
        UInt uiAccumulatedSubstreamLength = 0;
        for (Int iSubstrmIdx=0; iSubstrmIdx < iNumSubstreams; iSubstrmIdx++)
        {
          uiAccumulatedSubstreamLength += pcSubstreams[iSubstrmIdx].getNumberOfWrittenBits();
        }
        UInt uiLocationCount = pcSlice->getTileLocationCount();
        // add bits coded in previous entropy slices + bits coded so far
        pcSlice->setTileLocation( uiLocationCount, (pcSlice->getTileOffstForMultES() + uiAccumulatedSubstreamLength - uiBitsOriginallyInSubstreams) >> 3 ); 
        pcSlice->setTileLocationCount( uiLocationCount + 1 );
      }
    }

    TComDataCU*& pcCU = rpcPic->getCU( uiCUAddr );    
    if( (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()==0) && (rpcPic->getPicSym()->getNumColumnsMinus1()!=0) )
    {    
      // Synchronize cabac probabilities with LCU among Tiles
      if( (uiTileLCUX != 0) &&
          (uiCUAddr == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr()) )
      {
        TComDataCU *pcCULeft = pcCU->getCULeft();
        UInt uiMaxParts = 1<<(pcSlice->getSPS()->getMaxCUDepth()<<1);

        if ( (true/*bEnforceSliceRestriction*/ &&
              ((pcCULeft==NULL) || (pcCULeft->getSlice()==NULL) || 
               ((pcCULeft->getSCUAddr()+uiMaxParts-1) < pcSlice->getSliceCurStartCUAddr()) 
              )
             )||
             (true/*bEnforceEntropySliceRestriction*/ &&
              ((pcCULeft==NULL) || (pcCULeft->getSlice()==NULL) || 
               ((pcCULeft->getSCUAddr()+uiMaxParts-1) < pcSlice->getEntropySliceCurStartCUAddr())
              )
             )
           )
        {
          // Left not available.
        }
        else
        {
          // Left is available, we use it.
          pcSbacCoders[uiSubStrm].loadContexts( &m_pcBufferLowLatSbacCoders[uiTileCol-1] );
          m_pcSbacCoder->loadContexts(&pcSbacCoders[uiSubStrm]);  //this load is used to simplify the code (avoid to change all the call to m_pcSbacCoder)
        }
      }
    }
#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceEnable;
#endif
    if ( (m_pcCfg->getSliceMode()!=0 || m_pcCfg->getEntropySliceMode()!=0) && 
      uiCUAddr == rpcPic->getPicSym()->getCUOrderMap((uiBoundingCUAddr+rpcPic->getNumPartInCU()-1)/rpcPic->getNumPartInCU()-1) )
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
    if( m_pcCfg->getUseSBACRD() )
    {
       pcSbacCoders[uiSubStrm].load(m_pcSbacCoder);   //load back status of the entropy coder after encoding the LCU into relevant bitstream entropy coder
       

       //Store probabilties of second LCU in line into buffer
      if (pcSlice->getPPS()->getEntropyCodingSynchro() && (uiCol == uiTileLCUX+pcSlice->getPPS()->getEntropyCodingSynchro()))
      {
        m_pcBufferSbacCoders[uiTileCol].loadContexts( &pcSbacCoders[uiSubStrm] );
      }
    }
    if( (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()==0) && (rpcPic->getPicSym()->getNumColumnsMinus1()!=0) )
    {
      pcSbacCoders[uiSubStrm].load(m_pcSbacCoder);   //load back status of the entropy coder after encoding the LCU into relevant bitstream entropy coder
       //Store probabilties for next tile
      if( (uiLin == (rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getFirstCUAddr() / uiWidthInLCUs )) && 
          (uiCol == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(uiCUAddr))->getRightEdgePosInCU()) )
      {
        m_pcBufferLowLatSbacCoders[uiTileCol].loadContexts( &pcSbacCoders[uiSubStrm] );
      }
    }
  }

#if ADAPTIVE_QP_SELECTION
  if( m_pcCfg->getUseAdaptQpSelect() )
  {
    m_pcTrQuant->storeSliceQpNext(pcSlice);
  }
#endif
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
      uiBoundingCUAddrSlice    = ((uiStartCUAddrSlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU()) ? (uiStartCUAddrSlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
      break;
    case AD_HOC_SLICES_FIXED_NUMBER_OF_BYTES_IN_SLICE:
      uiCUAddrIncrement        = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrSlice    = pcSlice->getSliceCurEndCUAddr();
      break;
    default:
      uiCUAddrIncrement        = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrSlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
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
      uiBoundingCUAddrSlice    = ((uiStartCUAddrSlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU()) ? (uiStartCUAddrSlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
      break;
    default:
      uiCUAddrIncrement        = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrSlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
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
      uiBoundingCUAddrEntropySlice    = ((uiStartCUAddrEntropySlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU() ) ? (uiStartCUAddrEntropySlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
      break;
    case SHARP_MULTIPLE_CONSTRAINT_BASED_ENTROPY_SLICE:
      uiCUAddrIncrement               = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrEntropySlice    = pcSlice->getEntropySliceCurEndCUAddr();
      break;
    default:
      uiCUAddrIncrement               = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrEntropySlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
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
      uiBoundingCUAddrEntropySlice    = ((uiStartCUAddrEntropySlice + uiCUAddrIncrement) < uiNumberOfCUsInFrame*rpcPic->getNumPartInCU() ) ? (uiStartCUAddrEntropySlice + uiCUAddrIncrement) : uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
      break;
    default:
      uiCUAddrIncrement               = rpcPic->getNumCUsInFrame();
      uiBoundingCUAddrEntropySlice    = uiNumberOfCUsInFrame*rpcPic->getNumPartInCU();
      break;
    } 
    pcSlice->setEntropySliceCurEndCUAddr( uiBoundingCUAddrEntropySlice );
  }
  if(uiBoundingCUAddrEntropySlice>uiBoundingCUAddrSlice)
  {
    uiBoundingCUAddrEntropySlice = uiBoundingCUAddrSlice;
    pcSlice->setEntropySliceCurEndCUAddr(uiBoundingCUAddrSlice);
  }
  //calculate real entropy slice start address
  UInt uiInternalAddress = rpcPic->getPicSym()->getPicSCUAddr(pcSlice->getEntropySliceCurStartCUAddr()) % rpcPic->getNumPartInCU();
  UInt uiExternalAddress = rpcPic->getPicSym()->getPicSCUAddr(pcSlice->getEntropySliceCurStartCUAddr()) / rpcPic->getNumPartInCU();
  UInt uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  UInt uiWidth = pcSlice->getSPS()->getWidth();
  UInt uiHeight = pcSlice->getSPS()->getHeight();
  while((uiPosX>=uiWidth||uiPosY>=uiHeight)&&!(uiPosX>=uiWidth&&uiPosY>=uiHeight))
  {
    uiInternalAddress++;
    if(uiInternalAddress>=rpcPic->getNumPartInCU())
    {
      uiInternalAddress=0;
      uiExternalAddress = rpcPic->getPicSym()->getCUOrderMap(rpcPic->getPicSym()->getInverseCUOrderMap(uiExternalAddress)+1);
    }
    uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
    uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  }
  UInt uiRealStartAddress = rpcPic->getPicSym()->getPicSCUEncOrder(uiExternalAddress*rpcPic->getNumPartInCU()+uiInternalAddress);
  
  pcSlice->setEntropySliceCurStartCUAddr(uiRealStartAddress);
  uiStartCUAddrEntropySlice=uiRealStartAddress;
  
  //calculate real slice start address
  uiInternalAddress = rpcPic->getPicSym()->getPicSCUAddr(pcSlice->getSliceCurStartCUAddr()) % rpcPic->getNumPartInCU();
  uiExternalAddress = rpcPic->getPicSym()->getPicSCUAddr(pcSlice->getSliceCurStartCUAddr()) / rpcPic->getNumPartInCU();
  uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
  uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  uiWidth = pcSlice->getSPS()->getWidth();
  uiHeight = pcSlice->getSPS()->getHeight();
  while((uiPosX>=uiWidth||uiPosY>=uiHeight)&&!(uiPosX>=uiWidth&&uiPosY>=uiHeight))
  {
    uiInternalAddress++;
    if(uiInternalAddress>=rpcPic->getNumPartInCU())
    {
      uiInternalAddress=0;
      uiExternalAddress = rpcPic->getPicSym()->getCUOrderMap(rpcPic->getPicSym()->getInverseCUOrderMap(uiExternalAddress)+1);
    }
    uiPosX = ( uiExternalAddress % rpcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
    uiPosY = ( uiExternalAddress / rpcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
  }
  uiRealStartAddress = rpcPic->getPicSym()->getPicSCUEncOrder(uiExternalAddress*rpcPic->getNumPartInCU()+uiInternalAddress);
  
  pcSlice->setSliceCurStartCUAddr(uiRealStartAddress);
  uiStartCUAddrSlice=uiRealStartAddress;
  
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
