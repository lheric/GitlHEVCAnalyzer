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

/** \file     TComLoopFilter.cpp
    \brief    deblocking filter
*/

#include "TComLoopFilter.h"
#include "TComSlice.h"
#include "TComMv.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define   EDGE_VER    0
#define   EDGE_HOR    1
#define   QpUV(iQpY)  ( g_aucChromaScale[ max( min( (iQpY), MAX_QP ), MIN_QP ) ] )


#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
#define   DECIDE_FILTER                0
#define   EXECUTE_FILTER               1
#define   DECIDE_AND_EXECUTE_FILTER    2
#endif

#define F143 1
#if F143
#define DEFAULT_INTRA_TC_OFFSET 2 ///< Default intra TC offset
#else
#define DEFAULT_INTRA_TC_OFFSET 4 ///< Default intra TC offset
#endif

// ====================================================================================================================
// Tables
// ====================================================================================================================

const UChar tctable_8x8[56] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,5,5,6,6,7,8,9,9,10,10,11,11,12,12,13,13,14,14
};

const UChar betatable_8x8[52] =
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64
};

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComLoopFilter::TComLoopFilter()
: m_uiNumPartitions( 0 )
{
  m_uiDisableDeblockingFilterIdc = 0;
}

TComLoopFilter::~TComLoopFilter()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TComLoopFilter::setCfg( UInt uiDisableDblkIdc, Int iAlphaOffset, Int iBetaOffset)
{
  m_uiDisableDeblockingFilterIdc  = uiDisableDblkIdc;
}

#if PARALLEL_MERGED_DEBLK
Void TComLoopFilter::create( Int width, Int height, Int maxCUWidth, Int maxCUHeight, Int uiMaxCUDepth )
#else
Void TComLoopFilter::create( UInt uiMaxCUDepth )
#endif
{
  m_uiNumPartitions = 1 << ( uiMaxCUDepth<<1 );
  for( UInt uiDir = 0; uiDir < 2; uiDir++ )
  {
    for( UInt uiPlane = 0; uiPlane < 3; uiPlane++ )
    {
      m_aapucBS       [uiDir][uiPlane] = new UChar[m_uiNumPartitions];
      m_aapbEdgeFilter[uiDir][uiPlane] = new Bool [m_uiNumPartitions];
    }
  }
  
#if PARALLEL_MERGED_DEBLK
  m_preDeblockPic.createLuma( width, height, maxCUWidth, maxCUHeight, uiMaxCUDepth );
#endif
}

Void TComLoopFilter::destroy()
{
  for( UInt uiDir = 0; uiDir < 2; uiDir++ )
  {
    for( UInt uiPlane = 0; uiPlane < 3; uiPlane++ )
    {
      delete [] m_aapucBS       [uiDir][uiPlane];
      delete [] m_aapbEdgeFilter[uiDir][uiPlane];
    }
  }
  
#if PARALLEL_MERGED_DEBLK
  m_preDeblockPic.destroyLuma();
#endif
}

/**
 - call deblocking function for every CU
 .
 \param  pcPic   picture class (TComPic) pointer
 */
Void TComLoopFilter::loopFilterPic( TComPic* pcPic )
{
  if (m_uiDisableDeblockingFilterIdc == 1)
    return;
  
#if PARALLEL_MERGED_DEBLK
  pcPic->getPicYuvRec()->copyToPicLuma(&m_preDeblockPic);

  // Horizontal filtering
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

    for( Int iPlane = 0; iPlane < 3; iPlane++ )
    {
      ::memset( m_aapucBS       [EDGE_VER][iPlane], 0, sizeof( UChar ) * m_uiNumPartitions );
      ::memset( m_aapbEdgeFilter[EDGE_VER][iPlane], 0, sizeof( bool  ) * m_uiNumPartitions );
    }

    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0, EDGE_VER );
  }

  // Vertical filtering
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );

    for( Int iPlane = 0; iPlane < 3; iPlane++ )
    {
      ::memset( m_aapucBS       [EDGE_HOR][iPlane], 0, sizeof( UChar ) * m_uiNumPartitions );
      ::memset( m_aapbEdgeFilter[EDGE_HOR][iPlane], 0, sizeof( bool  ) * m_uiNumPartitions );
    }

    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0, EDGE_HOR );
  }
#else
  // for every CU
  for ( UInt uiCUAddr = 0; uiCUAddr < pcPic->getNumCUsInFrame(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pcPic->getCU( uiCUAddr );
    
    for( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
    {
      for( Int iPlane = 0; iPlane < 3; iPlane++ )
      {
        ::memset( m_aapucBS       [iDir][iPlane], 0, sizeof( UChar ) * m_uiNumPartitions );
        ::memset( m_aapbEdgeFilter[iDir][iPlane], 0, sizeof( bool  ) * m_uiNumPartitions );
      }
    }
    // CU-based deblocking
    xDeblockCU( pcCU, 0, 0 );
  }
#endif
}


// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

#if PARALLEL_MERGED_DEBLK
/**
 - Deblocking filter process in CU-based (the same function as conventional's)
 .
 \param Edge          the direction of the edge in block boundary (horizonta/vertical), which is added newly
*/
Void TComLoopFilter::xDeblockCU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int Edge )
#else
Void TComLoopFilter::xDeblockCU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
#endif
{
  TComPic* pcPic     = pcCU->getPic();
  UInt uiCurNumParts = pcPic->getNumPartInCU() >> (uiDepth<<1);
  UInt uiQNumParts   = uiCurNumParts>>2;
  
  if( pcCU->getDepth(uiAbsZorderIdx) > uiDepth )
  {
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx+=uiQNumParts )
    {
      UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsZorderIdx] ];
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#if PARALLEL_MERGED_DEBLK
        xDeblockCU( pcCU, uiAbsZorderIdx, uiDepth+1, Edge );
#else
        xDeblockCU( pcCU, uiAbsZorderIdx, uiDepth+1 );
#endif
    }
    return;
  }
  
  xSetLoopfilterParam( pcCU, uiAbsZorderIdx );
  
#if NSQT
  xSetEdgefilterTU   ( pcCU, g_auiZscanToRaster[ uiAbsZorderIdx ], uiAbsZorderIdx, uiDepth );
#else
  xSetEdgefilterTU   ( pcCU, uiAbsZorderIdx, uiDepth );
#endif
  xSetEdgefilterPU   ( pcCU, uiAbsZorderIdx );
  
#if PARALLEL_MERGED_DEBLK
  Int iDir = Edge;
#else
  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
#endif
  {
    for( UInt uiPartIdx = uiAbsZorderIdx; uiPartIdx < uiAbsZorderIdx + uiCurNumParts; uiPartIdx++ )
    {
      if ( m_aapbEdgeFilter[iDir][0][uiPartIdx] )
      {
        xGetBoundaryStrengthSingle ( pcCU, uiAbsZorderIdx, iDir, uiPartIdx );
      }
    }
  }
  
  UInt uiPelsInPart = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  UInt PartIdxIncr = DEBLOCK_SMALLEST_BLOCK / uiPelsInPart ? DEBLOCK_SMALLEST_BLOCK / uiPelsInPart : 1 ;
  
  UInt uiSizeInPU = pcPic->getNumPartInWidth()>>(uiDepth);
  
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
  {
    xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, iEdge, DECIDE_FILTER);//Decide horizontal filter
  }
  for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
  {
    xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, iEdge, DECIDE_AND_EXECUTE_FILTER);//Decide vertical filter
    if ( (iEdge % ( (DEBLOCK_SMALLEST_BLOCK<<1)/uiPelsInPart ) ) == 0 )
    {
      xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, iEdge );
    }
  }  
  for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
  {
    xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, iEdge, EXECUTE_FILTER );//Execute horizontal filter
    if ( (iEdge % ( (DEBLOCK_SMALLEST_BLOCK<<1)/uiPelsInPart ) ) == 0 )
    {
      xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, iEdge );
    }
  }  
