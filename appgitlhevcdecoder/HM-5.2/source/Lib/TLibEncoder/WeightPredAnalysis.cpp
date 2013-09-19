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

/** \file     WeightedPredAnalysis.cpp
    \brief    encoder class
*/

#include "../TLibCommon/TypeDef.h"
#include "../TLibCommon/TComSlice.h"
#include "../TLibCommon/TComPic.h"
#include "../TLibCommon/TComPicYuv.h"
#include "WeightPredAnalysis.h"

#define ABS(a)    ((a) < 0 ? - (a) : (a))
#define DTHRESH (0.99)

WeightPredAnalysis::WeightPredAnalysis()
{
  m_weighted_pred_flag = 0;
  m_weighted_bipred_idc = 0;

  for ( Int iList =0 ; iList<2 ; iList++ )
  {
    for ( Int iRefIdx=0 ; iRefIdx<MAX_NUM_REF ; iRefIdx++ ) 
    {
      for ( int comp=0 ; comp<3 ;comp++ ) 
      {
        wpScalingParam  *pwp   = &(m_wp[iList][iRefIdx][comp]);
        pwp->bPresentFlag      = false;
        pwp->uiLog2WeightDenom = 0;
        pwp->iWeight           = 1;
        pwp->iOffset           = 0;
      }
    }
  }
}

/** calculate AC and DC values for current original image
 * \param TComSlice *slice
 * \returns Void
 */
Bool  WeightPredAnalysis::xCalcACDCParamSlice(TComSlice *slice)
{
  //===== calculate AC/DC value =====
  TComPicYuv*   pPic = slice->getPic()->getPicYuvOrg();
  Int   iSample  = 0;

  // calculate DC/AC value for Y
  Pel*  pOrg    = pPic->getLumaAddr();
  Int64  iOrgDCY = xCalcDCValueSlice(slice, pOrg, &iSample);
  Int64  iOrgNormDCY = ((iOrgDCY+(iSample>>1)) / iSample);
  pOrg = pPic->getLumaAddr();
  Int64  iOrgACY  = xCalcACValueSlice(slice, pOrg, iOrgNormDCY);

  // calculate DC/AC value for Cb
  pOrg = pPic->getCbAddr();
  Int64  iOrgDCCb = xCalcDCValueUVSlice(slice, pOrg, &iSample);
  Int64  iOrgNormDCCb = ((iOrgDCCb+(iSample>>1)) / (iSample));
  pOrg = pPic->getCbAddr();
  Int64  iOrgACCb  = xCalcACValueUVSlice(slice, pOrg, iOrgNormDCCb);

  // calculate DC/AC value for Cr
  pOrg = pPic->getCrAddr();
  Int64  iOrgDCCr = xCalcDCValueUVSlice(slice, pOrg, &iSample);
  Int64  iOrgNormDCCr = ((iOrgDCCr+(iSample>>1)) / (iSample));
  pOrg = pPic->getCrAddr();
  Int64  iOrgACCr  = xCalcACValueUVSlice(slice, pOrg, iOrgNormDCCr);

  wpACDCParam weightACDCParam[3];
  weightACDCParam[0].iAC = iOrgACY;
  weightACDCParam[0].iDC = iOrgNormDCY;
  weightACDCParam[1].iAC = iOrgACCb;
  weightACDCParam[1].iDC = iOrgNormDCCb;
  weightACDCParam[2].iAC = iOrgACCr;
  weightACDCParam[2].iDC = iOrgNormDCCr;

  slice->setWpAcDcParam(weightACDCParam);
  return (true);
}

/** store weighted_pred_flag and weighted_bipred_idc values
 * \param weighted_pred_flag
 * \param weighted_bipred_idc
 * \returns Void
 */
Void  WeightPredAnalysis::xStoreWPparam(Bool weighted_pred_flag, Int weighted_bipred_idc)
{
  m_weighted_pred_flag = weighted_pred_flag;
  m_weighted_bipred_idc = weighted_bipred_idc;
}

/** restore weighted_pred_flag and weighted_bipred_idc values
 * \param TComSlice *slice
 * \returns Void
 */
Void  WeightPredAnalysis::xRestoreWPparam(TComSlice *slice)
{
  slice->getPPS()->setUseWP(m_weighted_pred_flag);
  slice->getPPS()->setWPBiPredIdc(m_weighted_bipred_idc);
}

/** check weighted pred or non-weighted pred
 * \param TComSlice *slice
 * \returns Void
 */
