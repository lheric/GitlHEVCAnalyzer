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

/** \file     TDecSlice.cpp
    \brief    slice decoder class
*/

#include "TDecSlice.h"
#if ENABLE_ANAYSIS_OUTPUT
#include "TLibSysuAnalyzer/TSysuAnalyzerOutput.h"
#endif
//! \ingroup TLibDecoder
//! \{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TDecSlice::TDecSlice()
{
  m_pcBufferSbacDecoders = NULL;
  m_pcBufferBinCABACs    = NULL;
  m_pcBufferLowLatSbacDecoders = NULL;
  m_pcBufferLowLatBinCABACs    = NULL;
}

TDecSlice::~TDecSlice()
{
}

Void TDecSlice::create( TComSlice* pcSlice, Int iWidth, Int iHeight, UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxDepth )
{
}

Void TDecSlice::destroy()
{
  if ( m_pcBufferSbacDecoders )
  {
    delete[] m_pcBufferSbacDecoders;
    m_pcBufferSbacDecoders = NULL;
  }
  if ( m_pcBufferBinCABACs )
  {
    delete[] m_pcBufferBinCABACs;
    m_pcBufferBinCABACs = NULL;
  }
  if ( m_pcBufferLowLatSbacDecoders )
  {
    delete[] m_pcBufferLowLatSbacDecoders;
    m_pcBufferLowLatSbacDecoders = NULL;
  }
  if ( m_pcBufferLowLatBinCABACs )
  {
    delete[] m_pcBufferLowLatBinCABACs;
    m_pcBufferLowLatBinCABACs = NULL;
  }
}

Void TDecSlice::init(TDecEntropy* pcEntropyDecoder, TDecCu* pcCuDecoder)
{
  m_pcEntropyDecoder  = pcEntropyDecoder;
  m_pcCuDecoder       = pcCuDecoder;
}

