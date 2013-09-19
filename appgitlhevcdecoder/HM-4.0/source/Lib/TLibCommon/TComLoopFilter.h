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

/** \file     TComLoopFilter.h
    \brief    deblocking filter (header)
*/

#ifndef __TCOMLOOPFILTER__
#define __TCOMLOOPFILTER__

#include "CommonDef.h"
#include "TComPic.h"

//! \ingroup TLibCommon
//! \{

#define DEBLOCK_SMALLEST_BLOCK  8

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// deblocking filter class
class TComLoopFilter
{
private:
  UInt      m_uiDisableDeblockingFilterIdc; ///< deblocking filter idc
  UInt      m_uiNumPartitions;
  UChar*    m_aapucBS[2][3];              ///< Bs for [Ver/Hor][Y/U/V][Blk_Idx]
  Bool*     m_aapbEdgeFilter[2][3];
  LFCUParam m_stLFCUParam;                  ///< status structure
  
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  UInt m_decisions_D     [MAX_CU_SIZE/DEBLOCK_SMALLEST_BLOCK][MAX_CU_SIZE/DEBLOCK_SMALLEST_BLOCK];
  UInt m_decisions_Sample[MAX_CU_SIZE/DEBLOCK_SMALLEST_BLOCK][MAX_CU_SIZE];
#if F118_LUMA_DEBLOCK
  UInt m_decisions_DP     [MAX_CU_SIZE/DEBLOCK_SMALLEST_BLOCK][MAX_CU_SIZE/DEBLOCK_SMALLEST_BLOCK];
  UInt m_decisions_DQ     [MAX_CU_SIZE/DEBLOCK_SMALLEST_BLOCK][MAX_CU_SIZE/DEBLOCK_SMALLEST_BLOCK];
#endif
#endif
#if PARALLEL_MERGED_DEBLK
  TComPicYuv m_preDeblockPic;
#endif
  
protected:
  /// CU-level deblocking function
#if PARALLEL_MERGED_DEBLK
  Void xDeblockCU                 ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int Edge );
#else
  Void xDeblockCU                 ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth );
#endif

  // set / get functions
  Void xSetLoopfilterParam        ( TComDataCU* pcCU, UInt uiAbsZorderIdx );
  // filtering functions
#if NSQT
  Void xSetEdgefilterTU           ( TComDataCU* pcCU, UInt uiRasterIdx, UInt uiAbsZorderIdx, UInt uiDepth );
#else
  Void xSetEdgefilterTU           ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth );
#endif
  Void xSetEdgefilterPU           ( TComDataCU* pcCU, UInt uiAbsZorderIdx );
  Void xGetBoundaryStrengthSingle ( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, UInt uiPartIdx );
#if NSQT
  UInt xCalcBsIdx                 ( TComDataCU* pcCU, UInt uiScanorderIdx, Int iDir, Int iEdgeIdx, Int iBaseUnitIdx, Bool bUseZScan = true )
  {
    TComPic* const pcPic = pcCU->getPic();
    const UInt uiLCUWidthInBaseUnits = pcPic->getNumPartInWidth();
    if( iDir == 0 && bUseZScan )
      return g_auiRasterToZscan[g_auiZscanToRaster[uiScanorderIdx] + iBaseUnitIdx * uiLCUWidthInBaseUnits + iEdgeIdx ];
    else if( iDir == 1 && bUseZScan )
      return g_auiRasterToZscan[g_auiZscanToRaster[uiScanorderIdx] + iEdgeIdx * uiLCUWidthInBaseUnits + iBaseUnitIdx ];
    else if( iDir == 0 && !bUseZScan )
      return g_auiRasterToZscan[uiScanorderIdx + iBaseUnitIdx * uiLCUWidthInBaseUnits + iEdgeIdx ];
    else
      return g_auiRasterToZscan[uiScanorderIdx + iEdgeIdx * uiLCUWidthInBaseUnits + iBaseUnitIdx ];
  }
#else
  UInt xCalcBsIdx                 ( TComDataCU* pcCU, UInt uiAbsZorderIdx, Int iDir, Int iEdgeIdx, Int iBaseUnitIdx )
  {
    TComPic* const pcPic = pcCU->getPic();
    const UInt uiLCUWidthInBaseUnits = pcPic->getNumPartInWidth();
    if( iDir == 0 )
      return g_auiRasterToZscan[g_auiZscanToRaster[uiAbsZorderIdx] + iBaseUnitIdx * uiLCUWidthInBaseUnits + iEdgeIdx ];
    else
      return g_auiRasterToZscan[g_auiZscanToRaster[uiAbsZorderIdx] + iEdgeIdx * uiLCUWidthInBaseUnits + iBaseUnitIdx ];
  }