Void  WeightPredAnalysis::xCheckWPEnable(TComSlice *slice)
{
  Int iPresentCnt = 0;
  for ( Int iList=0 ; iList<2 ; iList++ )
  {
    for ( Int iRefIdx=0 ; iRefIdx<MAX_NUM_REF ; iRefIdx++ ) 
    {
      for ( Int iComp=0 ; iComp<3 ;iComp++ ) 
      {
        wpScalingParam  *pwp = &(m_wp[iList][iRefIdx][iComp]);
        iPresentCnt += (Int)pwp->bPresentFlag;
      }
    }
  }

  if(iPresentCnt==0)
  {
    slice->getPPS()->setUseWP(false);
    slice->getPPS()->setWPBiPredIdc(0);
    for ( Int iList=0 ; iList<2 ; iList++ )
    {
      for ( Int iRefIdx=0 ; iRefIdx<MAX_NUM_REF ; iRefIdx++ ) 
      {
        for ( Int iComp=0 ; iComp<3 ;iComp++ ) 
        {
          wpScalingParam  *pwp = &(m_wp[iList][iRefIdx][iComp]);
          pwp->bPresentFlag      = false;
          pwp->uiLog2WeightDenom = 0;
          pwp->iWeight           = 1;
          pwp->iOffset           = 0;
        }
      }
    }
    slice->setWpScaling( m_wp );
  }
}

/** estimate wp tables for explicit wp
 * \param TComSlice *slice
 * \returns Bool
 */