Void TDecSlice::decompressSlice(TComInputBitstream* pcBitstream, TComInputBitstream** ppcSubstreams, TComPic*& rpcPic, TDecSbac* pcSbacDecoder, TDecSbac* pcSbacDecoders)
{
  TComDataCU* pcCU;
  UInt        uiIsLast = 0;
  Int   iStartCUEncOrder = max(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr()/rpcPic->getNumPartInCU(), rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getEntropySliceCurStartCUAddr()/rpcPic->getNumPartInCU());
  Int   iStartCUAddr = rpcPic->getPicSym()->getCUOrderMap(iStartCUEncOrder);

  // decoder don't need prediction & residual frame buffer
#if ENABLE_ANAYSIS_OUTPUT

#else
  rpcPic->setPicYuvPred( 0 );
  rpcPic->setPicYuvResi( 0 );
#endif
  
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

  UInt uiTilesAcross   = rpcPic->getPicSym()->getNumColumnsMinus1()+1;
  TComSlice*  pcSlice = rpcPic->getSlice(rpcPic->getCurrSliceIdx());
  UInt iSymbolMode    = pcSlice->getPPS()->getEntropyCodingMode();
  Int  iNumSubstreams = pcSlice->getPPS()->getNumSubstreams();

  if( iSymbolMode )
  {
    m_pcBufferSbacDecoders = new TDecSbac    [uiTilesAcross];  
    m_pcBufferBinCABACs    = new TDecBinCABAC[uiTilesAcross];
    for (UInt ui = 0; ui < uiTilesAcross; ui++)
    {
      m_pcBufferSbacDecoders[ui].init(&m_pcBufferBinCABACs[ui]);
    }
    //save init. state
    for (UInt ui = 0; ui < uiTilesAcross; ui++)
    {
      m_pcBufferSbacDecoders[ui].load(pcSbacDecoder);
    }
  }  
  if( iSymbolMode )
  {
    m_pcBufferLowLatSbacDecoders = new TDecSbac    [uiTilesAcross];  
    m_pcBufferLowLatBinCABACs    = new TDecBinCABAC[uiTilesAcross];
    for (UInt ui = 0; ui < uiTilesAcross; ui++)
      m_pcBufferLowLatSbacDecoders[ui].init(&m_pcBufferLowLatBinCABACs[ui]);
    //save init. state
    for (UInt ui = 0; ui < uiTilesAcross; ui++)
      m_pcBufferLowLatSbacDecoders[ui].load(pcSbacDecoder);
  }

  UInt uiWidthInLCUs  = rpcPic->getPicSym()->getFrameWidthInCU();
  //UInt uiHeightInLCUs = rpcPic->getPicSym()->getFrameHeightInCU();
  UInt uiCol=0, uiLin=0, uiSubStrm=0;
  Int iBreakDep;
  UInt uiTileCol;
  UInt uiTileStartLCU;
  UInt uiTileLCUX;
  UInt uiTileLCUY;
  UInt uiTileWidth;
  UInt uiTileHeight;
  Int iNumSubstreamsPerTile = 1; // if independent.

  for( Int iCUAddr = iStartCUAddr; !uiIsLast && iCUAddr < rpcPic->getNumCUsInFrame(); iCUAddr = rpcPic->getPicSym()->xCalculateNxtCUAddr(iCUAddr) )
  {
    pcCU = rpcPic->getCU( iCUAddr );
    pcCU->initCU( rpcPic, iCUAddr );

    iBreakDep = rpcPic->getPicSym()->getTileBoundaryIndependenceIdr();
    uiTileCol = rpcPic->getPicSym()->getTileIdxMap(iCUAddr) % (rpcPic->getPicSym()->getNumColumnsMinus1()+1); // what column of tiles are we in?
    uiTileStartLCU = rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getFirstCUAddr();
    uiTileLCUX = uiTileStartLCU % uiWidthInLCUs;
    uiTileLCUY = uiTileStartLCU / uiWidthInLCUs;
    uiTileWidth = rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getTileWidth();
    uiTileHeight = rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getTileHeight();
    uiCol     = iCUAddr % uiWidthInLCUs;
    uiLin     = iCUAddr / uiWidthInLCUs;
    // inherit from TR if necessary, select substream to use.
    if( iSymbolMode && pcSlice->getPPS()->getEntropyCodingSynchro() )
    {
      if (iBreakDep && pcSlice->getPPS()->getEntropyCodingSynchro())
      {
        // independent tiles => substreams are "per tile".  iNumSubstreams has already been multiplied.
        iNumSubstreamsPerTile = iNumSubstreams/rpcPic->getPicSym()->getNumTiles();
        uiSubStrm = rpcPic->getPicSym()->getTileIdxMap(iCUAddr)*iNumSubstreamsPerTile
                      + uiLin%iNumSubstreamsPerTile;
      }
      else
      {
        // dependent tiles => substreams are "per frame".
        uiSubStrm = uiLin % iNumSubstreams;
      }
      m_pcEntropyDecoder->setBitstream( ppcSubstreams[uiSubStrm] );
      // Synchronize cabac probabilities with upper-right LCU if it's available and we're at the start of a line.
      if (pcSlice->getPPS()->getEntropyCodingSynchro() && uiCol == uiTileLCUX)
      {
        // We'll sync if the TR is available.
        TComDataCU *pcCUUp = pcCU->getCUAbove();
        UInt uiWidthInCU = rpcPic->getFrameWidthInCU();
        TComDataCU *pcCUTR = NULL;
        if ( pcCUUp && ((iCUAddr%uiWidthInCU+pcSlice->getPPS()->getEntropyCodingSynchro()) < uiWidthInCU)  )
        {
          pcCUTR = rpcPic->getCU( iCUAddr - uiWidthInCU + pcSlice->getPPS()->getEntropyCodingSynchro() );
        }
        UInt uiMaxParts = 1<<(pcSlice->getSPS()->getMaxCUDepth()<<1);

        if ( (true/*bEnforceSliceRestriction*/ &&
             ((pcCUTR==NULL) || (pcCUTR->getSlice()==NULL) || 
             ((pcCUTR->getSCUAddr()+uiMaxParts-1) < pcSlice->getSliceCurStartCUAddr()) ||
             (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && (rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(iCUAddr)))
             ))||
             (true/*bEnforceEntropySliceRestriction*/ &&
             ((pcCUTR==NULL) || (pcCUTR->getSlice()==NULL) || 
             ((pcCUTR->getSCUAddr()+uiMaxParts-1) < pcSlice->getEntropySliceCurStartCUAddr()) ||
             (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr() && (rpcPic->getPicSym()->getTileIdxMap( pcCUTR->getAddr() ) != rpcPic->getPicSym()->getTileIdxMap(iCUAddr)))
             ))
           )
        {
          // TR not available.
        }
        else
        {
          // TR is available, we use it.
            pcSbacDecoders[uiSubStrm].loadContexts( &m_pcBufferSbacDecoders[uiTileCol] );
        }
      }
      pcSbacDecoder->load(&pcSbacDecoders[uiSubStrm]);  //this load is used to simplify the code (avoid to change all the call to pcSbacDecoders)
    }
    else if ( iSymbolMode && !pcSlice->getPPS()->getEntropyCodingSynchro() )
    {
      // Set variables to appropriate values to avoid later code change.
      iNumSubstreamsPerTile = 1;
    }

    if ( (iCUAddr == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getFirstCUAddr()) && // 1st in tile.
         (iCUAddr!=0) && (iCUAddr!=rpcPic->getPicSym()->getPicSCUAddr(rpcPic->getSlice(rpcPic->getCurrSliceIdx())->getSliceCurStartCUAddr())/rpcPic->getNumPartInCU())) // !1st in frame && !1st in slice
    {
      if (pcSlice->getPPS()->getEntropyCodingSynchro())
      {
        // We're crossing into another tile, tiles are independent.
        // When tiles are independent, we have "substreams per tile".  Each substream has already been terminated, and we no longer
        // have to perform it here.
        // For TILES_DECODER, there can be a header at the start of the 1st substream in a tile.  These are read when the substreams
        // are extracted, not here.
      }
      else
      {
        m_pcEntropyDecoder->updateContextTables( pcSlice->getSliceType(), pcSlice->getSliceQp() );
      }
      
      Bool bTileMarkerFoundFlag = false;
      TComInputBitstream *pcTmpPtr;
      pcTmpPtr = ppcSubstreams[uiSubStrm]; // for CABAC

      for (UInt uiIdx=0; uiIdx<pcTmpPtr->getTileMarkerLocationCount(); uiIdx++)
      {
        if ( pcTmpPtr->getByteLocation() == (pcTmpPtr->getTileMarkerLocation( uiIdx )+2) )
        {
          bTileMarkerFoundFlag = true;
          break;
        }
      }

      if (bTileMarkerFoundFlag)
      {
        UInt uiTileIdx;
        // Read tile index
        m_pcEntropyDecoder->readTileMarker( uiTileIdx, rpcPic->getPicSym()->getBitsUsedByTileIdx() );
      }
    }

    if ( (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()==0) && (rpcPic->getPicSym()->getNumColumnsMinus1()!=0) )
    {    
      // Synchronize cabac probabilities with LCU among Tiles
      if( (uiTileLCUX != 0) &&
          (iCUAddr == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getFirstCUAddr()) )
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
          pcSbacDecoders[uiSubStrm].loadContexts( &m_pcBufferLowLatSbacDecoders[uiTileCol-1] );
          pcSbacDecoder->loadContexts(&pcSbacDecoders[uiSubStrm]);  //this load is used to simplify the code (avoid to change all the call to pcSbacDecoders)
        }
      }
    }