#else  
#if !PARALLEL_MERGED_DEBLK
  for ( Int iDir = EDGE_VER; iDir <= EDGE_HOR; iDir++ )
#endif
  {
    for ( UInt iEdge = 0; iEdge < uiSizeInPU ; iEdge+=PartIdxIncr)
    {
      xEdgeFilterLuma     ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
      if ( (iEdge % ( (DEBLOCK_SMALLEST_BLOCK<<1)/uiPelsInPart ) ) == 0 )
        xEdgeFilterChroma   ( pcCU, uiAbsZorderIdx, uiDepth, iDir, iEdge );
    }
  }
#endif  
}


#if NSQT
Void TComLoopFilter::xSetEdgefilterMultiple( TComDataCU* pcCU, UInt uiScanIdx, UInt uiDepth, Int iDir, Int iEdgeIdx, Bool bValue,UInt uiWidthInBaseUnits, UInt uiHeightInBaseUnits)
{  
  if ( uiWidthInBaseUnits == 0 )
  {
    uiWidthInBaseUnits  = pcCU->getPic()->getNumPartInWidth () >> uiDepth;
  }
  if ( uiHeightInBaseUnits == 0 )
  {
    uiHeightInBaseUnits = pcCU->getPic()->getNumPartInHeight() >> uiDepth;
  }
  const UInt uiNumElem = iDir == 0 ? uiHeightInBaseUnits : uiWidthInBaseUnits;
  assert( uiNumElem > 0 );
  assert( uiWidthInBaseUnits > 0 );
  assert( uiHeightInBaseUnits > 0 );
  for( UInt ui = 0; ui < uiNumElem; ui++ )
  {
    UInt uiBsIdx;
    if ( uiWidthInBaseUnits == uiHeightInBaseUnits )
    {
      uiBsIdx = xCalcBsIdx( pcCU, uiScanIdx, iDir, iEdgeIdx, ui, true );
    }
    else
    {
      uiBsIdx = xCalcBsIdx( pcCU, uiScanIdx, iDir, iEdgeIdx, ui, false );
    }
    m_aapbEdgeFilter[iDir][0][uiBsIdx] = bValue;
    m_aapbEdgeFilter[iDir][1][uiBsIdx] = bValue;
    m_aapbEdgeFilter[iDir][2][uiBsIdx] = bValue;
#if F118_LUMA_DEBLOCK
    if (iEdgeIdx == 0)
    {
      m_aapucBS[iDir][0][uiBsIdx] = bValue;
    }
#endif
  }
}
#else
Void TComLoopFilter::xSetEdgefilterMultiple( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdgeIdx, Bool bValue )
{
  const UInt uiWidthInBaseUnits  = pcCU->getPic()->getNumPartInWidth () >> uiDepth;
  const UInt uiHeightInBaseUnits = pcCU->getPic()->getNumPartInHeight() >> uiDepth;
  const UInt uiNumElem = iDir == 0 ? uiHeightInBaseUnits : uiWidthInBaseUnits;
  assert( uiNumElem > 0 );
  for( UInt ui = 0; ui < uiNumElem; ui++ )
  {
    const UInt uiBsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdgeIdx, ui );
    m_aapbEdgeFilter[iDir][0][uiBsIdx] = bValue;
    m_aapbEdgeFilter[iDir][1][uiBsIdx] = bValue;
    m_aapbEdgeFilter[iDir][2][uiBsIdx] = bValue;
#if F118_LUMA_DEBLOCK
    if (iEdgeIdx == 0)
    {
      m_aapucBS[iDir][0][uiBsIdx] = bValue;
    }
#endif
  }
}
#endif

#if NSQT
Void TComLoopFilter::xSetEdgefilterTU( TComDataCU* pcCU, UInt uiRasterIdx, UInt uiAbsZorderIdx, UInt uiDepth )
#else
Void TComLoopFilter::xSetEdgefilterTU( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth )
#endif
{
  if( pcCU->getTransformIdx( uiAbsZorderIdx ) + pcCU->getDepth( uiAbsZorderIdx) > uiDepth )
  {
    const UInt uiCurNumParts = pcCU->getPic()->getNumPartInCU() >> (uiDepth<<1);
    const UInt uiQNumParts   = uiCurNumParts>>2;
#if NSQT
    const UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> ( uiDepth + 1 ) ] + 2;
    if( !pcCU->isIntra( uiAbsZorderIdx ) && pcCU->getTransformIdx( uiAbsZorderIdx ) && uiLog2TrSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() && pcCU->getWidth( uiAbsZorderIdx ) > 8 &&
#if AMP
      ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_2NxN || 
      ( pcCU->getPartitionSize( uiAbsZorderIdx ) >= SIZE_2NxnU && pcCU->getPartitionSize( uiAbsZorderIdx ) <= SIZE_nRx2N ) ) )
#else
      ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N ||pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_2NxN ) )
#endif
    {
      TComPic* const pcPic = pcCU->getPic();
      const UInt uiLCUWidthInBaseUnits = pcPic->getNumPartInWidth();

      if( ( pcCU->getDepth( uiAbsZorderIdx) && pcCU->getDepth( uiAbsZorderIdx) == uiDepth) || 
        ( pcCU->getWidth( 0 ) == 64 && ( uiDepth - pcCU->getDepth( uiAbsZorderIdx) == 1  ) ) ) 
      {
        const UInt iBaseUnitIdx = uiLCUWidthInBaseUnits >> ( uiDepth + 1 );
#if AMP
        UInt offset = ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nLx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nRx2N ) ? iBaseUnitIdx : iBaseUnitIdx * uiLCUWidthInBaseUnits;
#else
        UInt offset = ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N ) ? iBaseUnitIdx : iBaseUnitIdx * uiLCUWidthInBaseUnits;
#endif
        for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx += uiQNumParts )
        {
          xSetEdgefilterTU( pcCU, uiRasterIdx+uiPartIdx * offset, uiAbsZorderIdx, uiDepth+1 );
        }
      }
      else
      {
        if( ( 1 << uiLog2TrSize ) < DEBLOCK_SMALLEST_BLOCK )
          return;

#if AMP
        UInt uiTrWidth  = ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nLx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nRx2N ) ? 1 << ( uiLog2TrSize - 4 + 1) : 1 << ( uiLog2TrSize - 2 + 1 );
        UInt uiTrHeight = ( pcCU->getPartitionSize( uiAbsZorderIdx  )== SIZE_Nx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nLx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nRx2N ) ? 1 << ( uiLog2TrSize - 2 + 1) : 1 << ( uiLog2TrSize - 4 + 1 );
#else
        UInt uiTrWidth  = ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N ) ? 1 << ( uiLog2TrSize - 4 + 1) : 1 << ( uiLog2TrSize - 2 + 1 );
        UInt uiTrHeight = ( pcCU->getPartitionSize( uiAbsZorderIdx  )== SIZE_Nx2N ) ? 1 << ( uiLog2TrSize - 2 + 1) : 1 << ( uiLog2TrSize - 4 + 1 );
#endif
        xSetEdgefilterTU( pcCU, uiRasterIdx,                                                  uiAbsZorderIdx, uiDepth + 1 ); uiAbsZorderIdx += uiQNumParts;
        xSetEdgefilterTU( pcCU, uiRasterIdx + uiTrWidth,                                      uiAbsZorderIdx, uiDepth + 1 ); uiAbsZorderIdx += uiQNumParts;
        xSetEdgefilterTU( pcCU, uiRasterIdx + uiTrHeight * uiLCUWidthInBaseUnits,             uiAbsZorderIdx, uiDepth + 1 ); uiAbsZorderIdx += uiQNumParts;
        xSetEdgefilterTU( pcCU, uiRasterIdx + uiTrHeight * uiLCUWidthInBaseUnits + uiTrWidth, uiAbsZorderIdx, uiDepth + 1 );
      }
    }
    else