Bool  WeightPredAnalysis::xEstimateWPParamSlice(TComSlice *slice)
{
  Int iDenom  = 6;
  Int iRealDenom = iDenom + (g_uiBitDepth+g_uiBitIncrement-8);
  Int iRealOffset = ((Int)1<<(iRealDenom-1));

  if(slice->getNumRefIdx(REF_PIC_LIST_0)>3)
  {
    iDenom  = 7;
    iRealDenom = iDenom + (g_uiBitDepth+g_uiBitIncrement-8);
    iRealOffset = ((Int)1<<(iRealDenom-1));
  }
  
  Int iNumPredDir = slice->isInterP() ? 1 : 2;
  for ( Int iRefList = 0; iRefList < iNumPredDir; iRefList++ )
  {
    RefPicList  eRefPicList = ( iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
    for ( Int iRefIdxTemp = 0; iRefIdxTemp < slice->getNumRefIdx(eRefPicList); iRefIdxTemp++ )
    {
      wpACDCParam *CurrWeightACDCParam, *RefWeightACDCParam;
      slice->getWpAcDcParam(CurrWeightACDCParam);
      slice->getRefPic(eRefPicList, iRefIdxTemp)->getSlice(0)->getWpAcDcParam(RefWeightACDCParam);

      for ( Int iComp = 0; iComp < 3; iComp++ )
      {
        // current frame
        Int64 iCurrDC = CurrWeightACDCParam[iComp].iDC;
        Int64 iCurrAC = CurrWeightACDCParam[iComp].iAC;
        // reference frame
        Int64 iRefDC = RefWeightACDCParam[iComp].iDC;
        Int64 iRefAC = RefWeightACDCParam[iComp].iAC;

        // calculating iWeight and iOffset params
        Double dWeight = (iRefAC==0) ? (Double)1.0 : ( (Double)(iCurrAC) / (Double)iRefAC);
        Int iWeight = (Int)( 0.5 + dWeight * (Double)(1<<iDenom) );
        Int iOffset = (Int)( ((iCurrDC<<iDenom) - ((Int64)iWeight * iRefDC) + (Int64)iRealOffset) >> iRealDenom );

        m_wp[iRefList][iRefIdxTemp][iComp].bPresentFlag = true;
        m_wp[iRefList][iRefIdxTemp][iComp].iWeight = (Int)iWeight;
        m_wp[iRefList][iRefIdxTemp][iComp].iOffset = (Int)iOffset;
        m_wp[iRefList][iRefIdxTemp][iComp].uiLog2WeightDenom = (Int)iDenom;
      }
    }
  }

  // selecting whether WP is used, or not
  xSelectWP(slice, m_wp, iDenom);
  
  slice->setWpScaling( m_wp );

  return (true);
}

/** select whether weighted pred enables or not. 
 * \param TComSlice *slice
 * \param wpScalingParam
 * \param iDenom
 * \returns Bool
 */
Bool WeightPredAnalysis::xSelectWP(TComSlice *slice, wpScalingParam weightPredTable[2][MAX_NUM_REF][3], Int iDenom)
{
  TComPicYuv*   pPic = slice->getPic()->getPicYuvOrg();
  Int iWidth  = pPic->getWidth();
  Int iHeight = pPic->getHeight();
  Int iDefaultWeight = ((Int)1<<iDenom);
  Int iNumPredDir = slice->isInterP() ? 1 : 2;

  for ( Int iRefList = 0; iRefList < iNumPredDir; iRefList++ )
  {
    Int64 iSADWP = 0, iSADnoWP = 0;
    RefPicList  eRefPicList = ( iRefList ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
    for ( Int iRefIdxTemp = 0; iRefIdxTemp < slice->getNumRefIdx(eRefPicList); iRefIdxTemp++ )
    {
      Pel*  pOrg    = pPic->getLumaAddr();
      Pel*  pRef    = slice->getRefPic(eRefPicList, iRefIdxTemp)->getPicYuvRec()->getLumaAddr();
      Int   iOrgStride = pPic->getStride();
      Int   iRefStride = slice->getRefPic(eRefPicList, iRefIdxTemp)->getPicYuvRec()->getStride();

      // calculate SAD costs with/without wp for luma
      iSADWP   = this->xCalcSADvalueWP(pOrg, pRef, iWidth, iHeight, iOrgStride, iRefStride, iDenom, weightPredTable[iRefList][iRefIdxTemp][0].iWeight, weightPredTable[iRefList][iRefIdxTemp][0].iOffset);
      iSADnoWP = this->xCalcSADvalueWP(pOrg, pRef, iWidth, iHeight, iOrgStride, iRefStride, iDenom, iDefaultWeight, 0);

      pOrg = pPic->getCbAddr();
      pRef = slice->getRefPic(eRefPicList, iRefIdxTemp)->getPicYuvRec()->getCbAddr();
      iOrgStride = pPic->getCStride();
      iRefStride = slice->getRefPic(eRefPicList, iRefIdxTemp)->getPicYuvRec()->getCStride();

      // calculate SAD costs with/without wp for chroma cb
      iSADWP   += this->xCalcSADvalueWP(pOrg, pRef, iWidth>>1, iHeight>>1, iOrgStride, iRefStride, iDenom, weightPredTable[iRefList][iRefIdxTemp][1].iWeight, weightPredTable[iRefList][iRefIdxTemp][1].iOffset);
      iSADnoWP += this->xCalcSADvalueWP(pOrg, pRef, iWidth>>1, iHeight>>1, iOrgStride, iRefStride, iDenom, iDefaultWeight, 0);

      pOrg = pPic->getCrAddr();
      pRef = slice->getRefPic(eRefPicList, iRefIdxTemp)->getPicYuvRec()->getCrAddr();

      // calculate SAD costs with/without wp for chroma cr
      iSADWP   += this->xCalcSADvalueWP(pOrg, pRef, iWidth>>1, iHeight>>1, iOrgStride, iRefStride, iDenom, weightPredTable[iRefList][iRefIdxTemp][2].iWeight, weightPredTable[iRefList][iRefIdxTemp][2].iOffset);
      iSADnoWP += this->xCalcSADvalueWP(pOrg, pRef, iWidth>>1, iHeight>>1, iOrgStride, iRefStride, iDenom, iDefaultWeight, 0);

      Double dRatio = ((Double)iSADWP / (Double)iSADnoWP);
      if(dRatio >= (Double)DTHRESH)
      {
        for ( Int iComp = 0; iComp < 3; iComp++ )
        {
          weightPredTable[iRefList][iRefIdxTemp][iComp].bPresentFlag = false;
          weightPredTable[iRefList][iRefIdxTemp][iComp].iOffset = (Int)0;
          weightPredTable[iRefList][iRefIdxTemp][iComp].iWeight = (Int)iDefaultWeight;
          weightPredTable[iRefList][iRefIdxTemp][iComp].uiLog2WeightDenom = (Int)iDenom;
        }
      }
    }
  }
  return (true);
}

/** calculate DC value of original image for luma. 
 * \param TComSlice *slice
 * \param Pel *pPel
 * \param Int *iSample
 * \returns Int64
 */
Int64 WeightPredAnalysis::xCalcDCValueSlice(TComSlice *slice, Pel *pPel, Int *iSample)
{
  TComPicYuv* pPic = slice->getPic()->getPicYuvOrg();
  Int iStride = pPic->getStride();

  *iSample = 0;
  Int iWidth  = pPic->getWidth();
  Int iHeight = pPic->getHeight();
  *iSample = iWidth*iHeight;
  Int64 iDC = xCalcDCValue(pPel, iWidth, iHeight, iStride);

  return (iDC);
}

/** calculate AC value of original image for luma. 
 * \param TComSlice *slice
 * \param Pel *pPel
 * \param Int iDC
 * \returns Int64
 */
Int64 WeightPredAnalysis::xCalcACValueSlice(TComSlice *slice, Pel *pPel, Int64 iDC)
{
  TComPicYuv* pPic = slice->getPic()->getPicYuvOrg();
  Int iStride = pPic->getStride();

  Int iWidth  = pPic->getWidth();
  Int iHeight = pPic->getHeight();
  Int64 iAC = xCalcACValue(pPel, iWidth, iHeight, iStride, iDC);

  return (iAC);
}

/** calculate DC value of original image for chroma. 
 * \param TComSlice *slice
 * \param Pel *pPel
 * \param Int *iSample
 * \returns Int64
 */
Int64 WeightPredAnalysis::xCalcDCValueUVSlice(TComSlice *slice, Pel *pPel, Int *iSample)
{
  TComPicYuv* pPic = slice->getPic()->getPicYuvOrg();
  Int iCStride = pPic->getCStride();

  *iSample = 0;
  Int iWidth  = pPic->getWidth()>>1;
  Int iHeight = pPic->getHeight()>>1;
  *iSample = iWidth*iHeight;
  Int64 iDC = xCalcDCValue(pPel, iWidth, iHeight, iCStride);

  return (iDC);
}

/** calculate AC value of original image for chroma. 
 * \param TComSlice *slice
 * \param Pel *pPel
 * \param Int iDC
 * \returns Int64
 */
Int64 WeightPredAnalysis::xCalcACValueUVSlice(TComSlice *slice, Pel *pPel, Int64 iDC)
{
  TComPicYuv* pPic = slice->getPic()->getPicYuvOrg();
  Int iCStride = pPic->getCStride();

  Int iWidth  = pPic->getWidth()>>1;
  Int iHeight = pPic->getHeight()>>1;
  Int64 iAC = xCalcACValue(pPel, iWidth, iHeight, iCStride, iDC);

  return (iAC);
}

/** calculate DC value. 
 * \param Pel *pPel
 * \param Int iWidth
 * \param Int iHeight
 * \param Int iStride
 * \returns Int64
 */
Int64 WeightPredAnalysis::xCalcDCValue(Pel *pPel, Int iWidth, Int iHeight, Int iStride)
{
  Int x, y;
  Int64 iDC = 0;
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iDC += (Int)( pPel[x] );
    }
    pPel += iStride;
  }
  return (iDC);
}

