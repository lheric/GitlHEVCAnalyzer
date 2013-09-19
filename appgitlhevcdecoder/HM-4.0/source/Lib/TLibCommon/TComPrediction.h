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

/** \file     TComPrediction.h
    \brief    prediction class (header)
*/

#ifndef __TCOMPREDICTION__
#define __TCOMPREDICTION__


// Include files
#include "TComPic.h"
#include "TComMotionInfo.h"
#include "TComPattern.h"
#include "TComTrQuant.h"
#if GENERIC_IF
#include "TComInterpolationFilter.h"
#else
#include "TComPredFilter.h"
#endif

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// prediction class
#if GENERIC_IF
class TComPrediction
#else
class TComPrediction : public TComPredFilter
#endif
{
protected:
  Int*      m_piYuvExt;
  Int       m_iYuvExtStride;
  Int       m_iYuvExtHeight;
  
  TComYuv   m_acYuvPred[2];
  TComYuv   m_cYuvPredTemp;
#if !GENERIC_IF
  TComYuv   m_cYuvExt;
#else
  TComYuv m_filteredBlock[4][4];
  TComYuv m_filteredBlockTmp[4];
  
  TComInterpolationFilter m_if;
#endif
  
#if LM_CHROMA
  Pel*   m_pLumaRecBuffer;       ///< array for downsampled reconstructed luma sample 
  Int    m_iLumaRecStride;       ///< stride of #m_pLumaRecBuffer array
#if LM_CHROMA_SIMPLIFICATION
  UInt   m_uiaShift[ 63 ];       // Table for multiplication to substitue of division operation
#else
  UInt   m_uiaShift[ 65 ];       // Table for multiplication to substitue of division operation
#endif
#endif

  Void xPredIntraAng            ( Int* pSrc, Int srcStride, Pel*& rpDst, Int dstStride, UInt width, UInt height, UInt dirMode, Bool blkAboveAvailable, Bool blkLeftAvailable );
#if ADD_PLANAR_MODE
#if REFERENCE_SAMPLE_PADDING
  Void xPredIntraPlanar         ( Int* pSrc, Int srcStride, Pel* rpDst, Int dstStride, UInt width, UInt height );
#else
  Void xPredIntraPlanar         ( Int* pSrc, Int srcStride, Pel* rpDst, Int dstStride, UInt width, UInt height, Bool blkAboveAvailable, Bool blkLeftAvailable );
#endif
#endif
  
  // motion compensation functions
  Void xPredInterUni            ( TComDataCU* pcCU,                          UInt uiPartAddr,               Int iWidth, Int iHeight, RefPicList eRefPicList, TComYuv*& rpcYuvPred, Int iPartIdx, Bool bi=false          );
  Void xPredInterBi             ( TComDataCU* pcCU,                          UInt uiPartAddr,               Int iWidth, Int iHeight,                         TComYuv*& rpcYuvPred, Int iPartIdx          );
#if GENERIC_IF
  Void xPredInterLumaBlk  ( TComDataCU *cu, TComPicYuv *refPic, UInt partAddr, TComMv *mv, Int width, Int height, TComYuv *&dstPic, Bool bi );
  Void xPredInterChromaBlk( TComDataCU *cu, TComPicYuv *refPic, UInt partAddr, TComMv *mv, Int width, Int height, TComYuv *&dstPic, Bool bi );
#else
  Void xPredInterLumaBlk        ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight,                         TComYuv*& rpcYuv );
  Void xPredInterChromaBlk      ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight,                         TComYuv*& rpcYuv                            );
#endif
  Void xWeightedAverage         ( TComDataCU* pcCU, TComYuv* pcYuvSrc0, TComYuv* pcYuvSrc1, Int iRefIdx0, Int iRefIdx1, UInt uiPartAddr, Int iWidth, Int iHeight, TComYuv*& rpcYuvDst );
#if !GENERIC_IF
  Void xDCTIF_FilterC ( Pel*  piRefC, Int iRefStride,Pel*  piDstC,Int iDstStride,Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac);

  Void xPredInterLumaBlk_ha        ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight,                         TComYuv*& rpcYuv );
  Void xPredInterChromaBlk_ha      ( TComDataCU* pcCU, TComPicYuv* pcPicYuvRef, UInt uiPartAddr, TComMv* pcMv, Int iWidth, Int iHeight,                         TComYuv*& rpcYuv                            );
  Void xDCTIF_FilterC_ha ( Pel*  piRefC, Int iRefStride,Pel*  piDstC,Int iDstStride,Int iWidth, Int iHeight,Int iMVyFrac,Int iMVxFrac);
#endif
  
#if LM_CHROMA
  Void xGetLLSPrediction ( TComPattern* pcPattern, Int* pSrc0, Int iSrcStride, Pel* pDst0, Int iDstStride, UInt uiWidth, UInt uiHeight, UInt uiExt0 );
#endif

#if MN_DC_PRED_FILTER
  Void xDCPredFiltering( Int* pSrc, Int iSrcStride, Pel*& rpDst, Int iDstStride, Int iWidth, Int iHeight );
#endif

public:
  TComPrediction();
  virtual ~TComPrediction();
  
  Void    initTempBuff();
  
  // inter
  Void motionCompensation         ( TComDataCU*  pcCU, TComYuv* pcYuvPred, RefPicList eRefPicList = REF_PIC_LIST_X, Int iPartIdx = -1 );
  
  // motion vector prediction
  Void getMvPredAMVP              ( TComDataCU* pcCU, UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, TComMv& rcMvPred );
  
  // Angular Intra
  Void predIntraLumaAng           ( TComPattern* pcTComPattern, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight,  TComDataCU* pcCU, Bool bAbove, Bool bLeft );
  Void predIntraChromaAng         ( TComPattern* pcTComPattern, Int* piSrc, UInt uiDirMode, Pel* piPred, UInt uiStride, Int iWidth, Int iHeight, TComDataCU* pcCU, Bool bAbove, Bool bLeft );
  
  Pel  predIntraGetPredValDC      ( Int* pSrc, Int iSrcStride, UInt iWidth, UInt iHeight, Bool bAbove, Bool bLeft );
  
  Int* getPredicBuf()             { return m_piYuvExt;      }
  Int  getPredicBufWidth()        { return m_iYuvExtStride; }
  Int  getPredicBufHeight()       { return m_iYuvExtHeight; }

#if LM_CHROMA
  Void predLMIntraChroma( TComPattern* pcPattern, Int* piSrc, Pel* pPred, UInt uiPredStride, UInt uiCWidth, UInt uiCHeight, UInt uiChromaId );
  Void getLumaRecPixels  ( TComPattern* pcPattern, UInt uiWidth0, UInt uiHeight0 );
#endif

};

//! \}

#endif // __TCOMPREDICTION__