#endif 
    for ( UInt uiPartIdx = 0; uiPartIdx < 4; uiPartIdx++, uiAbsZorderIdx+=uiQNumParts )
    {
#if NSQT
      xSetEdgefilterTU( pcCU,uiRasterIdx, uiAbsZorderIdx, uiDepth + 1 );
#else
      xSetEdgefilterTU( pcCU, uiAbsZorderIdx, uiDepth+1 );
#endif
    }
    return;
  }

#if NSQT
  const UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
  if( !pcCU->isIntra( uiAbsZorderIdx ) && pcCU->getTransformIdx( uiAbsZorderIdx ) && uiLog2TrSize<pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() && pcCU->getWidth( uiAbsZorderIdx ) > 8 &&
#if AMP
    ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_2NxN || 
    ( pcCU->getPartitionSize( uiAbsZorderIdx ) >= SIZE_2NxnU && pcCU->getPartitionSize( uiAbsZorderIdx ) <= SIZE_nRx2N ) ) )
#else
    ( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_2NxN ) )
#endif
  {
    if( ( 1 << uiLog2TrSize ) < DEBLOCK_SMALLEST_BLOCK )
      return;

    UInt uiWidthInBaseUnits  = pcCU->getPic()->getNumPartInWidth () >> uiDepth;
    UInt uiHeightInBaseUnits = pcCU->getPic()->getNumPartInWidth () >> uiDepth;

#if AMP
    if( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nLx2N || pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_nRx2N )
#else
    if( pcCU->getPartitionSize( uiAbsZorderIdx ) == SIZE_Nx2N )
#endif
    {
      uiWidthInBaseUnits  >>= 1;
      uiHeightInBaseUnits <<= 1;
    }
    else
    {
      uiWidthInBaseUnits  <<= 1;
      uiHeightInBaseUnits >>= 1;
    }
    xSetEdgefilterMultiple( pcCU, uiRasterIdx, uiDepth, EDGE_VER, 0, m_stLFCUParam.bInternalEdge, uiWidthInBaseUnits, uiHeightInBaseUnits );
    xSetEdgefilterMultiple( pcCU, uiRasterIdx, uiDepth, EDGE_HOR, 0, m_stLFCUParam.bInternalEdge, uiWidthInBaseUnits, uiHeightInBaseUnits );
  }
  else
  {
#endif
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, 0, m_stLFCUParam.bInternalEdge );
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, 0, m_stLFCUParam.bInternalEdge );
#if NSQT
  }
#endif
}

Void TComLoopFilter::xSetEdgefilterPU( TComDataCU* pcCU, UInt uiAbsZorderIdx )
{
  const UInt uiDepth = pcCU->getDepth( uiAbsZorderIdx );
  const UInt uiWidthInBaseUnits  = pcCU->getPic()->getNumPartInWidth () >> uiDepth;
  const UInt uiHeightInBaseUnits = pcCU->getPic()->getNumPartInHeight() >> uiDepth;
  const UInt uiHWidthInBaseUnits  = uiWidthInBaseUnits  >> 1;
  const UInt uiHHeightInBaseUnits = uiHeightInBaseUnits >> 1;
#if AMP
  const UInt uiQWidthInBaseUnits  = uiWidthInBaseUnits  >> 2;
  const UInt uiQHeightInBaseUnits = uiHeightInBaseUnits >> 2;
#endif
  
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, 0, m_stLFCUParam.bLeftEdge );
  xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, 0, m_stLFCUParam.bTopEdge );
  
  switch ( pcCU->getPartitionSize( uiAbsZorderIdx ) )
  {
    case SIZE_2Nx2N:
    {
      break;
    }
    case SIZE_2NxN:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiHHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
    case SIZE_Nx2N:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiHWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
    case SIZE_NxN:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiHWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiHHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
#if AMP
  case SIZE_2NxnU:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiQHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
  case SIZE_2NxnD:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_HOR, uiHeightInBaseUnits - uiQHeightInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
  case SIZE_nLx2N:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiQWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
  case SIZE_nRx2N:
    {
      xSetEdgefilterMultiple( pcCU, uiAbsZorderIdx, uiDepth, EDGE_VER, uiWidthInBaseUnits - uiQWidthInBaseUnits, m_stLFCUParam.bInternalEdge );
      break;
    }
#endif
    default:
    {
      assert(0);
      break;
    }
  }
}


Void TComLoopFilter::xSetLoopfilterParam( TComDataCU* pcCU, UInt uiAbsZorderIdx )
{
  UInt uiX           = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[ uiAbsZorderIdx ] ];
  UInt uiY           = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[ uiAbsZorderIdx ] ];
  
#if MTK_NONCROSS_INLOOP_FILTER
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
#endif

  m_stLFCUParam.bInternalEdge = m_uiDisableDeblockingFilterIdc ? false : true ;
  
  if ( (uiX == 0) || (m_uiDisableDeblockingFilterIdc == 1) )
    m_stLFCUParam.bLeftEdge = false;
  else
    m_stLFCUParam.bLeftEdge = true;
#if MTK_NONCROSS_INLOOP_FILTER
  if ( m_stLFCUParam.bLeftEdge )
  {
    pcTempCU = pcCU->getPULeft( uiTempPartIdx, uiAbsZorderIdx, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false );
    if ( pcTempCU )
      m_stLFCUParam.bLeftEdge = true;
    else
      m_stLFCUParam.bLeftEdge = false;
  }
#endif
  
  if ( (uiY == 0 ) || (m_uiDisableDeblockingFilterIdc == 1) )
    m_stLFCUParam.bTopEdge = false;
  else
    m_stLFCUParam.bTopEdge = true;
#if MTK_NONCROSS_INLOOP_FILTER
  if ( m_stLFCUParam.bTopEdge )
  {
    pcTempCU = pcCU->getPUAbove( uiTempPartIdx, uiAbsZorderIdx, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false );
    if ( pcTempCU )
      m_stLFCUParam.bTopEdge = true;
    else
      m_stLFCUParam.bTopEdge = false;
  }
#endif
}

Void TComLoopFilter::xGetBoundaryStrengthSingle ( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, UInt uiAbsPartIdx )
{
  const UInt uiHWidth  = pcCU->getWidth( uiAbsZorderIdx ) >> 1;
  const UInt uiHHeight = pcCU->getHeight( uiAbsZorderIdx ) >> 1;
  const bool bAtCUBoundary = iDir == EDGE_VER ? g_auiRasterToPelX[g_auiZscanToRaster[uiAbsZorderIdx]] == g_auiRasterToPelX[g_auiZscanToRaster[uiAbsPartIdx]]
  : g_auiRasterToPelY[g_auiZscanToRaster[uiAbsZorderIdx]] == g_auiRasterToPelY[g_auiZscanToRaster[uiAbsPartIdx]];
  const bool bAtCUHalf     = iDir == EDGE_VER ? ( g_auiRasterToPelX[g_auiZscanToRaster[uiAbsZorderIdx]] + uiHWidth ) == g_auiRasterToPelX[g_auiZscanToRaster[uiAbsPartIdx]]
  : ( g_auiRasterToPelY[g_auiZscanToRaster[uiAbsZorderIdx]] + uiHHeight ) == g_auiRasterToPelY[g_auiZscanToRaster[uiAbsPartIdx]];
  TComSlice* const pcSlice = pcCU->getSlice();
  
  const UInt uiPartQ = uiAbsPartIdx;
  TComDataCU* const pcCUQ = pcCU;
  
  UInt uiPartP;
  TComDataCU* pcCUP;
  UInt uiBs;
  
#if MTK_NONCROSS_INLOOP_FILTER
  //-- Calculate Block Index
  if (iDir == EDGE_VER)
  {
    pcCUP = pcCUQ->getPULeft(uiPartP, uiPartQ, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false);
  }
  else  // (iDir == EDGE_HOR)
  {
    pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false);
  }