/** calculate AC value. 
 * \param Pel *pPel
 * \param Int iWidth
 * \param Int iHeight
 * \param Int iStride
 * \param Int iDC
 * \returns Int64
 */
Int64 WeightPredAnalysis::xCalcACValue(Pel *pPel, Int iWidth, Int iHeight, Int iStride, Int64 iDC)
{
  Int x, y;
  Int64 iAC = 0;
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iAC += abs( (Int)pPel[x] - (Int)iDC );
    }
    pPel += iStride;
  }
  return (iAC);
}

/** calculate SAD values for both WP version and non-WP version. 
 * \param Pel *pOrgPel
 * \param Pel *pRefPel
 * \param Int iWidth
 * \param Int iHeight
 * \param Int iOrgStride
 * \param Int iRefStride
 * \param Int iDenom
 * \param Int iWeight
 * \param Int iOffset
 * \returns Int64
 */
Int64 WeightPredAnalysis::xCalcSADvalueWP(Pel *pOrgPel, Pel *pRefPel, Int iWidth, Int iHeight, Int iOrgStride, Int iRefStride, Int iDenom, Int iWeight, Int iOffset)
{
  Int x, y;
  Int64 iSAD = 0;
  Int64 iSize   = iWidth*iHeight;
  Int64 iRealDenom = iDenom + (g_uiBitDepth+g_uiBitIncrement-8);
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iSAD += ABS(( ((Int64)pOrgPel[x]<<(Int64)iDenom) - ( (Int64)pRefPel[x] * (Int64)iWeight + ((Int64)iOffset<<iRealDenom) ) ) );
    }
    pOrgPel += iOrgStride;
    pRefPel += iRefStride;
  }
  return (iSAD/iSize);
}


