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

/** \file     TEncTop.cpp
    \brief    encoder class
*/

#include "TLibCommon/CommonDef.h"
#include "TEncTop.h"
#include "TEncPic.h"
#if FAST_BIT_EST
#include "TLibCommon/ContextModel.h"
#endif

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncTop::TEncTop()
{
  m_iPOCLast          = -1;
  m_iNumPicRcvd       =  0;
  m_uiNumAllPicCoded  =  0;
  m_pppcRDSbacCoder   =  NULL;
  m_pppcBinCoderCABAC =  NULL;
  m_cRDGoOnSbacCoder.init( &m_cRDGoOnBinCoderCABAC );
#if ENC_DEC_TRACE
  g_hTrace = fopen( "TraceEnc.txt", "wb" );
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif

  m_iMaxRefPicNum     = 0;

#if FAST_BIT_EST
  ContextModel::buildNextStateTable();
#endif

  m_pcSbacCoders           = NULL;
  m_pcBinCoderCABACs       = NULL;
  m_ppppcRDSbacCoders      = NULL;
  m_ppppcBinCodersCABAC    = NULL;
  m_pcRDGoOnSbacCoders     = NULL;
  m_pcRDGoOnBinCodersCABAC = NULL;
  m_pcBitCounters          = NULL;
  m_pcRdCosts              = NULL;
}

TEncTop::~TEncTop()
{
#if ENC_DEC_TRACE
  fclose( g_hTrace );
#endif
}