#else
  //-- Calculate Block Index
  if (iDir == EDGE_VER)
  {
    pcCUP = pcCUQ->getPULeft(uiPartP, uiPartQ, false, false);
  }
  else  // (iDir == EDGE_HOR)
  {
    pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, false, false);
  }
#endif
  
  //-- Set BS for Intra MB : BS = 4 or 3
  if ( pcCUP->isIntra(uiPartP) || pcCUQ->isIntra(uiPartQ) )
  {
    uiBs = bAtCUBoundary ? 4 : 3;   // Intra MB && MB boundary
  }
  
  //-- Set BS for not Intra MB : BS = 2 or 1 or 0
  if ( !pcCUP->isIntra(uiPartP) && !pcCUQ->isIntra(uiPartQ) )
  {
#if F118_LUMA_DEBLOCK
    if ( m_aapucBS[iDir][0][uiAbsPartIdx] && (pcCUQ->getCbf( uiPartQ, TEXT_LUMA, pcCUQ->getTransformIdx(uiPartQ)) != 0 || pcCUP->getCbf( uiPartP, TEXT_LUMA, pcCUP->getTransformIdx(uiPartP) ) != 0) )
#else
    if ( pcCUQ->getCbf( uiPartQ, TEXT_LUMA, pcCUQ->getTransformIdx(uiPartQ)) != 0 || pcCUP->getCbf( uiPartP, TEXT_LUMA, pcCUP->getTransformIdx(uiPartP) ) != 0)
#endif
    {
      uiBs = 2;
    }
    else
    {
#if REDUCE_UPPER_MOTION_DATA
      if (iDir == EDGE_HOR)
      {
#if MTK_NONCROSS_INLOOP_FILTER
        pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, !pcCU->getSlice()->getSPS()->getLFCrossSliceBoundaryFlag(), false, true);
#else
        pcCUP = pcCUQ->getPUAbove(uiPartP, uiPartQ, false, false, true);
#endif
      }
#endif
      if (pcSlice->isInterB())
      {
        Int iRefIdx;
        Int *piRefP0, *piRefP1, *piRefQ0, *piRefQ1;
        iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartP);
        piRefP0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiPartP);
        piRefP1 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx);
        iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartQ);
        piRefQ0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_1)->getRefIdx(uiPartQ);
        piRefQ1 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx);
        
        
        TComMv pcMvP0 = pcCUP->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartP);
        TComMv pcMvP1 = pcCUP->getCUMvField(REF_PIC_LIST_1)->getMv(uiPartP);
        TComMv pcMvQ0 = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartQ);
        TComMv pcMvQ1 = pcCUQ->getCUMvField(REF_PIC_LIST_1)->getMv(uiPartQ);
        
        if ( ((piRefP0==piRefQ0)&&(piRefP1==piRefQ1)) || ((piRefP0==piRefQ1)&&(piRefP1==piRefQ0)) )
        {
          uiBs = 0;
          if ( piRefP0 != piRefP1 )   // Different L0 & L1
          {
            if ( piRefP0 == piRefQ0 )
            {
              pcMvP0 -= pcMvQ0;   pcMvP1 -= pcMvQ1;
              uiBs = (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
              (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4);
            }
            else
            {
              pcMvP0 -= pcMvQ1;   pcMvP1 -= pcMvQ0;
              uiBs = (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
              (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4);
            }
          }
          else    // Same L0 & L1
          {
            TComMv pcMvSub0 = pcMvP0 - pcMvQ0;
            TComMv pcMvSub1 = pcMvP1 - pcMvQ1;
            pcMvP0 -= pcMvQ1;   pcMvP1 -= pcMvQ0;
            uiBs = ( (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4) |
                    (pcMvP1.getAbsHor() >= 4) | (pcMvP1.getAbsVer() >= 4) ) &&
            ( (pcMvSub0.getAbsHor() >= 4) | (pcMvSub0.getAbsVer() >= 4) |
             (pcMvSub1.getAbsHor() >= 4) | (pcMvSub1.getAbsVer() >= 4) );
          }
        }
        else // for all different Ref_Idx
        {
          uiBs = 1;
        }
      }
      else  // pcSlice->isInterP()
      {
        Int iRefIdx;
        Int *piRefP0, *piRefQ0;
        iRefIdx = pcCUP->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartP);
        piRefP0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        iRefIdx = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getRefIdx(uiPartQ);
        piRefQ0 = (iRefIdx < 0) ? NULL :  (Int*) pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx);
        TComMv pcMvP0 = pcCUP->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartP);
        TComMv pcMvQ0 = pcCUQ->getCUMvField(REF_PIC_LIST_0)->getMv(uiPartQ);
        
        pcMvP0 -= pcMvQ0;
        uiBs = (piRefP0 != piRefQ0) | (pcMvP0.getAbsHor() >= 4) | (pcMvP0.getAbsVer() >= 4);
      }
    }   // enf of "if( one of BCBP == 0 )"
  }   // enf of "if( not Intra )"
  
  m_aapucBS[iDir][0][uiAbsPartIdx] = uiBs;
  if ( bAtCUBoundary || bAtCUHalf )
  {
    m_aapucBS[iDir][1][uiAbsPartIdx] = uiBs;
    m_aapucBS[iDir][2][uiAbsPartIdx] = uiBs;
  }
}


#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
Void TComLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge, Int iDecideExecute )
#else
Void TComLoopFilter::xEdgeFilterLuma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge  )
#endif
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Pel* piSrc    = pcPicYuvRec->getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel* piTmpSrc = piSrc;
#if PARALLEL_MERGED_DEBLK
  Pel* piSrcJudge    = m_preDeblockPic.getLumaAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel* piTmpSrcJudge = piSrcJudge;
#endif

  Int  iStride = pcPicYuvRec->getStride();
  Int  iQP = pcCU->getQP( uiAbsZorderIdx );
#if E057_INTRA_PCM
  if(pcCU->getIPCMFlag( uiAbsZorderIdx )) 
  {
    iQP = 0; 
  }
#endif
  UInt uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;
  
  UInt  uiPelsInPart = g_uiMaxCUWidth >> g_uiMaxCUDepth;
  UInt  PartIdxIncr = DEBLOCK_SMALLEST_BLOCK / uiPelsInPart ? DEBLOCK_SMALLEST_BLOCK / uiPelsInPart : 1;
  UInt  uiBlocksInPart = uiPelsInPart / DEBLOCK_SMALLEST_BLOCK ? uiPelsInPart / DEBLOCK_SMALLEST_BLOCK : 1;
  UInt  uiBsAbsIdx, uiBs;
  Int   iOffset, iSrcStep;
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  Pel*  piTmpSrc1;
  UInt* piDecisions_D;
  UInt* piDecisions_Sample;
#if F118_LUMA_DEBLOCK
  UInt* piDecisions_DP;
  UInt* piDecisions_DQ;
#endif
#endif  

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  Bool  bPCMFilter = (pcCU->getSlice()->getSPS()->getPCMFilterDisableFlag() && ((1<<pcCU->getSlice()->getSPS()->getPCMLog2MinSize()) <= g_uiMaxCUWidth))? true : false;
  Bool  bPartPNoFilter = false;
  Bool  bPartQNoFilter = false; 
  UInt  uiPartPIdx;
  UInt  uiPartQIdx;
  TComDataCU* pcCUP; 
  TComDataCU* pcCUQ = pcCU;