#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceEnable;
#endif
#if ENABLE_ANAYSIS_OUTPUT
    UInt uiBefore = ppcSubstreams[uiSubStrm]->getByteLocation();
    TSysuAnalyzerOutput::getInstance()->aiCUBits.clear();
    m_pcCuDecoder->decodeCU     ( ppcSubstreams[uiSubStrm], pcCU, uiIsLast );
    pcCU->getTotalBits() = ppcSubstreams[uiSubStrm]->getByteLocation() - uiBefore;
#else
    m_pcCuDecoder->decodeCU     ( pcCU, uiIsLast );
#endif
    m_pcCuDecoder->decompressCU ( pcCU );


#if ENC_DEC_TRACE
    g_bJustDoIt = g_bEncDecTraceDisable;
#endif
    if( iSymbolMode )
    {
#if OL_FLUSH
      /*If at the end of a LCU line but not at the end of a substream, perform CABAC flush*/
      if (!uiIsLast && pcSlice->getPPS()->getCabacIstateReset())
      {
        if ((iBreakDep && (uiCol == uiTileLCUX+uiTileWidth-1) && (uiLin+iNumSubstreamsPerTile < uiTileLCUY+uiTileHeight))
            || (!iBreakDep && (uiCol == uiWidthInLCUs-1) && (uiLin+iNumSubstreams < pcCU->getPic()->getFrameHeightInCU())))
        {
          m_pcEntropyDecoder->decodeFlush();
        }
      }
#endif
      pcSbacDecoders[uiSubStrm].load(pcSbacDecoder);

      //Store probabilities of second LCU in line into buffer
      if (pcSlice->getPPS()->getEntropyCodingSynchro() && (uiCol == uiTileLCUX+pcSlice->getPPS()->getEntropyCodingSynchro()))
      {
        m_pcBufferSbacDecoders[uiTileCol].loadContexts( &pcSbacDecoders[uiSubStrm] );
      }

    }
    if ( (rpcPic->getPicSym()->getTileBoundaryIndependenceIdr()==0) && (rpcPic->getPicSym()->getNumColumnsMinus1()!=0) )
    {
      pcSbacDecoders[uiSubStrm].load(pcSbacDecoder);
       //Store probabilties for next tile
      if( (uiLin == (rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getFirstCUAddr() / uiWidthInLCUs )) && 
          (uiCol == rpcPic->getPicSym()->getTComTile(rpcPic->getPicSym()->getTileIdxMap(iCUAddr))->getRightEdgePosInCU()) )
      {
        m_pcBufferLowLatSbacDecoders[uiTileCol].loadContexts( &pcSbacDecoders[uiSubStrm] );
      }
    }
  }

}