Void TEncTop::create ()
{
  // initialize global variables
  initROM();
  
  // create processing unit classes
  m_cGOPEncoder.        create( getSourceWidth(), getSourceHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight );
  m_cSliceEncoder.      create( getSourceWidth(), getSourceHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  m_cCuEncoder.         create( g_uiMaxCUDepth, g_uiMaxCUWidth, g_uiMaxCUHeight );
  if (m_bUseSAO)
  {
    m_cEncSAO.create( getSourceWidth(), getSourceHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
    m_cEncSAO.createEncBuffer();
  }
#if ADAPTIVE_QP_SELECTION
  if (m_bUseAdaptQpSelect)
  {
    m_cTrQuant.initSliceQpDelta();
  }
#endif
  m_cAdaptiveLoopFilter.create( getSourceWidth(), getSourceHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
  m_cLoopFilter.        create( g_uiMaxCUDepth );
  
  if(m_bUseALF)
  {
    m_cAdaptiveLoopFilter.setGOPSize( getGOPSize() );
    m_cAdaptiveLoopFilter.createAlfGlobalBuffers(m_iALFEncodePassReduction);
  }

  if(m_bUseSAO || m_bUseALF)
  {
    m_vAPS.reserve(MAX_NUM_SUPPORTED_APS);
  }

  // if SBAC-based RD optimization is used
  if( m_bUseSBACRD )
  {
    m_pppcRDSbacCoder = new TEncSbac** [g_uiMaxCUDepth+1];
#if FAST_BIT_EST
    m_pppcBinCoderCABAC = new TEncBinCABACCounter** [g_uiMaxCUDepth+1];
#else
    m_pppcBinCoderCABAC = new TEncBinCABAC** [g_uiMaxCUDepth+1];
#endif
    
    for ( Int iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
    {
      m_pppcRDSbacCoder[iDepth] = new TEncSbac* [CI_NUM];
#if FAST_BIT_EST
      m_pppcBinCoderCABAC[iDepth] = new TEncBinCABACCounter* [CI_NUM];
#else
      m_pppcBinCoderCABAC[iDepth] = new TEncBinCABAC* [CI_NUM];
#endif
      
      for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
      {
        m_pppcRDSbacCoder[iDepth][iCIIdx] = new TEncSbac;
#if FAST_BIT_EST
        m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABACCounter;
#else
        m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABAC;
#endif
        m_pppcRDSbacCoder   [iDepth][iCIIdx]->init( m_pppcBinCoderCABAC [iDepth][iCIIdx] );
      }
    }
  }
}

/**
 - Allocate coders required for wavefront for the nominated number of substreams.
 .
 \param iNumSubstreams Determines how much information to allocate.
 */
Void TEncTop::createWPPCoders(Int iNumSubstreams)
{
  if (m_pcSbacCoders != NULL)
    return; // already generated.

  m_iNumSubstreams         = iNumSubstreams;
  m_pcSbacCoders           = new TEncSbac       [iNumSubstreams];
  m_pcBinCoderCABACs       = new TEncBinCABAC   [iNumSubstreams];
  m_pcRDGoOnSbacCoders     = new TEncSbac       [iNumSubstreams];
  m_pcRDGoOnBinCodersCABAC = new TEncBinCABAC   [iNumSubstreams];
  m_pcBitCounters          = new TComBitCounter [iNumSubstreams];
  m_pcRdCosts              = new TComRdCost     [iNumSubstreams];

  for ( UInt ui = 0 ; ui < iNumSubstreams; ui++ )
  {
    m_pcRDGoOnSbacCoders[ui].init( &m_pcRDGoOnBinCodersCABAC[ui] );
    m_pcSbacCoders[ui].init( &m_pcBinCoderCABACs[ui] );
  }
  if( m_bUseSBACRD )
  {
    m_ppppcRDSbacCoders      = new TEncSbac***    [iNumSubstreams];
    m_ppppcBinCodersCABAC    = new TEncBinCABAC***[iNumSubstreams];
    for ( UInt ui = 0 ; ui < iNumSubstreams ; ui++ )
    {
      m_ppppcRDSbacCoders[ui]  = new TEncSbac** [g_uiMaxCUDepth+1];
      m_ppppcBinCodersCABAC[ui]= new TEncBinCABAC** [g_uiMaxCUDepth+1];
      
      for ( Int iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
      {
        m_ppppcRDSbacCoders[ui][iDepth]  = new TEncSbac*     [CI_NUM];
        m_ppppcBinCodersCABAC[ui][iDepth]= new TEncBinCABAC* [CI_NUM];

        for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
        {
          m_ppppcRDSbacCoders  [ui][iDepth][iCIIdx] = new TEncSbac;
          m_ppppcBinCodersCABAC[ui][iDepth][iCIIdx] = new TEncBinCABAC;
          m_ppppcRDSbacCoders  [ui][iDepth][iCIIdx]->init( m_ppppcBinCodersCABAC[ui][iDepth][iCIIdx] );
        }
      }
    }
  }
}

Void TEncTop::destroy ()
{
  if(m_bUseALF)
  {
    m_cAdaptiveLoopFilter.destroyAlfGlobalBuffers();
  }

  for(Int i=0; i< m_vAPS.size(); i++)
  {
    TComAPS& cAPS = m_vAPS[i];
    m_cGOPEncoder.freeAPS(&cAPS, &m_cSPS);
  }

  // destroy processing unit classes
  m_cGOPEncoder.        destroy();
  m_cSliceEncoder.      destroy();
  m_cCuEncoder.         destroy();
  if (m_cSPS.getUseSAO())
  {
    m_cEncSAO.destroy();
    m_cEncSAO.destroyEncBuffer();
  }
  m_cAdaptiveLoopFilter.destroy();
  m_cLoopFilter.        destroy();
  m_cRPSList.               destroy();
  
  // SBAC RD
  if( m_bUseSBACRD )
  {
    Int iDepth;
    for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
    {
      for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
      {
        delete m_pppcRDSbacCoder[iDepth][iCIIdx];
        delete m_pppcBinCoderCABAC[iDepth][iCIIdx];
      }
    }
    
    for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
    {
      delete [] m_pppcRDSbacCoder[iDepth];
      delete [] m_pppcBinCoderCABAC[iDepth];
    }
    
    delete [] m_pppcRDSbacCoder;
    delete [] m_pppcBinCoderCABAC;

    for ( UInt ui = 0; ui < m_iNumSubstreams; ui++ )
    {
      for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
      {
        for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
        {
          delete m_ppppcRDSbacCoders  [ui][iDepth][iCIIdx];
          delete m_ppppcBinCodersCABAC[ui][iDepth][iCIIdx];
        }
      }

      for ( iDepth = 0; iDepth < g_uiMaxCUDepth+1; iDepth++ )
      {
        delete [] m_ppppcRDSbacCoders  [ui][iDepth];
        delete [] m_ppppcBinCodersCABAC[ui][iDepth];
      }
      delete[] m_ppppcRDSbacCoders  [ui];
      delete[] m_ppppcBinCodersCABAC[ui];
    }
    delete[] m_ppppcRDSbacCoders;
    delete[] m_ppppcBinCodersCABAC;
  }
  delete[] m_pcSbacCoders;
  delete[] m_pcBinCoderCABACs;
  delete[] m_pcRDGoOnSbacCoders;  
  delete[] m_pcRDGoOnBinCodersCABAC;
  delete[] m_pcBitCounters;
  delete[] m_pcRdCosts;
  
  // destroy ROM
  destroyROM();
  
  return;
}

Void TEncTop::init()
{
  UInt *aTable4=NULL, *aTable8=NULL;
  UInt* aTableLastPosVlcIndex=NULL; 
  // initialize SPS
  xInitSPS();
  
  // initialize PPS
  m_cPPS.setSPS(&m_cSPS);
  m_cPPS.setRPSList(&m_cRPSList);
  xInitPPS();
  xInitRPS();

  xInitPPSforTiles();

  // initialize processing unit classes
  m_cGOPEncoder.  init( this );
  m_cSliceEncoder.init( this );
  m_cCuEncoder.   init( this );
  
  // initialize transform & quantization class
  m_pcCavlcCoder = getCavlcCoder();
  
  m_cTrQuant.init( g_uiMaxCUWidth, g_uiMaxCUHeight, 1 << m_uiQuadtreeTULog2MaxSize,
                  0,
                  aTable4, aTable8, 
                  aTableLastPosVlcIndex, m_bUseRDOQ, true 
#if ADAPTIVE_QP_SELECTION                  
                  , m_bUseAdaptQpSelect
#endif
                  );
  
  // initialize encoder search class
  m_cSearch.init( this, &m_cTrQuant, m_iSearchRange, m_bipredSearchRange, m_iFastSearch, 0, &m_cEntropyCoder, &m_cRdCost, getRDSbacCoder(), getRDGoOnSbacCoder() );

  if(m_bUseALF)
  {
    m_cAdaptiveLoopFilter.setALFEncodePassReduction( m_iALFEncodePassReduction );
    m_cAdaptiveLoopFilter.setALFMaxNumberFilters( m_iALFMaxNumberFilters );
  }

  m_iMaxRefPicNum = 0;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncTop::deletePicBuffer()
{
  TComList<TComPic*>::iterator iterPic = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );
  
  for ( Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);
    
    pcPic->destroy();
    delete pcPic;
    pcPic = NULL;
  }
}

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \param   bEos                true if end-of-sequence is reached
 \param   pcPicYuvOrg         original YUV picture
 \retval  rcListPicYuvRecOut  list of reconstruction YUV pictures
 \retval  rcListBitstreamOut  list of output bitstreams
 \retval  iNumEncoded         number of encoded pictures
 */
Void TEncTop::encode( bool bEos, TComPicYuv* pcPicYuvOrg, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded )
{
  TComPic* pcPicCurr = NULL;
  
  // get original YUV
  xGetNewPicBuffer( pcPicCurr );
  pcPicYuvOrg->copyToPic( pcPicCurr->getPicYuvOrg() );
  
  // compute image characteristics
  if ( getUseAdaptiveQP() )
  {
    m_cPreanalyzer.xPreanalyze( dynamic_cast<TEncPic*>( pcPicCurr ) );
  }
  
  if ( m_iPOCLast != 0 && ( m_iNumPicRcvd != m_iGOPSize && m_iGOPSize ) && !bEos )
  {
    iNumEncoded = 0;
    return;
  }
  
  // compress GOP
  m_cGOPEncoder.compressGOP(m_iPOCLast, m_iNumPicRcvd, m_cListPic, rcListPicYuvRecOut, accessUnitsOut);
  
  iNumEncoded         = m_iNumPicRcvd;
  m_iNumPicRcvd       = 0;
  m_uiNumAllPicCoded += iNumEncoded;
  
  if (bEos)
  {
    m_cGOPEncoder.printOutSummary (m_uiNumAllPicCoded);
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

/**
 - Application has picture buffer list with size of GOP + 1
 - Picture buffer list acts like as ring buffer
 - End of the list has the latest picture
 .
 \retval rpcPic obtained picture buffer
 */
Void TEncTop::xGetNewPicBuffer ( TComPic*& rpcPic )
{
  TComSlice::sortPicList(m_cListPic);
  
  if (m_cListPic.size() >= (UInt)(m_iGOPSize + getMaxNumberOfReferencePictures() + 2) )
  {
    TComList<TComPic*>::iterator iterPic  = m_cListPic.begin();
    Int iSize = Int( m_cListPic.size() );
    for ( Int i = 0; i < iSize; i++ )
    {
      rpcPic = *(++iterPic);
      if(rpcPic->getSlice(0)->isReferenced() == false)
         break;
    }
  }
  else
  {
        if ( getUseAdaptiveQP() )
        {
          TEncPic* pcEPic = new TEncPic;
          pcEPic->create( m_iSourceWidth, m_iSourceHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, m_cPPS.getMaxCuDQPDepth()+1 );
          rpcPic = pcEPic;
        }
        else
        {
          rpcPic = new TComPic;
          rpcPic->create( m_iSourceWidth, m_iSourceHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
        }
    m_cListPic.pushBack( rpcPic );
  }
  rpcPic->setReconMark (false);
  
  m_iPOCLast++;
  m_iNumPicRcvd++;
  
  rpcPic->getSlice(0)->setPOC( m_iPOCLast );
  // mark it should be extended
  rpcPic->getPicYuvRec()->setBorderExtension(false);
}

Void TEncTop::xInitSPS()
{
  m_cSPS.setWidth         ( m_iSourceWidth      );
  m_cSPS.setHeight        ( m_iSourceHeight     );
  m_cSPS.setPad           ( m_aiPad             );
  m_cSPS.setMaxCUWidth    ( g_uiMaxCUWidth      );
  m_cSPS.setMaxCUHeight   ( g_uiMaxCUHeight     );
  m_cSPS.setMaxCUDepth    ( g_uiMaxCUDepth      );
  m_cSPS.setMinTrDepth    ( 0                   );
  m_cSPS.setMaxTrDepth    ( 1                   );
  
  m_cSPS.setMaxNumberOfReferencePictures(m_uiMaxNumberOfReferencePictures);
  m_cSPS.setNumReorderFrames(m_numReorderFrames);
  m_cSPS.setPCMLog2MinSize (m_uiPCMLog2MinSize);
  m_cSPS.setUsePCM        ( m_usePCM           );
  m_cSPS.setPCMLog2MaxSize( m_pcmLog2MaxSize  );

  m_cSPS.setUseALF        ( m_bUseALF           );
  
  m_cSPS.setQuadtreeTULog2MaxSize( m_uiQuadtreeTULog2MaxSize );
  m_cSPS.setQuadtreeTULog2MinSize( m_uiQuadtreeTULog2MinSize );
  m_cSPS.setQuadtreeTUMaxDepthInter( m_uiQuadtreeTUMaxDepthInter    );
  m_cSPS.setQuadtreeTUMaxDepthIntra( m_uiQuadtreeTUMaxDepthIntra    );
  
  m_cSPS.setUsePAD        ( m_bUsePAD           );
  
  m_cSPS.setUseMRG        ( m_bUseMRG           ); // SOPH:

  m_cSPS.setUseLMChroma   ( m_bUseLMChroma           );  
  
  m_cSPS.setMaxTrSize   ( 1 << m_uiQuadtreeTULog2MaxSize );
  
  m_cSPS.setUseLComb    ( m_bUseLComb           );
  m_cSPS.setLCMod       ( m_bLCMod   );
  m_cSPS.setUseNSQT( m_useNSQT );
  
  Int i;
#if HHI_AMVP_OFF
  for ( i = 0; i < g_uiMaxCUDepth; i++ )
  {
    m_cSPS.setAMVPMode( i, AM_NONE );
  }
#else
  for ( i = 0; i < g_uiMaxCUDepth; i++ )
  {
    m_cSPS.setAMVPMode( i, AM_EXPL );
  }
#endif
  
  for (i = 0; i < g_uiMaxCUDepth-1; i++ )
  {
    m_cSPS.setAMPAcc( i, m_useAMP );
    //m_cSPS.setAMPAcc( i, 1 );
  }

  m_cSPS.setUseAMP ( m_useAMP );

  for (i = g_uiMaxCUDepth-1; i < g_uiMaxCUDepth; i++ )
  {
    m_cSPS.setAMPAcc(i, 0);
  }

  m_cSPS.setBitDepth    ( g_uiBitDepth        );
  m_cSPS.setBitIncrement( g_uiBitIncrement    );

  m_cSPS.setLFCrossSliceBoundaryFlag( m_bLFCrossSliceBoundaryFlag );
  m_cSPS.setUseSAO( m_bUseSAO );

  if ( m_bTLayering )
  {
    Int iMaxTLayers = 1;
    for ( i = 1; ; i++)
    {
      iMaxTLayers = i;
      if ( (m_iGOPSize >> i) == 0 ) 
      {
        break;
      }
    }
  
    m_cSPS.setMaxTLayers( (UInt)iMaxTLayers );

    Bool bTemporalIdNestingFlag = true;
    for ( i = 0; i < m_cSPS.getMaxTLayers()-1; i++ )
    {
      if ( !m_abTLayerSwitchingFlag[i] )
      {
        bTemporalIdNestingFlag = false;
        break;
      }
    }

    m_cSPS.setTemporalIdNestingFlag( bTemporalIdNestingFlag );
  }
  else
  {
    m_cSPS.setMaxTLayers( 1 );
    m_cSPS.setTemporalIdNestingFlag( false );
  }
  m_cSPS.setPCMBitDepthLuma (g_uiPCMBitDepthLuma);
  m_cSPS.setPCMBitDepthChroma (g_uiPCMBitDepthChroma);
  m_cSPS.setPCMFilterDisableFlag  ( m_bPCMFilterDisableFlag );

  m_cSPS.setLFCrossTileBoundaryFlag( m_bLFCrossTileBoundaryFlag );
  m_cSPS.setUniformSpacingIdr( m_iUniformSpacingIdr );
  m_cSPS.setTileBoundaryIndependenceIdr( m_iTileBoundaryIndependenceIdr );
  m_cSPS.setNumColumnsMinus1( m_iNumColumnsMinus1 );
  m_cSPS.setNumRowsMinus1( m_iNumRowsMinus1 );
  if( m_iUniformSpacingIdr == 0 )
  {
    m_cSPS.setColumnWidth( m_puiColumnWidth );
    m_cSPS.setRowHeight( m_puiRowHeight );
  }
  m_cSPS.setScalingListFlag ( (m_useScalingListId == 0) ? 0 : 1 );
  m_cSPS.setUseDF( m_loopFilterOffsetInAPS );
}

Void TEncTop::xInitPPS()
{
  m_cPPS.setConstrainedIntraPred( m_bUseConstrainedIntraPred );
  m_cPPS.setSliceGranularity(m_iSliceGranularity);
  if ( m_cSPS.getTemporalIdNestingFlag() ) 
  {
    m_cPPS.setNumTLayerSwitchingFlags( 0 );
    for ( UInt i = 0; i < m_cSPS.getMaxTLayers() - 1; i++ )
    {
      m_cPPS.setTLayerSwitchingFlag( i, true );
    }
  }
  else
  {
    m_cPPS.setNumTLayerSwitchingFlags( m_cSPS.getMaxTLayers() - 1 );
    for ( UInt i = 0; i < m_cPPS.getNumTLayerSwitchingFlags(); i++ )
    {
      m_cPPS.setTLayerSwitchingFlag( i, m_abTLayerSwitchingFlag[i] );
    }
  }   
  Int max_temporal_layers = m_cPPS.getSPS()->getMaxTLayers();
  if(max_temporal_layers > 4)
     m_cPPS.setBitsForTemporalId(3);
  else if(max_temporal_layers > 2)
     m_cPPS.setBitsForTemporalId(2);
  else if(max_temporal_layers > 1)
     m_cPPS.setBitsForTemporalId(1);
  else
     m_cPPS.setBitsForTemporalId(0);

  Bool bUseDQP = (getMaxCuDQPDepth() > 0)? true : false;

  if(bUseDQP == false)
  {
    if((getMaxDeltaQP() != 0 )|| getUseAdaptiveQP())
    {
      bUseDQP = true;
    }
  }

  if(bUseDQP)
  {
    m_cPPS.setUseDQP(true);
    m_cPPS.setMaxCuDQPDepth( m_iMaxCuDQPDepth );
    m_cPPS.setMinCuDQPSize( m_cPPS.getSPS()->getMaxCUWidth() >> ( m_cPPS.getMaxCuDQPDepth()) );
  }
  else
  {
    m_cPPS.setUseDQP(false);
    m_cPPS.setMaxCuDQPDepth( 0 );
    m_cPPS.setMinCuDQPSize( m_cPPS.getSPS()->getMaxCUWidth() >> ( m_cPPS.getMaxCuDQPDepth()) );
  }

  m_cPPS.setChromaQpOffset   ( m_iChromaQpOffset    );
  m_cPPS.setChromaQpOffset2nd( m_iChromaQpOffset2nd );

  m_cPPS.setEntropyCodingMode( 1 ); // In the PPS now, but also remains in slice header!
  m_cPPS.setEntropyCodingSynchro(m_iWaveFrontSynchro);
  m_cPPS.setCabacIstateReset(m_iWaveFrontFlush != 0);
  m_cPPS.setNumSubstreams(m_iWaveFrontSubstreams);
  m_cPPS.setUseWP( m_bUseWeightPred );
  m_cPPS.setWPBiPredIdc( m_uiBiPredIdc );
  m_cPPS.setEnableTMVPFlag( m_bEnableTMVP );
}

Void TEncTop::xInitRPS()
{
  TComReferencePictureSet*      pcRPS;
  // In this function 
  // a number of Reference Picture Sets
  // are defined for different coding structures.
  // This is the place 
  // where you need to do changes in
  // order to try different Reference Picture Sets.
  // In a future implementation the 
  // Reference Picture Sets will be 
  // configured directly from the config file.
  // Here we check what BD is appropriate
  
  m_cRPSList.create(getGOPSize()+m_iExtraRPSs);
  for( Int i = 0; i < getGOPSize()+m_iExtraRPSs; i++) 
  {
    GOPEntry pGE = getGOPEntry(i);
    pcRPS = m_cRPSList.getReferencePictureSet(i);
    pcRPS->setNumberOfPictures(pGE.m_iNumRefPics);
    pcRPS->setNumRefIdc(pGE.m_iNumRefIdc);
    int iNumNeg = 0;
    int iNumPos = 0;
    for( Int j = 0; j < pGE.m_iNumRefPics; j++)
    {
      pcRPS->setDeltaPOC(j,pGE.m_aiReferencePics[j]);
      pcRPS->setUsed(j,pGE.m_aiUsedByCurrPic[j]);
      if(pGE.m_aiReferencePics[j]>0)
        iNumPos++;
      else
        iNumNeg++;
    }
    pcRPS->setNumberOfNegativePictures(iNumNeg);
    pcRPS->setNumberOfPositivePictures(iNumPos);
    pcRPS->setInterRPSPrediction(pGE.m_bInterRPSPrediction);
    if (pGE.m_bInterRPSPrediction)
    {
      pcRPS->setDeltaRIdxMinus1(pGE.m_iDeltaRIdxMinus1);
      pcRPS->setDeltaRPS(pGE.m_iDeltaRPS);
      pcRPS->setNumRefIdc(pGE.m_iNumRefIdc);
      for (Int j = 0; j < pGE.m_iNumRefIdc; j++ )
      {
        pcRPS->setRefIdc(j, pGE.m_aiRefIdc[j]);
      }
#if WRITE_BACK
      // the folowing code overwrite the deltaPOC and Used by current values read from the config file with the ones
      // computed from the RefIdc.  This is not necessary if both are identical. Currently there is no check to see if they are identical.
      iNumNeg = 0;
      iNumPos = 0;
      TComReferencePictureSet*     pcRPSRef = m_cRPSList.getReferencePictureSet(i-(pGE.m_iDeltaRIdxMinus1+1));
      for (Int j = 0; j < pGE.m_iNumRefIdc; j++ )
      {
        if (pGE.m_aiRefIdc[j])
        {
          int deltaPOC = pGE.m_iDeltaRPS + ((j < pcRPSRef->getNumberOfPictures())? pcRPSRef->getDeltaPOC(j) : 0);
          pcRPS->setDeltaPOC((iNumNeg+iNumPos),deltaPOC);
          pcRPS->setUsed((iNumNeg+iNumPos),pGE.m_aiRefIdc[j]==1?1:0);
          if (deltaPOC<0)
            iNumNeg++;
          else
            iNumPos++;
        }
      }
      pcRPS->setNumberOfNegativePictures(iNumNeg);
      pcRPS->setNumberOfPositivePictures(iNumPos);
      pcRPS->sortDeltaPOC();
#endif
    }
  }
  
}

Void TEncTop::selectReferencePictureSet(TComSlice* pcSlice, UInt uiPOCCurr, UInt iGOPid,TComList<TComPic*>& rcListPic )
{

   // This is a function that 
   // decides what Reference Picture Set to use 
   // for a specific picture (with POC = uiPOCCurr)

  pcSlice->setRPSidx(iGOPid);

  for(Int extraNum=m_iGOPSize; extraNum<m_iExtraRPSs+m_iGOPSize; extraNum++)
  {    
    if(m_uiIntraPeriod > 0)
    {
      if(uiPOCCurr%m_uiIntraPeriod==m_pcGOPList[extraNum].m_iPOC)
      {
        pcSlice->setRPSidx(extraNum);
      }
    }
    else
    {
      if(uiPOCCurr==m_pcGOPList[extraNum].m_iPOC)
      {
        pcSlice->setRPSidx(extraNum);
      }
    }
  }

  pcSlice->setRPS(getRPSList()->getReferencePictureSet(pcSlice->getRPSidx()));
  pcSlice->getRPS()->setNumberOfPictures(pcSlice->getRPS()->getNumberOfNegativePictures()+pcSlice->getRPS()->getNumberOfPositivePictures());

}

Void  TEncTop::xInitPPSforTiles()
{
    m_cPPS.setColumnRowInfoPresent( m_iColumnRowInfoPresent );
    m_cPPS.setUniformSpacingIdr( m_iUniformSpacingIdr );
    m_cPPS.setTileBoundaryIndependenceIdr( m_iTileBoundaryIndependenceIdr );
    m_cPPS.setNumColumnsMinus1( m_iNumColumnsMinus1 );
    m_cPPS.setNumRowsMinus1( m_iNumRowsMinus1 );
    if( m_iUniformSpacingIdr == 0 )
    {
      m_cPPS.setColumnWidth( m_puiColumnWidth );
      m_cPPS.setRowHeight( m_puiRowHeight );
    }
    m_cPPS.setTileBehaviorControlPresentFlag( m_iTileBehaviorControlPresentFlag );
    m_cPPS.setLFCrossTileBoundaryFlag( m_bLFCrossTileBoundaryFlag );

    // # substreams is "per tile" when tiles are independent.
    if (m_iTileBoundaryIndependenceIdr && m_iWaveFrontSynchro)
      m_cPPS.setNumSubstreams(m_iWaveFrontSubstreams * (m_iNumColumnsMinus1+1)*(m_iNumRowsMinus1+1));
}

Void  TEncCfg::xCheckGSParameters()
{
  Int   iWidthInCU = ( m_iSourceWidth%g_uiMaxCUWidth ) ? m_iSourceWidth/g_uiMaxCUWidth + 1 : m_iSourceWidth/g_uiMaxCUWidth;
  Int   iHeightInCU = ( m_iSourceHeight%g_uiMaxCUHeight ) ? m_iSourceHeight/g_uiMaxCUHeight + 1 : m_iSourceHeight/g_uiMaxCUHeight;
  UInt  uiCummulativeColumnWidth = 0;
  UInt  uiCummulativeRowHeight = 0;

  //check the column relative parameters
  if( m_iNumColumnsMinus1 >= (1<<(LOG2_MAX_NUM_COLUMNS_MINUS1+1)) )
  {
    printf( "The number of columns is larger than the maximum allowed number of columns.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumColumnsMinus1 >= iWidthInCU )
  {
    printf( "The current picture can not have so many columns.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumColumnsMinus1 && m_iUniformSpacingIdr==0 )
  {
    for(Int i=0; i<m_iNumColumnsMinus1; i++)
      uiCummulativeColumnWidth += m_puiColumnWidth[i];

    if( uiCummulativeColumnWidth >= iWidthInCU )
    {
      printf( "The width of the column is too large.\n" );
      exit( EXIT_FAILURE );
    }
  }

  //check the row relative parameters
  if( m_iNumRowsMinus1 >= (1<<(LOG2_MAX_NUM_ROWS_MINUS1+1)) )
  {
    printf( "The number of rows is larger than the maximum allowed number of rows.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumRowsMinus1 >= iHeightInCU )
  {
    printf( "The current picture can not have so many rows.\n" );
    exit( EXIT_FAILURE );
  }

  if( m_iNumRowsMinus1 && m_iUniformSpacingIdr==0 )
  {
    for(Int i=0; i<m_iNumRowsMinus1; i++)
      uiCummulativeRowHeight += m_puiRowHeight[i];

    if( uiCummulativeRowHeight >= iHeightInCU )
    {
      printf( "The height of the row is too large.\n" );
      exit( EXIT_FAILURE );
    }
  }
}
//! \}