#endif

  if (iDir == EDGE_VER)
  {
    iOffset = 1;
    iSrcStep = iStride;
    piTmpSrc += iEdge*uiPelsInPart;
#if PARALLEL_MERGED_DEBLK
    piTmpSrcJudge += iEdge*uiPelsInPart;
#endif
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset = iStride;
    iSrcStep = 1;
    piTmpSrc += iEdge*uiPelsInPart*iStride;
#if PARALLEL_MERGED_DEBLK
    piTmpSrcJudge += iEdge*uiPelsInPart*iStride;
#endif
  }
  
  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx+=PartIdxIncr )
  {
    
    uiBs = 0;
    for (UInt iIdxInside = 0; iIdxInside<PartIdxIncr; iIdxInside++)
    {
      uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx+iIdxInside);
      if (uiBs < m_aapucBS[iDir][0][uiBsAbsIdx])
      {
        uiBs = m_aapucBS[iDir][0][uiBsAbsIdx];
      }
    }
    
    Int iBitdepthScale = (1<<(g_uiBitIncrement+g_uiBitDepth-8));
    
    Int uiTcOffset = ( uiBs > 2 ) ? DEFAULT_INTRA_TC_OFFSET : 0;
    
    Int iIndexTC = Clip3(0, MAX_QP+4, iQP + uiTcOffset );
    Int iIndexB = Clip3(0, MAX_QP, iQP );
    
    Int iTc =  tctable_8x8[iIndexTC]*iBitdepthScale;
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
    if (iDecideExecute==DECIDE_AND_EXECUTE_FILTER && uiBs)
    {
      Int iBeta = betatable_8x8[iIndexB]*iBitdepthScale;
      
#if F118_LUMA_DEBLOCK
      Int iSideThreshold = (iBeta+(iBeta>>1))>>3;
      Int iThrCut = iTc*10;
#endif          
      for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
      {
        Int iTmp=iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK;
#if F118_LUMA_DEBLOCK
        Int iDP = xCalcDP( piTmpSrc+iSrcStep*(iTmp+2), iOffset) + xCalcDP( piTmpSrc+iSrcStep*(iTmp+5), iOffset);
        Int iDQ = xCalcDQ( piTmpSrc+iSrcStep*(iTmp+2), iOffset) + xCalcDQ( piTmpSrc+iSrcStep*(iTmp+5), iOffset);
        Int iD = iDP + iDQ;
#else
        Int iD = xCalcD( piTmpSrc+iSrcStep*(iTmp+2), iOffset) + xCalcD( piTmpSrc+iSrcStep*(iTmp+5), iOffset);
#endif        
        
        if (iD < iBeta)
        {
#if F118_LUMA_DEBLOCK
          Bool bFilterP = (iDP < iSideThreshold);  
          Bool bFilterQ = (iDQ < iSideThreshold);
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
          if(bPCMFilter)
          {
            // Derive current PU index
            uiPartQIdx = xCalcBsIdx(pcCUQ, uiAbsZorderIdx, iDir, iEdge, (iIdx+(iBlkIdx*DEBLOCK_SMALLEST_BLOCK/uiPelsInPart)));

            // Derive neighboring PU index
            if (iDir == EDGE_VER)
            {
              pcCUP = pcCUQ->getPULeft (uiPartPIdx, uiPartQIdx);
            }
            else  // (iDir == EDGE_HOR)
            {
              pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx);
            }

            // Check if each of PUs is I_PCM
            bPartPNoFilter = (pcCUP->getIPCMFlag(uiPartPIdx));
            bPartQNoFilter = (pcCUQ->getIPCMFlag(uiPartQIdx));
          }
#endif
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
            xPelFilterLuma( piTmpSrc+iSrcStep*(iTmp+i), iOffset, iD, iBeta, iTc, bPartPNoFilter, bPartQNoFilter, iThrCut, bFilterP, bFilterQ);
#else
            xPelFilterLuma( piTmpSrc+iSrcStep*(iTmp+i), iOffset, iD, iBeta, iTc, bPartPNoFilter, bPartQNoFilter);
#endif
#else
#if F118_LUMA_DEBLOCK
            xPelFilterLuma( piTmpSrc+iSrcStep*(iTmp+i), iOffset, iD, iBeta, iTc, iThrCut, bFilterP, bFilterQ);
#else
            xPelFilterLuma( piTmpSrc+iSrcStep*(iTmp+i), iOffset, iD, iBeta, iTc);
#endif
#endif
          }
        }
      }      
    }
    else if (iDecideExecute==DECIDE_FILTER && uiBs)
    {
      Int iBeta = betatable_8x8[iIndexB]*iBitdepthScale;
#if F118_LUMA_DEBLOCK
      Int iSideThreshold = (iBeta+(iBeta>>1))>>3;
#endif
      
      piDecisions_D      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_D     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      piDecisions_Sample =  iIdx*uiPelsInPart                         + m_decisions_Sample[(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      
#if F118_LUMA_DEBLOCK
      piDecisions_DP      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_DP     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      piDecisions_DQ      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_DQ     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
#endif      
      for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
      {
        Int iTmp=iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK;
#if F118_LUMA_DEBLOCK
        Int iDP = xCalcDP( piTmpSrc+iSrcStep*(iTmp+2), iOffset) + xCalcDP( piTmpSrc+iSrcStep*(iTmp+5), iOffset);
        Int iDQ = xCalcDQ( piTmpSrc+iSrcStep*(iTmp+2), iOffset) + xCalcDQ( piTmpSrc+iSrcStep*(iTmp+5), iOffset);
        Int iD = iDP + iDQ;
#else
        Int iD = xCalcD( piTmpSrc+(iTmp+2), iOffset) + xCalcD( piTmpSrc+(iTmp+5), iOffset);
#endif
        piTmpSrc1 = piTmpSrc+iTmp;
        
        if (iD < iBeta)
        {
          *piDecisions_D++=1;
#if F118_LUMA_DEBLOCK
          Bool bFilterP = (iDP < iSideThreshold);  
          Bool bFilterQ = (iDQ < iSideThreshold);
          *piDecisions_DP++=bFilterP;
          *piDecisions_DQ++=bFilterQ;
#endif
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
            *piDecisions_Sample++ = xPelFilterLumaDecision( piTmpSrc1++, iOffset, iD, iBeta, iTc);
          }
        }
        else
        {
          *piDecisions_D++=0;
          piDecisions_Sample+=DEBLOCK_SMALLEST_BLOCK;
        }
      }
    }
    else if ( uiBs ) // EXECUTE_FILTER
    {
      piDecisions_D      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_D     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
#if F118_LUMA_DEBLOCK
      piDecisions_DP      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_DP     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      piDecisions_DQ      = (iIdx*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK + m_decisions_DQ     [(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      Int iThrCut = iTc*10;
#endif
      piDecisions_Sample =  iIdx*uiPelsInPart                         + m_decisions_Sample[(iEdge*uiPelsInPart)/DEBLOCK_SMALLEST_BLOCK];
      piTmpSrc1          = piTmpSrc+iIdx*uiPelsInPart;
      
      for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
      {
#if F118_LUMA_DEBLOCK
        *piDecisions_DP++;
        *piDecisions_DQ++;
#endif
        if ( *piDecisions_D++ )
        {
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
          if(bPCMFilter)
          {
            // Derive current PU index
            uiPartQIdx = xCalcBsIdx(pcCUQ, uiAbsZorderIdx, iDir, iEdge, (iIdx+(iBlkIdx*DEBLOCK_SMALLEST_BLOCK/uiPelsInPart)));

            // Derive neighboring PU index
            if (iDir == EDGE_VER)
            {
              pcCUP = pcCUQ->getPULeft (uiPartPIdx, uiPartQIdx);
            }
            else  // (iDir == EDGE_HOR)
            {
              pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx);
            }

            // Check if each of PUs is I_PCM
            bPartPNoFilter = (pcCUP->getIPCMFlag(uiPartPIdx));
            bPartQNoFilter = (pcCUQ->getIPCMFlag(uiPartQIdx));
          }
#endif

          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
            xPelFilterLumaExecution( piTmpSrc1++, iOffset, iTc, *piDecisions_Sample++,  bPartPNoFilter, bPartQNoFilter, iThrCut, *piDecisions_DP, *piDecisions_DQ);
#else
            xPelFilterLumaExecution( piTmpSrc1++, iOffset, iTc, *piDecisions_Sample++,  bPartPNoFilter, bPartQNoFilter);
#endif
#else
#if F118_LUMA_DEBLOCK
            xPelFilterLumaExecution( piTmpSrc1++, iOffset, iTc, *piDecisions_Sample++, iThrCut, *piDecisions_DP, *piDecisions_DQ);
#else
            xPelFilterLumaExecution( piTmpSrc1++, iOffset, iTc, *piDecisions_Sample++  );
#endif
#endif
          }
        }
        else
        {
          piDecisions_Sample+=DEBLOCK_SMALLEST_BLOCK;
          piTmpSrc1+=DEBLOCK_SMALLEST_BLOCK;
        }
      }
    }
#else
    Int iBeta = betatable_8x8[iIndexB]*iBitdepthScale;
#if F118_LUMA_DEBLOCK
    Int iSideThreshold = (iBeta+(iBeta>>1))>>3;
    Int iThrCut = iTc*10;
#endif    
    
    
    for (UInt iBlkIdx = 0; iBlkIdx< uiBlocksInPart; iBlkIdx ++)
    {
      if ( uiBs )
      {
#if PARALLEL_MERGED_DEBLK
#if F118_LUMA_DEBLOCK
        Int iDP = xCalcDP( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcDP( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
        Int iDQ = xCalcDQ( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcDQ( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
        Int iD = iDP + iDQ;
#else
        Int iD = xCalcD( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcD( piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
#endif
#else  
#if F118_LUMA_DEBLOCK
        Int iDP = xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcDP( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
        Int iDQ = xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcDQ( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
        Int iD = iDP + iDQ;
#else
        Int iD = xCalcD( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+2), iOffset) + xCalcD( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+5), iOffset);
#endif
#endif
        if (iD < iBeta)
        {
#if F118_LUMA_DEBLOCK
          Bool bFilterP = (iDP < iSideThreshold);  
          Bool bFilterQ = (iDQ < iSideThreshold);
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
          if(bPCMFilter)
          {
            // Derive current PU index
            uiPartQIdx = xCalcBsIdx(pcCUQ, uiAbsZorderIdx, iDir, iEdge, (iIdx+(iBlkIdx*DEBLOCK_SMALLEST_BLOCK/uiPelsInPart)));

            // Derive neighboring PU index
            if (iDir == EDGE_VER)
            {
              pcCUP = pcCUQ->getPULeft (uiPartPIdx, uiPartQIdx);
            }
            else  // (iDir == EDGE_HOR)
            {
              pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx);
            }

            // Check if each of PUs is I_PCM
            bPartPNoFilter = (pcCUP->getIPCMFlag(uiPartPIdx));
            bPartQNoFilter = (pcCUQ->getIPCMFlag(uiPartQIdx));
          }
#endif
          for ( UInt i = 0; i < DEBLOCK_SMALLEST_BLOCK; i++)
          {
#if PARALLEL_MERGED_DEBLK
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc , piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), bPartPNoFilter, bPartQNoFilter, iThrCut, bFilterP, bFilterQ);
#else
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc , piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), bPartPNoFilter, bPartQNoFilter);
#endif
#else
#if F118_LUMA_DEBLOCK
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc , piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i),iThrCut, bFilterP, bFilterQ);
#else
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc , piTmpSrcJudge+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i));
#endif
#endif
#else   
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc , bPartPNoFilter, bPartQNoFilter, iThrCut, bFilterP, bFilterQ);
#else
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc , bPartPNoFilter, bPartQNoFilter);
#endif
#else
#if F118_LUMA_DEBLOCK
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc, iThrCut, bFilterP, bFilterQ );
#else
            xPelFilterLuma( piTmpSrc+iSrcStep*(iIdx*uiPelsInPart+iBlkIdx*DEBLOCK_SMALLEST_BLOCK+i), iOffset, iD, iBeta, iTc );