ParameterSetManagerDecoder::ParameterSetManagerDecoder()
: m_spsBuffer(256)
, m_ppsBuffer(16)
, m_apsBuffer(64)
{

}

ParameterSetManagerDecoder::~ParameterSetManagerDecoder()
{

}

TComSPS* ParameterSetManagerDecoder::getPrefetchedSPS  (Int spsId)
{
  if (m_spsBuffer.getPS(spsId) != NULL )
  {
    return m_spsBuffer.getPS(spsId);
  }
  else
  {
    return getSPS(spsId);
  }
}

TComPPS* ParameterSetManagerDecoder::getPrefetchedPPS  (Int ppsId)
{
  if (m_ppsBuffer.getPS(ppsId) != NULL )
  {
    return m_ppsBuffer.getPS(ppsId);
  }
  else
  {
    return getPPS(ppsId);
  }
}

TComAPS* ParameterSetManagerDecoder::getPrefetchedAPS  (Int apsId)
{
  if (m_apsBuffer.getPS(apsId) != NULL )
  {
    return m_apsBuffer.getPS(apsId);
  }
  else
  {
    return getAPS(apsId);
  }
}

Void     ParameterSetManagerDecoder::applyPrefetchedPS()
{
  m_apsMap.mergePSList(m_apsBuffer);
  m_ppsMap.mergePSList(m_ppsBuffer);
  m_spsMap.mergePSList(m_spsBuffer);
}

//! \}