#endif  
  
#if NSQT  
  Void xSetEdgefilterMultiple( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdgeIdx, Bool bValue ,UInt uiWidthInBaseUnits = 0, UInt uiHeightInBaseUnits = 0 );
#else
  Void xSetEdgefilterMultiple( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdgeIdx, Bool bValue );
#endif
  
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  Void xEdgeFilterLuma            ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge, Int iDecideExecute);
#else
  Void xEdgeFilterLuma            ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge );
#endif
  Void xEdgeFilterChroma          ( TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, Int iDir, Int iEdge );
  
#if (PARALLEL_DEBLK_DECISION && !PARALLEL_MERGED_DEBLK)
  __inline Void xPelFilterLumaStrong    ( Pel* piSrc, Int iOffset, Pel m0, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6, Pel m7);
#if F118_LUMA_DEBLOCK
  __inline Void xPelFilterLumaWeak      ( Pel* piSrc, Int iOffset, Int tc, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ);
#else
  __inline Void xPelFilterLumaWeak      ( Pel* piSrc, Int iOffset, Int tc, Pel m1, Pel m2, Pel m3, Pel m4, Pel m5, Pel m6);
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
  __inline Void xPelFilterLumaExecution ( Pel* piSrc, Int iOffset, Int tc, Int strongFilter, Bool bPartPNoFilter, Bool bPartQNoFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ);
#else
  __inline Void xPelFilterLumaExecution ( Pel* piSrc, Int iOffset, Int tc, Int strongFilter, Bool bPartPNoFilter, Bool bPartQNoFilter);
#endif
#else
#if F118_LUMA_DEBLOCK
  __inline Void xPelFilterLumaExecution ( Pel* piSrc, Int iOffset, Int tc, Int strongFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ);
#else
  __inline Void xPelFilterLumaExecution ( Pel* piSrc, Int iOffset, Int tc, Int strongFilter);
#endif
#endif
  __inline Int  xPelFilterLumaDecision  ( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc);
#endif
#if PARALLEL_MERGED_DEBLK
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Pel* piSrcJudge, Bool bPartPNoFilter, Bool bPartQNoFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ);
#else
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Pel* piSrcJudge, Bool bPartPNoFilter, Bool bPartQNoFilter);
#endif
#else
#if F118_LUMA_DEBLOCK
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Pel* piSrcJudge, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ);
#else
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Pel* piSrcJudge);
#endif
#endif
#else
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
#if F118_LUMA_DEBLOCK
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Bool bPartPNoFilter, Bool bPartQNoFilter, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ);
#else
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Bool bPartPNoFilter, Bool bPartQNoFilter);
#endif
#else
#if F118_LUMA_DEBLOCK
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc, Int iThrCut, Bool bFilterSecondP, Bool bFilterSecondQ );
#else
  __inline Void xPelFilterLuma( Pel* piSrc, Int iOffset, Int d, Int beta, Int tc );
#endif
#endif
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  __inline Void xPelFilterChroma( Pel* piSrc, Int iOffset, Int tc, Bool bPartPNoFilter, Bool bPartQNoFilter);
#else
  __inline Void xPelFilterChroma( Pel* piSrc, Int iOffset, Int tc );
#endif
#if F118_LUMA_DEBLOCK
  __inline Int xCalcDP( Pel* piSrc, Int iOffset);
  __inline Int xCalcDQ( Pel* piSrc, Int iOffset);
#else
  __inline Int xCalcD( Pel* piSrc, Int iOffset);
#endif
  
public:
  TComLoopFilter();
  virtual ~TComLoopFilter();
  
#if PARALLEL_MERGED_DEBLK
  Void  create                    ( Int width, Int height, Int maxCUWidth, Int maxCUHeight, Int uiMaxCUDepth );
#else
  Void  create                    ( UInt uiMaxCUDepth );
#endif
  Void  destroy                   ();
  
  /// set configuration
  Void setCfg( UInt uiDisableDblkIdc, Int iAlphaOffset, Int iBetaOffset );
  
  /// picture-level deblocking filter
  Void loopFilterPic( TComPic* pcPic );
};

//! \}

#endif