#endif
#endif
#endif
          }
        }
      }
    }
#endif
  }
}


Void TComLoopFilter::xEdgeFilterChroma( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge )
{
  TComPicYuv* pcPicYuvRec = pcCU->getPic()->getPicYuvRec();
  Int         iStride     = pcPicYuvRec->getCStride();
  Pel*        piSrcCb     = pcPicYuvRec->getCbAddr( pcCU->getAddr(), uiAbsZorderIdx );
  Pel*        piSrcCr     = pcPicYuvRec->getCrAddr( pcCU->getAddr(), uiAbsZorderIdx );
  
  Int   iQP = QpUV((Int) pcCU->getQP( uiAbsZorderIdx ));
#if E057_INTRA_PCM
  if(pcCU->getIPCMFlag( uiAbsZorderIdx )) 
  {
    iQP = QpUV(0); 
  }
#endif

  UInt  uiPelsInPartChroma = g_uiMaxCUWidth >> (g_uiMaxCUDepth+1);
  
  Int   iOffset, iSrcStep;
  
  const UInt uiLCUWidthInBaseUnits = pcCU->getPic()->getNumPartInWidth();
  
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  Bool  bPCMFilter = (pcCU->getSlice()->getSPS()->getPCMFilterDisableFlag() && ((1<<pcCU->getSlice()->getSPS()->getPCMLog2MinSize()) <= g_uiMaxCUWidth))? true : false;
  Bool  bPartPNoFilter = false;
  Bool  bPartQNoFilter = false; 
  UInt  uiPartPIdx;
  UInt  uiPartQIdx;
  TComDataCU* pcCUP; 
  TComDataCU* pcCUQ = pcCU;
#endif

  // Vertical Position
  UInt uiEdgeNumInLCUVert = g_auiZscanToRaster[uiAbsZorderIdx]%uiLCUWidthInBaseUnits + iEdge;
  UInt uiEdgeNumInLCUHor = g_auiZscanToRaster[uiAbsZorderIdx]/uiLCUWidthInBaseUnits + iEdge;
  
  if ( ( (uiEdgeNumInLCUVert%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&&(iDir==0) ) || ( (uiEdgeNumInLCUHor%(DEBLOCK_SMALLEST_BLOCK/uiPelsInPartChroma))&& iDir ) )
  {
    return;
  }
  
  UInt  uiNumParts = pcCU->getPic()->getNumPartInWidth()>>uiDepth;
  
  UInt  uiBsAbsIdx;
  UChar ucBs;
  
  Pel* piTmpSrcCb = piSrcCb;
  Pel* piTmpSrcCr = piSrcCr;
  
  
  if (iDir == EDGE_VER)
  {
    iOffset   = 1;
    iSrcStep  = iStride;
    piTmpSrcCb += iEdge*uiPelsInPartChroma;
    piTmpSrcCr += iEdge*uiPelsInPartChroma;
  }
  else  // (iDir == EDGE_HOR)
  {
    iOffset   = iStride;
    iSrcStep  = 1;
    piTmpSrcCb += iEdge*iStride*uiPelsInPartChroma;
    piTmpSrcCr += iEdge*iStride*uiPelsInPartChroma;
  }
  
  for ( UInt iIdx = 0; iIdx < uiNumParts; iIdx++ )
  {
    ucBs = 0;
    
    uiBsAbsIdx = xCalcBsIdx( pcCU, uiAbsZorderIdx, iDir, iEdge, iIdx);
    ucBs = m_aapucBS[iDir][0][uiBsAbsIdx];
    
    Int iBitdepthScale = (1<<(g_uiBitIncrement+g_uiBitDepth-8));
    
    Int uiTcOffset = ( ucBs > 2 ) ? DEFAULT_INTRA_TC_OFFSET : 0;
    
    Int iIndexTC = Clip3(0, MAX_QP+4, iQP + uiTcOffset );
    
    Int iTc =  tctable_8x8[iIndexTC]*iBitdepthScale;
    
    if ( ucBs > 2)
    {
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
      if(bPCMFilter)
      {
        // Derive current PU index
        uiPartQIdx = xCalcBsIdx(pcCUQ, uiAbsZorderIdx, iDir, iEdge, iIdx);

        // Derive neighboring PU index
        if (iDir == EDGE_VER)
        {
          pcCUP = pcCUQ->getPULeft (uiPartPIdx, uiPartQIdx);
        }
        else  // (iDir == EDGE_HOR)
        {
          pcCUP = pcCUQ->getPUAbove(uiPartPIdx, uiPartQIdx);
        }

        // Check if each of PUs is IPCM
        bPartPNoFilter = (pcCUP->getIPCMFlag(uiPartPIdx));
        bPartQNoFilter = (pcCUQ->getIPCMFlag(uiPartQIdx));
      }
#endif

      for ( UInt uiStep = 0; uiStep < uiPelsInPartChroma; uiStep++ )
      {
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
        xPelFilterChroma( piTmpSrcCb + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc , bPartPNoFilter, bPartQNoFilter);
        xPelFilterChroma( piTmpSrcCr + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc , bPartPNoFilter, bPartQNoFilter);
#else
        xPelFilterChroma( piTmpSrcCb + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc );
        xPelFilterChroma( piTmpSrcCr + iSrcStep*(uiStep+iIdx*uiPelsInPartChroma), iOffset, iTc );
#endif
      }
    }
  }
}

#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
/**
 - Decision for one line/column for the luminance component to use strong or weak deblocking
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param d             d value
 \param beta          beta value
 \param tc            tc value

 \returns decision to use strong or weak deblocking
 */
__inline Int TComLoopFilter::xPelFilterLumaDecision( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc)
{
  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  
  if ( ((abs(piSrc[-iOffset*4]-m3) + abs(piSrc[ iOffset*3]-m4)) < (beta>>3)) && (d<(beta>>2)) && ( abs(m3-m4) < ((tc*5+1)>>1)) ) //strong filtering
  {
    return 1;
  }
  else //weak filtering
  {
    return 0;
  }
}


/**
 - Strong deblocking of one line/column for the luminance component
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param m0            sample value
 \param m1            sample value
 \param m2            sample value
 \param m3            sample value
 \param m4            sample value
 \param m5            sample value
 \param m6            sample value
 \param m7            sample value
 */
__inline Void TComLoopFilter::xPelFilterLumaStrong(Pel* piSrc, Int iOffset, Pel m0, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6, Pel m7)
{
  piSrc[-iOffset] = Clip(( m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3 );
  piSrc[0] = Clip(( m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3 );

  piSrc[-iOffset*2] = Clip(( m1 + m2 + m3 + m4 + 2)>>2);
  piSrc[ iOffset] = Clip(( m3 + m4 + m5 + m6 + 2)>>2);

  piSrc[-iOffset*3] = Clip(( 2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3);
  piSrc[ iOffset*2] = Clip(( m3 + m4 + m5 + 3*m6 + 2*m7 +4 )>>3);  
}

/**
 - Weak deblocking of one line/column for the luminance component
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param tc            tc value
 \param m1            sample value
 \param m2            sample value
 \param m3            sample value
 \param m4            sample value
 \param m5            sample value
 \param m6            sample value
 */
#if F118_LUMA_DEBLOCK
__inline Void TComLoopFilter::xPelFilterLumaWeak(Pel* piSrc, Int iOffset, Int tc, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ)
#else
__inline Void TComLoopFilter::xPelFilterLumaWeak(Pel* piSrc, Int iOffset, Int tc, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6)
#endif
{
#if F118_LUMA_DEBLOCK
  Int delta = (9*(m4-m3) -3*(m5-m2) + 8)>>4 ;

  if ( abs(delta) < iThrCut )
  {
    delta = Clip3(-tc, tc, delta);        
    piSrc[-iOffset] = Clip((m3+delta));
    piSrc[0] = Clip((m4-delta));

    Int tc2 = tc>>1;
    if(bFilterSecondP)
    {
      Int delta1 = Clip3(-tc2, tc2, (( ((m1+m3+1)>>1)- m2+delta)>>1));
      piSrc[-iOffset*2] = Clip((m2+delta1));
    }
    if(bFilterSecondQ)
    {
      Int delta2 = Clip3(-tc2, tc2, (( ((m6+m4+1)>>1)- m5-delta)>>1));
      piSrc[ iOffset] = Clip((m5+delta2));
    }
  }
#else
  Int delta = Clip3(-tc, tc, ((13*(m4-m3) + 4*(m5-m2) - 5*(m6-m1)+16)>>5) );

  piSrc[-iOffset] = Clip(m3+delta);
  piSrc[0] = Clip(m4-delta);
  piSrc[-iOffset*2] = Clip(m2+delta/2);
  piSrc[ iOffset] = Clip(m5-delta/2);
#endif
}
#endif


#if PARALLEL_MERGED_DEBLK
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
/**
 - Deblocking for the luminance component with strong or weak filter
 .
 \param piSrcJudge    pointer to picture data for decision
 \param bPartPNoFilter  indicator to disable filtering on partP
 \param bPartQNoFilter  indicator to disable filtering on partQ
*/
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc , Pel* piSrcJudge, Bool bPartPNoFilter, Bool bPartQNoFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ)
#else
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc , Pel* piSrcJudge, Bool bPartPNoFilter, Bool bPartQNoFilter)
#endif
#else
#if F118_LUMA_DEBLOCK
/**
 - Deblocking for the luminance component with strong or weak filter
 .
 \param piSrcJudge    pointer to picture data for decision
*/
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc , Pel* piSrcJudge, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ)
#else
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc , Pel* piSrcJudge)
#endif
#endif
#else
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc , Bool bPartPNoFilter, Bool bPartQNoFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ)
#else
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc , Bool bPartPNoFilter, Bool bPartQNoFilter)
#endif
#else
#if F118_LUMA_DEBLOCK
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ)
#else
__inline Void TComLoopFilter::xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc )
#endif
#endif
#endif
{
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  Int d_strong;
#else  
  Int d_strong, delta;
#endif
  
  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  Pel m5  = piSrc[ iOffset];
  Pel m2  = piSrc[-iOffset*2];
  Pel m6  = piSrc[ iOffset*2];
  Pel m1  = piSrc[-iOffset*3];
  Pel m7  = piSrc[ iOffset*3];
  Pel m0  = piSrc[-iOffset*4];
#if PARALLEL_MERGED_DEBLK
  Pel m4j  = piSrcJudge[0];
  Pel m3j  = piSrcJudge[-iOffset];
  Pel m7j  = piSrcJudge[ iOffset*3];
  Pel m0j  = piSrcJudge[-iOffset*4];

  d_strong = abs(m0j-m3j) + abs(m7j-m4j);

  if ( (d_strong < (beta>>3)) && (d<(beta>>2)) && ( abs(m3j-m4j) < ((tc*5+1)>>1)) ) //strong filtering
#else
  d_strong = abs(m0-m3) + abs(m7-m4);
  
  if ( (d_strong < (beta>>3)) && (d<(beta>>2)) && ( abs(m3-m4) < ((tc*5+1)>>1)) ) //strong filtering
#endif
  {
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
    xPelFilterLumaStrong(piSrc, iOffset, m0, m1, m2, m3, m4, m5, m6, m7);
#else  
    piSrc[-iOffset] = Clip(( m1 + 2*m2 + 2*m3 + 2*m4 + m5 + 4) >> 3 );
    piSrc[0] = Clip(( m2 + 2*m3 + 2*m4 + 2*m5 + m6 + 4) >> 3 );
    
    piSrc[-iOffset*2] = Clip(( m1 + m2 + m3 + m4 + 2)>>2);
    piSrc[ iOffset] = Clip(( m3 + m4 + m5 + m6 + 2)>>2);
    
    piSrc[-iOffset*3] = Clip(( 2*m0 + 3*m1 + m2 + m3 + m4 + 4 )>>3);
    piSrc[ iOffset*2] = Clip(( m3 + m4 + m5 + 3*m6 + 2*m7 +4 )>>3);
#endif
  }
  else
  {
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
#if F118_LUMA_DEBLOCK
    xPelFilterLumaWeak(piSrc, iOffset, tc, m1, m2, m3, m4, m5, m6, iThrCut, bFilterSecondP, bFilterSecondQ);
#else
    xPelFilterLumaWeak(piSrc, iOffset, tc, m1, m2, m3, m4, m5, m6);
#endif
#else    
    /* Weak filter */
#if F118_LUMA_DEBLOCK
    delta = (9*(m4-m3) -3*(m5-m2) + 8)>>4 ;

    if ( abs(delta) < iThrCut )
    {
      delta = Clip3(-tc, tc, delta);        
      piSrc[-iOffset] = Clip((m3+delta));
      piSrc[0] = Clip((m4-delta));

      Int tc2 = tc>>1;
      if(bFilterSecondP)
      {
        Int delta1 = Clip3(-tc2, tc2, (( ((m1+m3+1)>>1)- m2+delta)>>1));
        piSrc[-iOffset*2] = Clip((m2+delta1));
      }
      if(bFilterSecondQ)
      {
        Int delta2 = Clip3(-tc2, tc2, (( ((m6+m4+1)>>1)- m5-delta)>>1));
        piSrc[ iOffset] = Clip((m5+delta2));
      }
    }
#else
    delta = Clip3(-tc, tc, ((13*(m4-m3) + 4*(m5-m2) - 5*(m6-m1)+16)>>5) );
    
    piSrc[-iOffset] = Clip(m3+delta);
    piSrc[0] = Clip(m4-delta);
    piSrc[-iOffset*2] = Clip(m2+delta/2);
    piSrc[ iOffset] = Clip(m5-delta/2);
#endif
#endif
  }

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  if(bPartPNoFilter)
  {
    piSrc[-iOffset] = m3;
    piSrc[-iOffset*2] = m2;
    piSrc[-iOffset*3] = m1;
  }
  if(bPartQNoFilter)
  {
    piSrc[0] = m4;
    piSrc[ iOffset] = m5;
    piSrc[ iOffset*2] = m6;
  }
#endif
}

#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK

/**
 - Deblocking of one line/column for the luminance component
 .
 \param piSrc           pointer to picture data
 \param iOffset         offset value for picture data
 \param tc              tc value
 \param strongFilter    indicator to use either strong or weak filter   
 \param bPartPNoFilter  indicator to disable filtering on partP
 \param bPartQNoFilter  indicator to disable filtering on partQ
 */
__inline Void TComLoopFilter::xPelFilterLumaExecution( Pel* piSrc, Int iOffset, Int tc, Int strongFilter, Bool bPartPNoFilter, Bool bPartQNoFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ)
#else
__inline Void TComLoopFilter::xPelFilterLumaExecution( Pel* piSrc, Int iOffset, Int tc, Int strongFilter, Bool bPartPNoFilter, Bool bPartQNoFilter)
#endif
#else
#if F118_LUMA_DEBLOCK
/**
 - Deblocking of one line/column for the luminance component
 .
 \param piSrc         pointer to picture data
 \param iOffset       offset value for picture data
 \param tc            tc value
 \param strongFilter  indicator to use either strong or weak filter      
 */
__inline Void TComLoopFilter::xPelFilterLumaExecution( Pel* piSrc, Int iOffset, Int tc, Int strongFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ)
#else
__inline Void TComLoopFilter::xPelFilterLumaExecution( Pel* piSrc, Int iOffset, Int tc, Int strongFilter)
#endif
#endif
{
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  Pel m5  = piSrc[ iOffset];
  Pel m2  = piSrc[-iOffset*2];
  Pel m6  = piSrc[ iOffset*2];
  Pel m1  = piSrc[-iOffset*3];
#endif

  if (strongFilter) //strong filtering
  {
    xPelFilterLumaStrong(piSrc, iOffset, piSrc[-iOffset*4], piSrc[-iOffset*3], piSrc[-iOffset*2], piSrc[-iOffset], piSrc[0], piSrc[ iOffset], piSrc[ iOffset*2], piSrc[ iOffset*3]);
  }
  else //weak filtering
  {
#if F118_LUMA_DEBLOCK
    xPelFilterLumaWeak(piSrc, iOffset, tc, piSrc[-iOffset*3], piSrc[-iOffset*2], piSrc[-iOffset], piSrc[0], piSrc[ iOffset], piSrc[ iOffset*2], iThrCut, bFilterSecondP, bFilterSecondQ);
#else
    xPelFilterLumaWeak(piSrc, iOffset, tc, piSrc[-iOffset*3], piSrc[-iOffset*2], piSrc[-iOffset], piSrc[0], piSrc[ iOffset], piSrc[ iOffset*2]);
#endif
  }

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  if(bPartPNoFilter)
  {
    piSrc[-iOffset] = m3;
    piSrc[-iOffset*2] = m2;
    piSrc[-iOffset*3] = m1;
  }
  if(bPartQNoFilter)
  {
    piSrc[0] = m4;
    piSrc[ iOffset] = m5;
    piSrc[ iOffset*2] = m6;
  }
#endif
}
#endif

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
/**
 - Deblocking of one line/column for the chrominance component
 .
 \param piSrc           pointer to picture data
 \param iOffset         offset value for picture data
 \param tc              tc value
 \param bPartPNoFilter  indicator to disable filtering on partP
 \param bPartQNoFilter  indicator to disable filtering on partQ
 */
__inline Void TComLoopFilter::xPelFilterChroma( Pel* piSrc, Int iOffset, Int tc, Bool bPartPNoFilter, Bool bPartQNoFilter)
#else
__inline Void TComLoopFilter::xPelFilterChroma( Pel* piSrc, Int iOffset, Int tc )
#endif
{
  int delta;
  
  Pel m4  = piSrc[0];
  Pel m3  = piSrc[-iOffset];
  Pel m5  = piSrc[ iOffset];
  Pel m2  = piSrc[-iOffset*2];
  
  delta = Clip3(-tc,tc, (((( m4 - m3 ) << 2 ) + m2 - m5 + 4 ) >> 3) );
  piSrc[-iOffset] = Clip(m3+delta);
  piSrc[0] = Clip(m4-delta);

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  if(bPartPNoFilter)
  {
    piSrc[-iOffset] = m3;
  }
  if(bPartQNoFilter)
  {
    piSrc[0] = m4;
  }
#endif
}


#if F118_LUMA_DEBLOCK
__inline Int TComLoopFilter::xCalcDP( Pel* piSrc, Int iOffset)
{
  return abs( piSrc[-iOffset*3] - 2*piSrc[-iOffset*2] + piSrc[-iOffset] ) ;
}
__inline Int TComLoopFilter::xCalcDQ( Pel* piSrc, Int iOffset)
{
  return abs( piSrc[0] - 2*piSrc[iOffset] + piSrc[iOffset*2] );
}
#else
__inline Int TComLoopFilter::xCalcD( Pel* piSrc, Int iOffset)
{
  return abs( piSrc[-iOffset*3] - 2*piSrc[-iOffset*2] + piSrc[-iOffset] ) + abs( piSrc[0] - 2*piSrc[iOffset] + piSrc[iOffset*2] );
}
#endif
//! \}
