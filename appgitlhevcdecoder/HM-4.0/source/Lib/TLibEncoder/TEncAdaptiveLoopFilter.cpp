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

/** \file     TEncAdaptiveLoopFilter.cpp
 \brief    estimation part of adaptive loop filter class
 */
#include "TEncAdaptiveLoopFilter.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define ALF_NUM_OF_REDESIGN 3

// ====================================================================================================================
// Tables
// ====================================================================================================================

const Int TEncAdaptiveLoopFilter::m_aiSymmetricArray9x9[81] =
{
   0,  1,  2,  3,  4,  5,  6,  7,  8,
   9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26,
  27, 28, 29, 30, 31, 32, 33, 34, 35,
  36, 37, 38, 39, 40, 39, 38, 37, 36,
  35, 34, 33, 32, 31, 30, 29, 28, 27,
  26, 25, 24, 23, 22, 21, 20, 19, 18,
  17, 16, 15, 14, 13, 12, 11, 10,  9,
   8,  7,  6,  5,  4,  3,  2,  1,  0
};

const Int TEncAdaptiveLoopFilter::m_aiSymmetricArray7x7[49] =
{
  0,  1,  2,  3,  4,  5,  6,
  7,  8,  9, 10, 11, 12, 13,
  14, 15, 16, 17, 18, 19, 20,
  21, 22, 23, 24, 23, 22, 21,
  20, 19, 18, 17, 16, 15, 14,
  13, 12, 11, 10,  9,  8,  7,
  6,  5,  4,  3,  2,  1,  0,
};

const Int TEncAdaptiveLoopFilter::m_aiSymmetricArray5x5[25] =
{
  0,  1,  2,  3,  4,
  5,  6,  7,  8,  9,
  10, 11, 12, 11, 10,
  9,  8,  7,  6,  5,
  4,  3,  2,  1,  0,
};

#if TI_ALF_MAX_VSIZE_7
const Int TEncAdaptiveLoopFilter::m_aiSymmetricArray9x7[63] =
{
   0,  1,  2,  3,  4,  5,  6,  7,  8,
   9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26,
  27, 28, 29, 30, 31, 30, 29, 28, 27,
  26, 25, 24, 23, 22, 21, 20, 19, 18,
  17, 16, 15, 14, 13, 12, 11, 10,  9,
   8,  7,  6,  5,  4,  3,  2,  1,  0
};
#endif

#if MQT_ALF_NPASS
#if TI_ALF_MAX_VSIZE_7
Int TEncAdaptiveLoopFilter::m_aiTapPos9x9_In9x9Sym[21] =
#else
Int TEncAdaptiveLoopFilter::m_aiTapPos9x9_In9x9Sym[22] =
#endif
{
#if TI_ALF_MAX_VSIZE_7
                  0,  1,  2,
              3,  4,  5,  6,  7,
          8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20
#else
                   0,
               1,  2,  3,
           4,  5,  6,  7,  8,
       9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21
#endif
};

Int TEncAdaptiveLoopFilter::m_aiTapPos7x7_In9x9Sym[14] =
{                 
#if TI_ALF_MAX_VSIZE_7
                  1,    
              4,  5,  6,    
          9, 10, 11, 12, 13,    
     16, 17, 18, 19, 20

#else

               2,
           5,  6,  7,
      10, 11, 12, 13, 14,
  17, 18, 19, 20, 21
#endif
};

Int TEncAdaptiveLoopFilter::m_aiTapPos5x5_In9x9Sym[8]  =
{

#if TI_ALF_MAX_VSIZE_7
            5,
       10, 11, 12, 
   17, 18, 19, 20
#else
           6,
      11, 12, 13,
  18, 19, 20, 21

#endif

};

Int* TEncAdaptiveLoopFilter::m_iTapPosTabIn9x9Sym[NO_TEST_FILT] =
{
  m_aiTapPos9x9_In9x9Sym, m_aiTapPos7x7_In9x9Sym, m_aiTapPos5x5_In9x9Sym
};
#endif

#if STAR_CROSS_SHAPES_LUMA
Int TEncAdaptiveLoopFilter::m_aiFilterPosShape0In11x5Sym[10] =
{
   3,    5,    7,
     15,16,17,
  25,26,27,28
};
Int TEncAdaptiveLoopFilter::m_aiFilterPosShape1In11x5Sym[9] =
{
                  5, 
                 16, 
  22,23,24,25,26,27,28
};

Int* TEncAdaptiveLoopFilter::m_iFilterTabIn11x5Sym[NO_TEST_FILT] =
{
  m_aiFilterPosShape0In11x5Sym, m_aiFilterPosShape1In11x5Sym
};
#endif
// ====================================================================================================================
// Constructor / destructor
// ====================================================================================================================

TEncAdaptiveLoopFilter::TEncAdaptiveLoopFilter()
{
  m_ppdAlfCorr = NULL;
#if ALF_CHROMA_NEW_SHAPES
  m_ppdAlfCorrCb = NULL;
  m_ppdAlfCorrCr = NULL;
#endif
  m_pdDoubleAlfCoeff = NULL;
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;
  m_pcBestAlfParam = NULL;
  m_pcTempAlfParam = NULL;
  m_pcPicYuvBest = NULL;
  m_pcPicYuvTmp = NULL;
#if STAR_CROSS_SHAPES_LUMA
  pcAlfParamShape0 = NULL;
  pcAlfParamShape1 = NULL;
  pcPicYuvRecShape0 = NULL;
  pcPicYuvRecShape1 = NULL;
#endif
#if MTK_NONCROSS_INLOOP_FILTER
  m_pcSliceYuvTmp = NULL;
#endif
#if MQT_BA_RA && MQT_ALF_NPASS
  m_aiFilterCoeffSaved = NULL;
#if STAR_CROSS_SHAPES_LUMA
  m_iPreviousFilterShape= NULL;
#endif
#endif

#if E045_SLICE_COMMON_INFO_SHARING
  m_bSharedPPSAlfParamEnabled = false;
#endif

}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

#if MQT_BA_RA && MQT_ALF_NPASS
/** create ALF global buffers
 * \param iALFEncodePassReduction 0: 16-pass encoding, 1: 1-pass encoding, 2: 2-pass encoding
 * This function is used to create the filter buffers to perform time-delay filtering.
 */
Void TEncAdaptiveLoopFilter::createAlfGlobalBuffers(Int iALFEncodePassReduction)
{
  if(iALFEncodePassReduction)
  {
    Int iNumOfBuffer = m_iGOPSize +1;

    for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
    {
      m_aiFilterCoeffSavedMethods[i] = new Int**[iNumOfBuffer];
      for(Int j=0; j< iNumOfBuffer; j++)
      {
        m_aiFilterCoeffSavedMethods[i][j] = new Int*[NO_VAR_BINS];
        for(Int k=0; k< NO_VAR_BINS; k++)
        {
          m_aiFilterCoeffSavedMethods[i][j][k] = new Int[MAX_SQR_FILT_LENGTH];
        }
      }
#if STAR_CROSS_SHAPES_LUMA
      m_iPreviousFilterShapeMethods[i] = new Int[iNumOfBuffer];
#endif
    }

  }
}
/** destroy ALF global buffers
 * This function is used to destroy the filter buffers.
 */

Void TEncAdaptiveLoopFilter::destroyAlfGlobalBuffers()
{
  if(m_iALFEncodePassReduction)
  {
    for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
    {
      for(Int j=0; j< m_iGOPSize+1; j++)
      {
        for(Int k=0; k< NO_VAR_BINS; k++)
        {
          delete[] m_aiFilterCoeffSavedMethods[i][j][k];
        }
        delete[] m_aiFilterCoeffSavedMethods[i][j];
      }
      delete[] m_aiFilterCoeffSavedMethods[i];
#if STAR_CROSS_SHAPES_LUMA
      delete[] m_iPreviousFilterShapeMethods[i];
#endif
    }

  }

}
#endif

/**
 \param pcPic           picture (TComPic) pointer
 \param pcEntropyCoder  entropy coder class
 */
Void TEncAdaptiveLoopFilter::startALFEnc( TComPic* pcPic, TEncEntropy* pcEntropyCoder )
{
  m_pcPic = pcPic;
  m_pcEntropyCoder = pcEntropyCoder;
  
  m_eSliceType = pcPic->getSlice(0)->getSliceType();
  m_iPicNalReferenceIdc = (pcPic->getSlice(0)->isReferenced() ? 1 :0);
  
  m_uiNumSCUInCU = m_pcPic->getNumPartInCU();
  
  xInitParam();
  xCreateTmpAlfCtrlFlags();
  
  Int iWidth = pcPic->getPicYuvOrg()->getWidth();
  Int iHeight = pcPic->getPicYuvOrg()->getHeight();
  
  m_pcPicYuvTmp = new TComPicYuv();
  m_pcPicYuvTmp->createLuma(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  m_pcPicYuvBest = pcPic->getPicYuvPred();
  
  m_pcBestAlfParam = new ALFParam;
  m_pcTempAlfParam = new ALFParam;
  allocALFParam(m_pcBestAlfParam);
  allocALFParam(m_pcTempAlfParam);
#if STAR_CROSS_SHAPES_LUMA
  pcPicYuvRecShape0 = new TComPicYuv();
  pcPicYuvRecShape0->createLuma(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
 
  pcPicYuvRecShape1 = new TComPicYuv();
  pcPicYuvRecShape1->createLuma(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  
  pcAlfParamShape0 = new ALFParam;
  pcAlfParamShape1 = new ALFParam;

  allocALFParam(pcAlfParamShape0);  
  allocALFParam(pcAlfParamShape1);
#endif
  m_im_width = iWidth;
  m_im_height = iHeight;
  
  // init qc_filter
  initMatrix4D_double(&m_EGlobalSym, NO_TEST_FILT,  NO_VAR_BINS, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);
  initMatrix3D_double(&m_yGlobalSym, NO_TEST_FILT, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); 
  initMatrix_int(&m_filterCoeffSymQuant, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); 
  
  m_pixAcc = (double *) calloc(NO_VAR_BINS, sizeof(double));
#if !MQT_BA_RA
  get_mem2Dpel(&m_varImg, m_im_height, m_im_width);
#endif
  get_mem2Dpel(&m_maskImg, m_im_height, m_im_width);
  
  initMatrix_double(&m_E_temp, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);//
  m_y_temp = (double *) calloc(MAX_SQR_FILT_LENGTH, sizeof(double));//
  initMatrix3D_double(&m_E_merged, NO_VAR_BINS, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);//
  initMatrix_double(&m_y_merged, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); //
  m_pixAcc_merged = (double *) calloc(NO_VAR_BINS, sizeof(double));//
  
  m_filterCoeffQuantMod = (int *) calloc(MAX_SQR_FILT_LENGTH, sizeof(int));//
  m_filterCoeff = (double *) calloc(MAX_SQR_FILT_LENGTH, sizeof(double));//
  m_filterCoeffQuant = (int *) calloc(MAX_SQR_FILT_LENGTH, sizeof(int));//
  initMatrix_int(&m_diffFilterCoeffQuant, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);//
  initMatrix_int(&m_FilterCoeffQuantTemp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);//
  
  m_tempALFp = new ALFParam;
  allocALFParam(m_tempALFp);
  m_pcDummyEntropyCoder = m_pcEntropyCoder;

#if MTK_NONCROSS_INLOOP_FILTER
  if( m_bUseNonCrossALF )
  {
    m_pcSliceYuvTmp = new TComPicYuv();
    m_pcSliceYuvTmp->create(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  }
#endif


}

Void TEncAdaptiveLoopFilter::endALFEnc()
{
  xUninitParam();
  xDestroyTmpAlfCtrlFlags();
  
  m_pcPicYuvTmp->destroyLuma();
  delete m_pcPicYuvTmp;
  m_pcPicYuvTmp = NULL;
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;
  
  freeALFParam(m_pcBestAlfParam);
  freeALFParam(m_pcTempAlfParam);
  delete m_pcBestAlfParam;
  delete m_pcTempAlfParam;
#if STAR_CROSS_SHAPES_LUMA
  pcPicYuvRecShape0->destroyLuma();
  delete pcPicYuvRecShape0;
  pcPicYuvRecShape0 = NULL;

  pcPicYuvRecShape1->destroyLuma();
  delete pcPicYuvRecShape1;
  pcPicYuvRecShape1 = NULL;

  freeALFParam(pcAlfParamShape0);
  freeALFParam(pcAlfParamShape1);

  delete pcAlfParamShape0;
  delete pcAlfParamShape1;
#endif
  // delete qc filters
  destroyMatrix4D_double(m_EGlobalSym, NO_TEST_FILT,  NO_VAR_BINS);
  destroyMatrix3D_double(m_yGlobalSym, NO_TEST_FILT);
  destroyMatrix_int(m_filterCoeffSymQuant);
  
  free(m_pixAcc);
#if !MQT_BA_RA
  free_mem2Dpel(m_varImg);
#endif
  free_mem2Dpel(m_maskImg);
  
  destroyMatrix3D_double(m_E_merged, NO_VAR_BINS);
  destroyMatrix_double(m_y_merged);
  destroyMatrix_double(m_E_temp);
  free(m_pixAcc_merged);
  
  free(m_filterCoeffQuantMod);
  free(m_y_temp);
  
  free(m_filterCoeff);
  free(m_filterCoeffQuant);
  destroyMatrix_int(m_diffFilterCoeffQuant);
  destroyMatrix_int(m_FilterCoeffQuantTemp);
  
  freeALFParam(m_tempALFp);
  delete m_tempALFp;

#if MTK_NONCROSS_INLOOP_FILTER
  if(m_bUseNonCrossALF)
  {
    m_pcSliceYuvTmp->destroy();
    delete m_pcSliceYuvTmp;
    m_pcSliceYuvTmp = NULL;
  }
#endif

}

#if ALF_CHROMA_LAMBDA  
/**
 \param pcAlfParam           ALF parameter
 \param dLambdaLuma          luma lambda value for RD cost computation
 \param dLambdaChroma        chroma lambda value for RD cost computation
 \retval ruiDist             distortion
 \retval ruiBits             required bits
 \retval ruiMaxAlfCtrlDepth  optimal partition depth
 */
Void TEncAdaptiveLoopFilter::ALFProcess( ALFParam* pcAlfParam, Double dLambdaLuma, Double dLambdaChroma, UInt64& ruiDist, UInt64& ruiBits, UInt& ruiMaxAlfCtrlDepth )
#else
/**
 \param pcAlfParam           ALF parameter
 \param dLambda              lambda value for RD cost computation
 \retval ruiDist             distortion
 \retval ruiBits             required bits
 \retval ruiMaxAlfCtrlDepth  optimal partition depth
 */
Void TEncAdaptiveLoopFilter::ALFProcess( ALFParam* pcAlfParam, Double dLambda, UInt64& ruiDist, UInt64& ruiBits, UInt& ruiMaxAlfCtrlDepth )
#endif
{
#if !STAR_CROSS_SHAPES_LUMA
  Int tap, num_coef;
  
  // set global variables
  tap         = ALF_MAX_NUM_TAP;
#if TI_ALF_MAX_VSIZE_7
  Int tapV = TComAdaptiveLoopFilter::ALFTapHToTapV(tap);
  num_coef = (tap * tapV + 1) >> 1;
#else
  num_coef    = (tap*tap+1)>>1;
#endif
  num_coef    = num_coef + 1; // DC offset
#endif
  
  // set lambda
#if ALF_CHROMA_LAMBDA  
  m_dLambdaLuma   = dLambdaLuma;
  m_dLambdaChroma = dLambdaChroma;
#else
  m_dLambdaLuma   = dLambda;
  m_dLambdaChroma = dLambda;
#endif

  TComPicYuv* pcPicOrg = m_pcPic->getPicYuvOrg();
  
  // extend image for filtering
  TComPicYuv* pcPicYuvRec    = m_pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvExtRec = m_pcTempPicYuv;
  
  pcPicYuvRec->copyToPic(pcPicYuvExtRec);
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
  {
#endif  
  pcPicYuvExtRec->setBorderExtension( false );
  pcPicYuvExtRec->extendPicBorder   ();
#if MTK_NONCROSS_INLOOP_FILTER
  }
#endif  
 
  // set min cost
  UInt64 uiMinRate = MAX_INT;
  UInt64 uiMinDist = MAX_INT;
  Double dMinCost  = MAX_DOUBLE;
  
  UInt64  uiOrigRate;
  UInt64  uiOrigDist;
  Double  dOrigCost;
  
  // calc original cost
  xCalcRDCost( pcPicOrg, pcPicYuvRec, NULL, uiOrigRate, uiOrigDist, dOrigCost );
  m_pcBestAlfParam->alf_flag = 0;
  m_pcBestAlfParam->cu_control_flag = 0;
  
  // initialize temp_alfps
  m_pcTempAlfParam->alf_flag        = 1;
#if !STAR_CROSS_SHAPES_LUMA
  m_pcTempAlfParam->tap             = tap;
#if TI_ALF_MAX_VSIZE_7
  m_pcTempAlfParam->tapV            = tapV;
#endif
  m_pcTempAlfParam->num_coeff       = num_coef;
#endif
  m_pcTempAlfParam->chroma_idc      = 0;
  m_pcTempAlfParam->cu_control_flag = 0;
  
#if MQT_ALF_NPASS
  setALFEncodingParam(m_pcPic);
#endif

  // adaptive in-loop wiener filtering
  xEncALFLuma_qc( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );
  
  // cu-based filter on/off control
  xCUAdaptiveControl_qc( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );
  
#if !STAR_CROSS_SHAPES_LUMA
  // adaptive tap-length
  xFilterTapDecision_qc( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );
  
  // compute RD cost
  xCalcRDCost( pcPicOrg, pcPicYuvRec, m_pcBestAlfParam, uiMinRate, uiMinDist, dMinCost );
#endif  

  // compare RD cost to non-ALF case
  if( dMinCost < dOrigCost )
  {
    m_pcBestAlfParam->alf_flag = 1;
    
    ruiBits = uiMinRate;
    ruiDist = uiMinDist;
  }
  else
  {
    m_pcBestAlfParam->alf_flag        = 0;
    m_pcBestAlfParam->cu_control_flag = 0;
    
    uiMinRate = uiOrigRate;
    uiMinDist = uiOrigDist;
    dMinCost = dMinCost;
    
    m_pcEntropyCoder->setAlfCtrl(false);
    pcPicYuvExtRec->copyToPicLuma(pcPicYuvRec);
    
    ruiBits = uiOrigRate;
    ruiDist = uiOrigDist;
  }
  // if ALF works
  if( m_pcBestAlfParam->alf_flag )
  {
    // predict ALF coefficients
    predictALFCoeff( m_pcBestAlfParam );
    
    // do additional ALF process for chroma
#if ALF_CHROMA_NEW_SHAPES
    xFilterTapDecisionChroma( uiMinRate, pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, ruiDist, ruiBits );
#else
    xEncALFChroma( uiMinRate, pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, ruiDist, ruiBits );
#endif
  }
  
  // copy to best storage
  copyALFParam(pcAlfParam, m_pcBestAlfParam);
  
  // store best depth
  ruiMaxAlfCtrlDepth = m_pcEntropyCoder->getMaxAlfCtrlDepth();
}

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX
/** PCM LF disable process.
 * \param pcPic picture (TComPic) pointer
 * \returns Void
 *
 * \note Replace filtered sample values of PCM mode blocks with the transmitted and reconstructed ones.
 */
Void TEncAdaptiveLoopFilter::PCMLFDisableProcess (TComPic* pcPic)
{
  xPCMRestoration(pcPic);
}
#endif

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

#if !ALF_CHROMA_NEW_SHAPES
Void TEncAdaptiveLoopFilter::xEncALFChroma( UInt64 uiLumaRate, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, UInt64& ruiBits )
{
  // restriction for non-referenced B-slice
  if (m_eSliceType == B_SLICE && m_iPicNalReferenceIdc == 0)
  {
    return;
  }
  
  Int tap, num_coef;
  
  // set global variables
  tap         = ALF_MAX_NUM_TAP_C;
  num_coef    = (tap*tap+1)>>1;
  num_coef    = num_coef + 1; // DC offset
  
  // set min cost
  UInt64 uiMinRate = uiLumaRate;
  UInt64 uiMinDist = MAX_INT;
  Double dMinCost  = MAX_DOUBLE;
  
  // calc original cost
  copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
  xCalcRDCostChroma(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiMinRate, uiMinDist, dMinCost);
  
  // initialize temp_alfps
  m_pcTempAlfParam->chroma_idc = 3;
  m_pcTempAlfParam->tap_chroma       = tap;
  m_pcTempAlfParam->num_coeff_chroma = num_coef;
  
  // Adaptive in-loop wiener filtering for chroma
  xFilteringFrameChroma(pcPicOrg, pcPicDec, pcPicRest);
  
  // filter on/off decision for chroma
  Int iCWidth = (pcPicOrg->getWidth()>>1);
  Int iCHeight = (pcPicOrg->getHeight()>>1);
  Int iCStride = pcPicOrg->getCStride();
  UInt64 uiFiltDistCb = xCalcSSD(pcPicOrg->getCbAddr(), pcPicRest->getCbAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiFiltDistCr = xCalcSSD(pcPicOrg->getCrAddr(), pcPicRest->getCrAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiOrgDistCb = xCalcSSD(pcPicOrg->getCbAddr(), pcPicDec->getCbAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiOrgDistCr = xCalcSSD(pcPicOrg->getCrAddr(), pcPicDec->getCrAddr(), iCWidth, iCHeight, iCStride);
  m_pcTempAlfParam->chroma_idc = 0;
  if(uiOrgDistCb > uiFiltDistCb)
    m_pcTempAlfParam->chroma_idc += 2;
  if(uiOrgDistCr  > uiFiltDistCr )
    m_pcTempAlfParam->chroma_idc += 1;

  if(m_pcTempAlfParam->chroma_idc)
  {
    if(m_pcTempAlfParam->chroma_idc!=3)
    {
      // chroma filter re-design
      xFilteringFrameChroma(pcPicOrg, pcPicDec, pcPicRest);
    }
    
    UInt64 uiRate, uiDist;
    Double dCost;
    xCalcRDCostChroma(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiRate, uiDist, dCost);
    if( dCost < dMinCost )
    {
      copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
      predictALFCoeffChroma(m_pcBestAlfParam);
      
      ruiBits += uiRate;
      ruiDist += uiDist;
    }
    else
    {
      m_pcBestAlfParam->chroma_idc = 0;
      
      if((m_pcTempAlfParam->chroma_idc>>1)&0x01)
        pcPicDec->copyToPicCb(pcPicRest);
      if(m_pcTempAlfParam->chroma_idc&0x01)
        pcPicDec->copyToPicCr(pcPicRest);
      
      ruiBits += uiMinRate;
      ruiDist += uiMinDist;
    }
  }
  else
  {
    m_pcBestAlfParam->chroma_idc = 0;
    
    ruiBits += uiMinRate;
    ruiDist += uiMinDist;
    
    pcPicDec->copyToPicCb(pcPicRest);
    pcPicDec->copyToPicCr(pcPicRest);
  }
}
#endif

// ====================================================================================================================
// Private member functions
// ====================================================================================================================

Void TEncAdaptiveLoopFilter::xInitParam()
{
  Int i, j;
  
  if (m_ppdAlfCorr != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorr[i][j] = 0;
      }
    }
  }
  else
  {
    m_ppdAlfCorr = new Double*[ALF_MAX_NUM_COEF];
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_ppdAlfCorr[i] = new Double[ALF_MAX_NUM_COEF+1];
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorr[i][j] = 0;
      }
    }
  }
  
  if (m_pdDoubleAlfCoeff != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_pdDoubleAlfCoeff[i] = 0;
    }
  }
  else
  {
    m_pdDoubleAlfCoeff = new Double[ALF_MAX_NUM_COEF];
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_pdDoubleAlfCoeff[i] = 0;
    }
  }
#if ALF_CHROMA_NEW_SHAPES
  if (m_ppdAlfCorrCb != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorrCb[i][j] = 0;
      }
    }
  }
  else
  {
    m_ppdAlfCorrCb = new Double*[ALF_MAX_NUM_COEF];
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_ppdAlfCorrCb[i] = new Double[ALF_MAX_NUM_COEF+1];
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorrCb[i][j] = 0;
      }
    }
  }
  
  if (m_ppdAlfCorrCr != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorrCr[i][j] = 0;
      }
    }
  }
  else
  {
    m_ppdAlfCorrCr = new Double*[ALF_MAX_NUM_COEF];
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      m_ppdAlfCorrCr[i] = new Double[ALF_MAX_NUM_COEF+1];
      for (j = 0; j < ALF_MAX_NUM_COEF+1; j++)
      {
        m_ppdAlfCorrCr[i][j] = 0;
      }
    }
  }
#endif
}

Void TEncAdaptiveLoopFilter::xUninitParam()
{
  Int i;
  
  if (m_ppdAlfCorr != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      delete[] m_ppdAlfCorr[i];
      m_ppdAlfCorr[i] = NULL;
    }
    delete[] m_ppdAlfCorr;
    m_ppdAlfCorr = NULL;
  }
  
  if (m_pdDoubleAlfCoeff != NULL)
  {
    delete[] m_pdDoubleAlfCoeff;
    m_pdDoubleAlfCoeff = NULL;
  }
#if ALF_CHROMA_NEW_SHAPES
  if (m_ppdAlfCorrCb != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      delete[] m_ppdAlfCorrCb[i];
      m_ppdAlfCorrCb[i] = NULL;
    }
    delete[] m_ppdAlfCorrCb;
    m_ppdAlfCorrCb = NULL;
  }
  
  if (m_ppdAlfCorrCr != NULL)
  {
    for (i = 0; i < ALF_MAX_NUM_COEF; i++)
    {
      delete[] m_ppdAlfCorrCr[i];
      m_ppdAlfCorrCr[i] = NULL;
    }
    delete[] m_ppdAlfCorrCr;
    m_ppdAlfCorrCr = NULL;
  }
#endif
}

Void TEncAdaptiveLoopFilter::xCreateTmpAlfCtrlFlags()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->createTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xDestroyTmpAlfCtrlFlags()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->destroyTmpAlfCtrlFlag();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsTo()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagFromTmp();
  }
}

Void TEncAdaptiveLoopFilter::xCopyTmpAlfCtrlFlagsFrom()
{
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    pcCU->copyAlfCtrlFlagToTmp();
  }
}

/** Encode ALF CU control flags
 */
Void TEncAdaptiveLoopFilter::xEncodeCUAlfCtrlFlags()
{
#if MTK_NONCROSS_INLOOP_FILTER
  if(m_uiNumSlicesInPic > 1)
  {
    for(UInt s=0; s< m_uiNumSlicesInPic; s++)
    {
      for(UInt idx=0; idx< m_pSlice[s].getNumLCUs(); idx++)
      {
        CAlfLCU& cAlfLCU = m_pSlice[s][idx];

        for(UInt i=0; i< cAlfLCU.getNumCtrlFlags(); i++)
        {
          m_pcEntropyCoder->encodeAlfCtrlFlag(cAlfLCU.getCUCtrlFlag(i));
        }

      }
    }
    return;
  }
#endif

  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    xEncodeCUAlfCtrlFlag(pcCU, 0, 0);
  }
}

Void TEncAdaptiveLoopFilter::xEncodeCUAlfCtrlFlag(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
#if AD_HOCS_SLICES  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
#else  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
#endif  
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
#if AD_HOCS_SLICES      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#else
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
#endif      
        xEncodeCUAlfCtrlFlag(pcCU, uiAbsPartIdx, uiDepth+1);
    }
    return;
  }
  
  m_pcEntropyCoder->encodeAlfCtrlFlag(pcCU, uiAbsPartIdx);
}
#if MTK_NONCROSS_INLOOP_FILTER
Void TEncAdaptiveLoopFilter::xCalcCorrelationFunc(Int ypos, Int xpos, Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride, Bool bSymmCopyBlockMatrix)
#else
Void TEncAdaptiveLoopFilter::xCalcCorrelationFunc(Pel* pOrg, Pel* pCmp, Int iTap, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride)
#endif
{
  //Patch should be extended before this point................
  //ext_offset  = tap>>1;
  
#if !ALF_CHROMA_NEW_SHAPES
#if TI_ALF_MAX_VSIZE_7
  Int iTapV   = TComAdaptiveLoopFilter::ALFTapHToTapV(iTap);
  Int N       = (iTap * iTapV + 1) >> 1;
  Int offsetV = iTapV >> 1;
#else
  Int N      = (iTap*iTap+1)>>1;
#endif
  Int offset = iTap>>1;
#endif
  
  const Int* pFiltPos;
  
#if ALF_CHROMA_NEW_SHAPES
  Int flH, flV, ii, jj, m, N;
  N = m_sqrFiltLengthTab[iTap] - 1;
  pFiltPos = patternTabShapes[iTap];
  // star shape
  if (iTap == 0)
  {
      flH   = 2;
      flV   = 2;
  }
  // cross shape
  else
  {
      flH   = 5;
      flV   = 2;
  }
#else
  switch(iTap)
  {
    case 5:
      pFiltPos = m_aiSymmetricArray5x5;
      break;
    case 7:
      pFiltPos = m_aiSymmetricArray7x7;
      break;
    case 9:
#if TI_ALF_MAX_VSIZE_7
      pFiltPos = m_aiSymmetricArray9x7;
#else
      pFiltPos = m_aiSymmetricArray9x9;
#endif
      break;
    default:
#if TI_ALF_MAX_VSIZE_7
      pFiltPos = m_aiSymmetricArray9x7;
#else
      pFiltPos = m_aiSymmetricArray9x9;
#endif
      assert(0);
      break;
  }
#endif
  
  Pel* pTerm = new Pel[N];
  
  Int i, j;
#if MTK_NONCROSS_INLOOP_FILTER
  for (Int y = ypos; y < ypos + iHeight; y++)
  {
    for (Int x = xpos; x < xpos + iWidth; x++)
    {
#else    
  for (Int y = 0; y < iHeight; y++)
  {
    for (Int x = 0; x < iWidth; x++)
    {
#endif
      i = 0;
      ::memset(pTerm, 0, sizeof(Pel)*N);
#if ALF_CHROMA_NEW_SHAPES
      if(iTap == 0)
      {
        for (ii=-flV; ii<0; ii++)
        {
          m = (ii == -flV) ? 2 : 1;
          for (jj=-m; jj<=m; jj++)
          {
            if ((m == 1) || (m == 2 && jj%2 == 0))
            {
              pTerm[pFiltPos[i++]] += (pCmp[(y+ii)*iCmpStride + (x+jj)] + pCmp[(y-ii)*iCmpStride + (x-jj)]);
            }
          }
        }
        for (jj=-flH; jj<0; jj++)
        {
          pTerm[pFiltPos[i++]] += (pCmp[(y)*iCmpStride + (x+jj)]+pCmp[(y)*iCmpStride + (x-jj)]);
        }
        pTerm[pFiltPos[i++]] += pCmp[y*iCmpStride + x];
      }
      else
      {
        for (ii=-flV; ii<0; ii++)
        {
          pTerm[pFiltPos[i++]] += (pCmp[(y+ii)*iCmpStride + x]+pCmp[(y-ii)*iCmpStride + x]);
        }
        for (jj=-flH; jj<0; jj++)
        {
          pTerm[pFiltPos[i++]] += (pCmp[(y)*iCmpStride + (x+jj)]+pCmp[(y)*iCmpStride + (x-jj)]);
        }
        pTerm[pFiltPos[i++]] += pCmp[y*iCmpStride + x];
      }
#else
#if TI_ALF_MAX_VSIZE_7
      for (Int yy = y - offsetV; yy <= y + offsetV; yy++)
#else
      for(Int yy=y-offset; yy<=y+offset; yy++)
#endif
      {
        for(Int xx=x-offset; xx<=x+offset; xx++)
        {
          pTerm[pFiltPos[i]] += pCmp[xx + yy*iCmpStride];
          i++;
        }
      }
#endif
      
      for(j=0; j<N; j++)
      {
        m_ppdAlfCorr[j][j] += pTerm[j]*pTerm[j];
        for(i=j+1; i<N; i++)
          m_ppdAlfCorr[j][i] += pTerm[j]*pTerm[i];
        
        // DC offset
        m_ppdAlfCorr[j][N]   += pTerm[j];
        m_ppdAlfCorr[j][N+1] += pOrg[x+y*iOrgStride]*pTerm[j];
      }
      // DC offset
      for(i=0; i<N; i++)
        m_ppdAlfCorr[N][i] += pTerm[i];
      m_ppdAlfCorr[N][N]   += 1;
      m_ppdAlfCorr[N][N+1] += pOrg[x+y*iOrgStride];
    }
  }
#if MTK_NONCROSS_INLOOP_FILTER
  if(bSymmCopyBlockMatrix)
  {
#endif
  for(j=0; j<N-1; j++)
  {
    for(i=j+1; i<N; i++)
      m_ppdAlfCorr[i][j] = m_ppdAlfCorr[j][i];
  }
#if MTK_NONCROSS_INLOOP_FILTER
  }
#endif

  delete[] pTerm;
  pTerm = NULL;
}

#if IBDI_DISTORTION
UInt64 TEncAdaptiveLoopFilter::xCalcSSD(Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride )
{
  UInt64 uiSSD = 0;
  Int x, y;

  Int iShift = g_uiBitIncrement;
  Int iOffset = (g_uiBitIncrement>0)? (1<<(g_uiBitIncrement-1)):0;
  Int iTemp;

  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = ((pOrg[x]+iOffset)>>iShift) - ((pCmp[x]+iOffset)>>iShift); uiSSD += iTemp * iTemp;
    }
    pOrg += iStride;
    pCmp += iStride;
  }

  return uiSSD;;
}
#else
UInt64 TEncAdaptiveLoopFilter::xCalcSSD(Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride )
{
  UInt64 uiSSD = 0;
  Int x, y;
  
  UInt uiShift = g_uiBitIncrement<<1;
  Int iTemp;
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      iTemp = pOrg[x] - pCmp[x]; uiSSD += ( iTemp * iTemp ) >> uiShift;
    }
    pOrg += iStride;
    pCmp += iStride;
  }
  
  return uiSSD;;
}
#endif

Int TEncAdaptiveLoopFilter::xGauss(Double **a, Int N)
{
  Int i, j, k;
  Double t;
  
  for(k=0; k<N; k++)
  {
    if (a[k][k] <0.000001)
      return 1;
  }
  
  for(k=0; k<N-1; k++)
  {
    for(i=k+1;i<N; i++)
    {
      t=a[i][k]/a[k][k];
      for(j=k+1; j<=N; j++)
      {
        a[i][j] -= t * a[k][j];
        if(i==j && fabs(a[i][j])<0.000001) return 1;
      }
    }
  }
  for(i=N-1; i>=0; i--)
  {
    t = a[i][N];
    for(j=i+1; j<N; j++)
      t -= a[i][j] * a[j][N];
    a[i][N] = t / a[i][i];
  }
  return 0;
}

Void TEncAdaptiveLoopFilter::xFilterCoefQuickSort( Double *coef_data, Int *coef_num, Int upper, Int lower )
{
  Double mid, tmp_data;
  Int i, j, tmp_num;
  
  i = upper;
  j = lower;
  mid = coef_data[(lower+upper)>>1];
  do
  {
    while( coef_data[i] < mid ) i++;
    while( mid < coef_data[j] ) j--;
    if( i <= j )
    {
      tmp_data = coef_data[i];
      tmp_num  = coef_num[i];
      coef_data[i] = coef_data[j];
      coef_num[i]  = coef_num[j];
      coef_data[j] = tmp_data;
      coef_num[j]  = tmp_num;
      i++;
      j--;
    }
  } while( i <= j );
  if ( upper < j ) xFilterCoefQuickSort(coef_data, coef_num, upper, j);
  if ( i < lower ) xFilterCoefQuickSort(coef_data, coef_num, i, lower);
}

Void TEncAdaptiveLoopFilter::xQuantFilterCoef(Double* h, Int* qh, Int tap, int bit_depth)
{
  Int i, N;
  Int max_value, min_value;
  Double dbl_total_gain;
  Int total_gain, q_total_gain;
  Int upper, lower;
  Double *dh;
  Int    *nc;
  const Int    *pFiltMag;
  
#if ALF_CHROMA_NEW_SHAPES
  N = m_sqrFiltLengthTab[tap] - 1;
  // star shape
  if(tap == 0)
  {
    pFiltMag = weightsShape0Sym;
  }
  // cross shape
  else
  {
    pFiltMag = weightsShape1Sym;
  }
#else
  switch(tap)
  {
    case 5:
      pFiltMag = m_aiSymmetricMag5x5;
      break;
    case 7:
      pFiltMag = m_aiSymmetricMag7x7;
      break;
    case 9:
#if TI_ALF_MAX_VSIZE_7
      pFiltMag = m_aiSymmetricMag9x7;
#else
      pFiltMag = m_aiSymmetricMag9x9;
#endif
      break;
    default:
#if TI_ALF_MAX_VSIZE_7
      pFiltMag = m_aiSymmetricMag9x7;
#else
      pFiltMag = m_aiSymmetricMag9x9;
#endif
      assert(0);
      break;
  }
  
#if TI_ALF_MAX_VSIZE_7
  Int tapV = TComAdaptiveLoopFilter::ALFTapHToTapV(tap);
  N = (tap * tapV + 1) >> 1;
#else
  N = (tap*tap+1)>>1;
#endif
#endif
  
  dh = new Double[N];
  nc = new Int[N];
  
  max_value =   (1<<(1+ALF_NUM_BIT_SHIFT))-1;
  min_value = 0-(1<<(1+ALF_NUM_BIT_SHIFT));
  
  dbl_total_gain=0.0;
  q_total_gain=0;
  for(i=0; i<N; i++)
  {
    if(h[i]>=0.0)
      qh[i] =  (Int)( h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
    else
      qh[i] = -(Int)(-h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
    
    dh[i] = (Double)qh[i]/(Double)(1<<ALF_NUM_BIT_SHIFT) - h[i];
    dh[i]*=pFiltMag[i];
    dbl_total_gain += h[i]*pFiltMag[i];
    q_total_gain   += qh[i]*pFiltMag[i];
    nc[i] = i;
  }
  
  // modification of quantized filter coefficients
  total_gain = (Int)(dbl_total_gain*(1<<ALF_NUM_BIT_SHIFT)+0.5);
  
  if( q_total_gain != total_gain )
  {
    xFilterCoefQuickSort(dh, nc, 0, N-1);
    if( q_total_gain > total_gain )
    {
      upper = N-1;
      while( q_total_gain > total_gain+1 )
      {
        i = nc[upper%N];
        qh[i]--;
        q_total_gain -= pFiltMag[i];
        upper--;
      }
      if( q_total_gain == total_gain+1 )
      {
        if(dh[N-1]>0)
          qh[N-1]--;
        else
        {
          i=nc[upper%N];
          qh[i]--;
          qh[N-1]++;
        }
      }
    }
    else if( q_total_gain < total_gain )
    {
      lower = 0;
      while( q_total_gain < total_gain-1 )
      {
        i=nc[lower%N];
        qh[i]++;
        q_total_gain += pFiltMag[i];
        lower++;
      }
      if( q_total_gain == total_gain-1 )
      {
        if(dh[N-1]<0)
          qh[N-1]++;
        else
        {
          i=nc[lower%N];
          qh[i]++;
          qh[N-1]--;
        }
      }
    }
  }
  
  // set of filter coefficients
  for(i=0; i<N; i++)
  {
    qh[i] = max(min_value,min(max_value, qh[i]));
  }
  
  // DC offset
  //  max_value = Min(  (1<<(3+Max(img_bitdepth_luma,img_bitdepth_chroma)))-1, (1<<14)-1);
  //  min_value = Max( -(1<<(3+Max(img_bitdepth_luma,img_bitdepth_chroma))),  -(1<<14)  );
  max_value = min(  (1<<(3+g_uiBitDepth + g_uiBitIncrement))-1, (1<<14)-1);
  min_value = max( -(1<<(3+g_uiBitDepth + g_uiBitIncrement)),  -(1<<14)  );
  
  qh[N] =  (h[N]>=0.0)? (Int)( h[N]*(1<<(ALF_NUM_BIT_SHIFT-bit_depth+8)) + 0.5) : -(Int)(-h[N]*(1<<(ALF_NUM_BIT_SHIFT-bit_depth+8)) + 0.5);
  qh[N] = max(min_value,min(max_value, qh[N]));
  
  delete[] dh;
  dh = NULL;
  
  delete[] nc;
  nc = NULL;
}

Void TEncAdaptiveLoopFilter::xClearFilterCoefInt(Int* qh, Int N)
{
  // clear
  memset( qh, 0, sizeof( Int ) * N );
  
  // center pos
  qh[N-2]  = 1<<ALF_NUM_BIT_SHIFT;
}

/** Calculate RD cost
 * \param pAlfParam ALF parameters
 * \param uiDist distortion
 * \returns ruiRate bitrate
 * \returns rdCost RD cost
 */
Void TEncAdaptiveLoopFilter::xCalcRDCost(ALFParam* pAlfParam, UInt64& ruiRate, UInt64 uiDist, Double& rdCost)
{
  if(pAlfParam != NULL)
  {
#if E045_SLICE_COMMON_INFO_SHARING
    ruiRate = 0;
#endif

    Int* piTmpCoef;
    piTmpCoef = new Int[ALF_MAX_NUM_COEF];
    
    memcpy(piTmpCoef, pAlfParam->coeff, sizeof(Int)*pAlfParam->num_coeff);
    
    predictALFCoeff(pAlfParam);
    
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);

#if E045_SLICE_COMMON_INFO_SHARING
    if(m_uiNumSlicesInPic ==1)
    {
      if(m_bSharedPPSAlfParamEnabled)
      {
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
      }

      m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
      ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
    }
    else
    {
      ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
      for(UInt s=0; s< m_uiNumSlicesInPic; s++)
      {
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam, m_uiNumSlicesInPic, &(m_pSlice[s]));
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
      }
    }
#else
    if(pAlfParam->cu_control_flag)
    {
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
      m_pcEntropyCoder->setSliceGranularity(m_iSGDepth);
#endif

      m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
    }
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
    memcpy(pAlfParam->coeff, piTmpCoef, sizeof(int)*pAlfParam->num_coeff);
    delete[] piTmpCoef;
    piTmpCoef = NULL;
  }
  else
  {
    ruiRate = 1;
  }
  
  rdCost      = (Double)(ruiRate) * m_dLambdaLuma + (Double)(uiDist);
}

/** Calculate RD cost
 * \param pcPicOrg original picture buffer
 * \param pcPicCmp compared picture buffer
 * \param pAlfParam ALF parameters
 * \returns ruiRate bitrate
 * \returns uiDist distortion
 * \returns rdCost RD cost
 */
Void TEncAdaptiveLoopFilter::xCalcRDCost(TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost)
{
  if(pAlfParam != NULL)
  {
#if E045_SLICE_COMMON_INFO_SHARING
    ruiRate = 0;
#endif
    Int* piTmpCoef;
    piTmpCoef = new Int[ALF_MAX_NUM_COEF];
    
    memcpy(piTmpCoef, pAlfParam->coeff, sizeof(Int)*pAlfParam->num_coeff);
    
    predictALFCoeff(pAlfParam);
    
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);
    
#if E045_SLICE_COMMON_INFO_SHARING
    if(m_uiNumSlicesInPic ==1)
    {
      if(m_bSharedPPSAlfParamEnabled)
      {
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
      }

      m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
      ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
    }
    else
    {
      ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
      for(UInt s=0; s< m_uiNumSlicesInPic; s++)
      {
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam, m_uiNumSlicesInPic, &(m_pSlice[s]));
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
      }
    }
#else
    if(pAlfParam->cu_control_flag)
    {
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
      m_pcEntropyCoder->setSliceGranularity(m_iSGDepth);
#endif

      m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
    }
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
    memcpy(pAlfParam->coeff, piTmpCoef, sizeof(int)*pAlfParam->num_coeff);
    delete[] piTmpCoef;
    piTmpCoef = NULL;
  }
  else
  {
    ruiRate = 1;
  }
  
  ruiDist     = xCalcSSD(pcPicOrg->getLumaAddr(), pcPicCmp->getLumaAddr(), pcPicOrg->getWidth(), pcPicOrg->getHeight(), pcPicOrg->getStride());
  rdCost      = (Double)(ruiRate) * m_dLambdaLuma + (Double)(ruiDist);
}

/** Calculate RD cost for chroma ALF
 * \param pcPicOrg original picture buffer
 * \param pcPicCmp compared picture buffer
 * \param pAlfParam ALF parameters
 * \returns ruiRate bitrate
 * \returns uiDist distortion
 * \returns rdCost RD cost
 */
Void TEncAdaptiveLoopFilter::xCalcRDCostChroma(TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost)
{
  if(pAlfParam->chroma_idc)
  {
#if ALF_CHROMA_NEW_SHAPES
    ruiRate = xCalcRateChroma(pAlfParam);
#else
#if E045_SLICE_COMMON_INFO_SHARING
    ruiRate = 0;
#endif

    Int* piTmpCoef;
    piTmpCoef = new Int[ALF_MAX_NUM_COEF_C];
    
    memcpy(piTmpCoef, pAlfParam->coeff_chroma, sizeof(Int)*pAlfParam->num_coeff_chroma);
    
    predictALFCoeffChroma(pAlfParam);
    
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);
    
#if E045_SLICE_COMMON_INFO_SHARING
    if(m_uiNumSlicesInPic ==1)
    {
      if(m_bSharedPPSAlfParamEnabled)
      {
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
      }

      m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
      ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
    }
    else
    {
      ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
      for(UInt s=0; s< m_uiNumSlicesInPic; s++)
      {
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam, m_uiNumSlicesInPic, &(m_pSlice[s]));
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
      }
    }
#else
    if(pAlfParam->cu_control_flag)
    {
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
      m_pcEntropyCoder->setSliceGranularity(m_iSGDepth);
#endif

      m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
    }
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
    memcpy(pAlfParam->coeff_chroma, piTmpCoef, sizeof(int)*pAlfParam->num_coeff_chroma);
    delete[] piTmpCoef;
    piTmpCoef = NULL;
#endif
  }
  ruiDist = 0;
  ruiDist += xCalcSSD(pcPicOrg->getCbAddr(), pcPicCmp->getCbAddr(), (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride());
  ruiDist += xCalcSSD(pcPicOrg->getCrAddr(), pcPicCmp->getCrAddr(), (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride());
  rdCost  = (Double)(ruiRate) * m_dLambdaChroma + (Double)(ruiDist);
}

Void TEncAdaptiveLoopFilter::xFilteringFrameChroma(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
#if ALF_CHROMA_NEW_SHAPES
  Int  tap;
  Int* qh;

  tap = m_pcTempAlfParam->realfiltNo_chroma;
#else
  Int    i, tap, N, err_code;
  Int* qh;
  
  tap  = m_pcTempAlfParam->tap_chroma;
  N    = m_pcTempAlfParam->num_coeff_chroma;
#endif
  qh   = m_pcTempAlfParam->coeff_chroma;

#if !ALF_CHROMA_NEW_SHAPES
  // initialize correlation
  for(i=0; i<N; i++)
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*(N+1));
  
  if ((m_pcTempAlfParam->chroma_idc>>1)&0x01)
  {
    Pel* pOrg = pcPicOrg->getCbAddr();
    Pel* pCmp = pcPicDec->getCbAddr();
#if MTK_NONCROSS_INLOOP_FILTER
    if(!m_bUseNonCrossALF)
      xCalcCorrelationFunc(0, 0, pOrg, pCmp, tap, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride(), true);
    else
      xCalcCorrelationFuncforChromaSlices(ALF_Cb, pOrg, pCmp, tap, pcPicOrg->getCStride(), pcPicDec->getCStride());
#else        
    xCalcCorrelationFunc(pOrg, pCmp, tap, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride());
#endif
  }
  if ((m_pcTempAlfParam->chroma_idc)&0x01)
  {
    Pel* pOrg = pcPicOrg->getCrAddr();
    Pel* pCmp = pcPicDec->getCrAddr();
#if MTK_NONCROSS_INLOOP_FILTER
    if(!m_bUseNonCrossALF)
      xCalcCorrelationFunc(0, 0, pOrg, pCmp, tap, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride(), true);
    else
      xCalcCorrelationFuncforChromaSlices(ALF_Cr, pOrg, pCmp, tap, pcPicOrg->getCStride(), pcPicDec->getCStride());
#else
    xCalcCorrelationFunc(pOrg, pCmp, tap, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride());
#endif
  }
  
  err_code = xGauss(m_ppdAlfCorr, N);
  
  if(err_code)
  {
    xClearFilterCoefInt(qh, N);
  }
  else
  {
    for(i=0; i<N; i++)
      m_pdDoubleAlfCoeff[i] = m_ppdAlfCorr[i][N];
    
    xQuantFilterCoef(m_pdDoubleAlfCoeff, qh, tap, g_uiBitDepth + g_uiBitIncrement);
  }
#endif
  
  
  if ((m_pcTempAlfParam->chroma_idc>>1)&0x01)
  {
#if MTK_NONCROSS_INLOOP_FILTER
    if(! m_bUseNonCrossALF)
      xFrameChroma(0, 0, (pcPicRest->getHeight() >> 1), (pcPicRest->getWidth() >>1), pcPicDec, pcPicRest, qh, tap, 0);
    else
      xFrameChromaforSlices(ALF_Cb, pcPicDec, pcPicRest, qh, tap);
#else
    xFrameChroma(pcPicDec, pcPicRest, qh, tap, 0);
#endif
  }
  if ((m_pcTempAlfParam->chroma_idc)&0x01)
  {
#if MTK_NONCROSS_INLOOP_FILTER
    if(! m_bUseNonCrossALF)
      xFrameChroma(0, 0, (pcPicRest->getHeight() >> 1), (pcPicRest->getWidth() >>1), pcPicDec, pcPicRest, qh, tap, 1);
    else
      xFrameChromaforSlices(ALF_Cr, pcPicDec, pcPicRest, qh, tap);
#else
    xFrameChroma(pcPicDec, pcPicRest, qh, tap, 1);
#endif
  }
  
  if(m_pcTempAlfParam->chroma_idc<3)
  {
    if(m_pcTempAlfParam->chroma_idc==1)
    {
      pcPicDec->copyToPicCb(pcPicRest);
    }
    if(m_pcTempAlfParam->chroma_idc==2)
    {
      pcPicDec->copyToPicCr(pcPicRest);
    }
  }
  
}

/** Restore the not-filtered pixels
 * \param pcPicDec picture buffer before filtering
 * \param pcPicRest picture buffer after filtering
 */
Void TEncAdaptiveLoopFilter::xCopyDecToRestCUs(TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{

#if MTK_NONCROSS_INLOOP_FILTER
  if(m_uiNumSlicesInPic > 1)
  {
    Pel* pPicDecLuma          = pcPicDec->getLumaAddr();
    Pel* pPicRestLuma         = pcPicRest->getLumaAddr();
    Int  iStride               = pcPicDec->getStride();
    UInt uiMaxNumSUInLCU       = m_pcPic->getNumPartInCU();
    UInt uiMaxNumSUInLCUWidth  = m_pcPic->getNumPartInWidth();
    UInt uiMAxNumSUInLCUHeight = m_pcPic->getNumPartInHeight();
    UInt uiSUWidth             = m_pcPic->getMinCUWidth();
    UInt uiSUHeight            = m_pcPic->getMinCUHeight();

    TComDataCU* pcCU;
    UInt s, idx, uiPosOffset;
    UInt uiStartSU, uiEndSU, uiLCUX, uiLCUY, uiCurrSU, uiCtrlNumSU, uiWidth, uiHeight, uiLPelX, uiTPelY;
    Int iAlfDepth, iCUDepth, iSetDepth;
    Pel* pDec;
    Pel* pRest;

    for(s=0; s< m_uiNumSlicesInPic; s++)
    {
      iAlfDepth = m_pSlice[s].getCUCtrlDepth();

      for(idx = 0; idx < m_pSlice[s].getNumLCUs(); idx++)
      {
        CAlfLCU& cAlfLCU = m_pSlice[s][idx];

        pcCU                   = cAlfLCU.getCU();
        uiStartSU              = cAlfLCU.getStartSU();
        uiEndSU                = cAlfLCU.getEndSU();
        uiLCUX                 = pcCU->getCUPelX();
        uiLCUY                 = pcCU->getCUPelY();

        if(uiStartSU == 0 && uiEndSU == uiMaxNumSUInLCU -1)
        {
          uiCurrSU = uiStartSU;
          while(uiCurrSU < uiMaxNumSUInLCU)
          {
            //depth of this CU
            iCUDepth = pcCU->getDepth(uiCurrSU);

            //choose the min. depth for ALF
            iSetDepth   = (iAlfDepth < iCUDepth)?(iAlfDepth):(iCUDepth);
            uiCtrlNumSU = uiMaxNumSUInLCU >> (iSetDepth << 1);

            if(!pcCU->getAlfCtrlFlag(uiCurrSU))
            {
              uiWidth = (uiMaxNumSUInLCUWidth  >> iSetDepth)*uiSUWidth  ;
              uiHeight= (uiMAxNumSUInLCUHeight >> iSetDepth)*uiSUHeight ;
              uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
              uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];

              uiPosOffset = uiTPelY*iStride + uiLPelX;
              pDec = pPicDecLuma + uiPosOffset;
              pRest= pPicRestLuma+ uiPosOffset;

              for(Int y=0; y< uiHeight; y++)
              {
                ::memcpy(pRest, pDec, sizeof(Pel)*uiWidth);
                pDec += iStride;
                pRest+= iStride;
              }
            }
            uiCurrSU += uiCtrlNumSU;
          }
        }
        else
        {
          for(uiCurrSU= uiStartSU; uiCurrSU<= uiEndSU; uiCurrSU++)
          {
            uiLPelX = uiLCUX + g_auiRasterToPelX[ g_auiZscanToRaster[uiCurrSU] ];
            uiTPelY = uiLCUY + g_auiRasterToPelY[ g_auiZscanToRaster[uiCurrSU] ];

            if( !( uiLPelX < m_img_width )  || !( uiTPelY < m_img_height )  )
            {
              continue;
            }

            if(!pcCU->getAlfCtrlFlag(uiCurrSU))
            {
              uiPosOffset = uiTPelY*iStride + uiLPelX;
              pDec = pPicDecLuma + uiPosOffset;
              pRest= pPicRestLuma+ uiPosOffset;
              for(Int y=0; y< uiSUHeight; y++)
              {
                ::memcpy(pRest, pDec, sizeof(Pel)*uiSUWidth);
                pDec += iStride;
                pRest+= iStride;
              }
            }
          }
        }
      }
    }
    return;
  }
#endif

  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    xCopyDecToRestCU(pcCU, 0, 0, pcPicDec, pcPicRest);
  }
}

Void TEncAdaptiveLoopFilter::xCopyDecToRestCU(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )      
        xCopyDecToRestCU(pcCU, uiAbsPartIdx, uiDepth+1, pcPicDec, pcPicRest);
    }
    return;
  }
  
  if (!pcCU->getAlfCtrlFlag(uiAbsPartIdx))
  {
    UInt uiCUAddr = pcCU->getAddr();
    
    Int iWidth = pcCU->getWidth(uiAbsPartIdx);
    Int iHeight = pcCU->getHeight(uiAbsPartIdx);
    
    Pel* pRec = pcPicDec->getLumaAddr(uiCUAddr, uiAbsPartIdx);
    Pel* pFilt = pcPicRest->getLumaAddr(uiCUAddr, uiAbsPartIdx);
    
    Int iRecStride = pcPicDec->getStride();
    Int iFiltStride = pcPicRest->getStride();
    
    for (Int y = 0; y < iHeight; y++)
    {
      for (Int x = 0; x < iWidth; x++)
      {
        pFilt[x] = pRec[x];
      }
      pRec += iRecStride;
      pFilt += iFiltStride;
    }
  }
}

Void TEncAdaptiveLoopFilter::xcollectStatCodeFilterCoeffForce0(int **pDiffQFilterCoeffIntPP, int fl, int sqrFiltLength, 
                                                               int filters_per_group, int bitsVarBin[])
{
  int i, k, kMin, kStart, minBits, ind, scanPos, maxScanVal, coeffVal, 
  *pDepthInt=NULL, kMinTab[MAX_SQR_FILT_LENGTH], bitsCoeffScan[MAX_SCAN_VAL][MAX_EXP_GOLOMB],
  minKStart, minBitsKStart, bitsKStart;
  
#if STAR_CROSS_SHAPES_LUMA
  pDepthInt = pDepthIntTabShapes[fl];
#else
  pDepthInt=pDepthIntTab[fl-2];
#endif
  
  maxScanVal=0;
  for (i=0; i<sqrFiltLength; i++)
  {
    maxScanVal=max(maxScanVal, pDepthInt[i]);
  }
  
  // vlc for all
  memset(bitsCoeffScan, 0, MAX_SCAN_VAL * MAX_EXP_GOLOMB * sizeof(int));
  for(ind=0; ind<filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {     
      scanPos=pDepthInt[i]-1;
      coeffVal=abs(pDiffQFilterCoeffIntPP[ind][i]);
      for (k=1; k<15; k++)
      {
        bitsCoeffScan[scanPos][k] += lengthGolomb(coeffVal, k);
      }
    }
  }
  
  minBitsKStart=0;
  minKStart = -1;
  for (k=1; k<8; k++)
  { 
    bitsKStart=0; kStart=k;
    for (scanPos=0; scanPos<maxScanVal; scanPos++)
    {
      kMin=kStart; minBits=bitsCoeffScan[scanPos][kMin];
      
      if (bitsCoeffScan[scanPos][kStart+1]<minBits)
      {
        kMin=kStart+1; minBits=bitsCoeffScan[scanPos][kMin];
      }
      kStart=kMin;
      bitsKStart+=minBits;
    }
    if (bitsKStart<minBitsKStart || k==1)
    {
      minBitsKStart=bitsKStart;
      minKStart=k;
    }
  }
  
  kStart = minKStart; 
  for (scanPos=0; scanPos<maxScanVal; scanPos++)
  {
    kMin=kStart; minBits=bitsCoeffScan[scanPos][kMin];
    
    if (bitsCoeffScan[scanPos][kStart+1]<minBits)
    {
      kMin = kStart+1; 
      minBits = bitsCoeffScan[scanPos][kMin];
    }
    
    kMinTab[scanPos] = kMin;
    kStart = kMin;
  }
  
  for(ind=0; ind<filters_per_group; ++ind)
  {
    bitsVarBin[ind]=0;
    for(i = 0; i < sqrFiltLength; i++)
    {
      scanPos=pDepthInt[i]-1;
      bitsVarBin[ind] += lengthGolomb(abs(pDiffQFilterCoeffIntPP[ind][i]), kMinTab[scanPos]);
    }
  }
}

Void TEncAdaptiveLoopFilter::xdecideCoeffForce0(int codedVarBins[NO_VAR_BINS], double errorForce0Coeff[], double errorForce0CoeffTab[NO_VAR_BINS][2], int bitsVarBin[NO_VAR_BINS], double lambda, int filters_per_fr)
{
  int filtNo;
  int ind;
  
  errorForce0Coeff[0]=errorForce0Coeff[1]=0;
  for (ind=0; ind<16; ind++) codedVarBins[ind]=0;
  
  for(filtNo=0; filtNo<filters_per_fr; filtNo++)
  {
    // No coeffcient prediction bits used
#if ENABLE_FORCECOEFF0
    double lagrangianDiff;
    lagrangianDiff=errorForce0CoeffTab[filtNo][0]-(errorForce0CoeffTab[filtNo][1]+lambda*bitsVarBin[filtNo]);
    codedVarBins[filtNo]=(lagrangianDiff>0)? 1 : 0;
    errorForce0Coeff[0]+=errorForce0CoeffTab[filtNo][codedVarBins[filtNo]];
    errorForce0Coeff[1]+=errorForce0CoeffTab[filtNo][1];
#else
    codedVarBins[filtNo]= 1;
    errorForce0Coeff[0]+=errorForce0CoeffTab[filtNo][codedVarBins[filtNo]];
    errorForce0Coeff[1]+=errorForce0CoeffTab[filtNo][1];
#endif
  }   
}

double TEncAdaptiveLoopFilter::xfindBestCoeffCodMethod(int codedVarBins[NO_VAR_BINS], int *forceCoeff0, 
                                                       int **filterCoeffSymQuant, int fl, int sqrFiltLength, 
                                                       int filters_per_fr, double errorForce0CoeffTab[NO_VAR_BINS][2], 
                                                       double *errorQuant, double lambda)

{
  int bitsVarBin[NO_VAR_BINS], createBistream, coeffBits, coeffBitsForce0;
  double errorForce0Coeff[2], lagrangianForce0, lagrangian;
  
  xcollectStatCodeFilterCoeffForce0(filterCoeffSymQuant, fl, sqrFiltLength,  
                                    filters_per_fr, bitsVarBin);
  
  xdecideCoeffForce0(codedVarBins, errorForce0Coeff, errorForce0CoeffTab, bitsVarBin, lambda, filters_per_fr);
  
  coeffBitsForce0 = xsendAllFiltersPPPredForce0(filterCoeffSymQuant, fl, sqrFiltLength, 
                                                filters_per_fr, codedVarBins, createBistream=0, m_tempALFp);
  
  coeffBits = xsendAllFiltersPPPred(filterCoeffSymQuant, fl, sqrFiltLength, filters_per_fr, 
                                    createBistream=0, m_tempALFp);
  
  lagrangianForce0=errorForce0Coeff[0]+lambda*coeffBitsForce0;
  lagrangian=errorForce0Coeff[1]+lambda*coeffBits;
  if (lagrangianForce0<lagrangian)
  {
    *errorQuant=errorForce0Coeff[0];
    *forceCoeff0=1;
    return(lagrangianForce0);
  }
  else
  {
    *errorQuant=errorForce0Coeff[1];
    *forceCoeff0=0;
    return(lagrangian);
  }
}

Int TEncAdaptiveLoopFilter::xsendAllFiltersPPPred(int **FilterCoeffQuant, int fl, int sqrFiltLength, 
                                                  int filters_per_group, int createBistream, ALFParam* ALFp)
{
  int ind, bit_ct = 0, bit_ct0 = 0, i;
  int predMethod = 0;
  int force0 = 0;
  Int64 Newbit_ct;
  
  bit_ct0 = xcodeFilterCoeff(FilterCoeffQuant, fl, sqrFiltLength, filters_per_group, 0);
  
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    if(ind == 0)
    {
      for(i = 0; i < sqrFiltLength; i++)
        m_diffFilterCoeffQuant[ind][i] = FilterCoeffQuant[ind][i];
    }
    else
    {
      for(i = 0; i < sqrFiltLength; i++)
        m_diffFilterCoeffQuant[ind][i] = FilterCoeffQuant[ind][i] - FilterCoeffQuant[ind-1][i];
    }
  }
  
  if(xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group, 0) >= bit_ct0)
  {
    predMethod = 0;  
    if(filters_per_group > 1)
      bit_ct += lengthPredFlags(force0, predMethod, NULL, 0, createBistream);
    bit_ct += xcodeFilterCoeff(FilterCoeffQuant, fl, sqrFiltLength, filters_per_group, createBistream);
  }
  else
  {
    predMethod = 1;
    if(filters_per_group > 1)
      bit_ct += lengthPredFlags(force0, predMethod, NULL, 0, createBistream);
    bit_ct += xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group, createBistream);
  }
  
  ALFp->forceCoeff0 = 0;
  ALFp->filters_per_group_diff = filters_per_group;
  ALFp->filters_per_group = filters_per_group;
  ALFp->predMethod = predMethod;
  ALFp->num_coeff = sqrFiltLength;
#if STAR_CROSS_SHAPES_LUMA
  ALFp->realfiltNo = fl;
#else
  if (ALFp->num_coeff == SQR_FILT_LENGTH_5SYM)
    ALFp->realfiltNo=2;
  else if (ALFp->num_coeff == SQR_FILT_LENGTH_7SYM)
    ALFp->realfiltNo=1;
  else
    ALFp->realfiltNo=0;
#endif
  
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      if (predMethod) ALFp->coeffmulti[ind][i] = m_diffFilterCoeffQuant[ind][i];
      else ALFp->coeffmulti[ind][i] = FilterCoeffQuant[ind][i];
    }
  }
  m_pcDummyEntropyCoder->codeFiltCountBit(ALFp, &Newbit_ct);
  
  
  //  return(bit_ct);
  return ((Int)Newbit_ct);
}


Int TEncAdaptiveLoopFilter::xsendAllFiltersPPPredForce0(int **FilterCoeffQuant, int fl, int sqrFiltLength, int filters_per_group, 
                                                        int codedVarBins[NO_VAR_BINS], int createBistream, ALFParam* ALFp)
{
  int ind, bit_ct=0, bit_ct0, i, j;
  int filters_per_group_temp, filters_per_group_diff;
  int chosenPred = 0;
  int force0 = 1;
  Int64 Newbit_ct;
  
  i = 0;
  for(ind = 0; ind < filters_per_group; ind++)
  {
    if(codedVarBins[ind] == 1)
    {
      for(j = 0; j < sqrFiltLength; j++)
        m_FilterCoeffQuantTemp[i][j]=FilterCoeffQuant[ind][j];
      i++;
    }
  }
  filters_per_group_diff = filters_per_group_temp = i;
  
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    if(ind == 0)
    {
      for(i = 0; i < sqrFiltLength; i++)
        m_diffFilterCoeffQuant[ind][i] = m_FilterCoeffQuantTemp[ind][i];
    }
    else
    {
      for(i = 0; i < sqrFiltLength; i++)
        m_diffFilterCoeffQuant[ind][i] = m_FilterCoeffQuantTemp[ind][i] - m_FilterCoeffQuantTemp[ind-1][i];
    }
  }
  
  if(!((filters_per_group_temp == 0) && (filters_per_group == 1)))
  {
    bit_ct0 = xcodeFilterCoeff(m_FilterCoeffQuantTemp, fl, sqrFiltLength, filters_per_group_temp, 0);
    
    if(xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group_diff, 0) >= bit_ct0)
    {
      chosenPred = 0;
      bit_ct += lengthPredFlags(force0, chosenPred, codedVarBins, filters_per_group, createBistream);
      bit_ct += xcodeFilterCoeff(m_FilterCoeffQuantTemp, fl, sqrFiltLength, filters_per_group_temp, createBistream);
    }
    else
    {
      chosenPred = 1;
      bit_ct += lengthPredFlags(force0, chosenPred, codedVarBins, filters_per_group, createBistream);
      bit_ct += xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group_temp, createBistream);
    }
  }
  ALFp->forceCoeff0 = 1;
  ALFp->predMethod = chosenPred;
  ALFp->filters_per_group_diff = filters_per_group_diff;
  ALFp->filters_per_group = filters_per_group;
  ALFp->num_coeff = sqrFiltLength;
#if STAR_CROSS_SHAPES_LUMA
  ALFp->realfiltNo = fl;
#else
  if (ALFp->num_coeff == SQR_FILT_LENGTH_5SYM)
    ALFp->realfiltNo=2;
  else if (ALFp->num_coeff == SQR_FILT_LENGTH_7SYM)
    ALFp->realfiltNo=1;
  else
    ALFp->realfiltNo=0;
#endif
  
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    ALFp->codedVarBins[ind] = codedVarBins[ind];
  }
  for(ind = 0; ind < filters_per_group_diff; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      if (chosenPred) ALFp->coeffmulti[ind][i] = m_diffFilterCoeffQuant[ind][i];
      else ALFp->coeffmulti[ind][i] = m_FilterCoeffQuantTemp[ind][i];
    }
  }
  m_pcDummyEntropyCoder->codeFiltCountBit(ALFp, &Newbit_ct);
  
  return ((Int)Newbit_ct);
}

//filtNo==-1/realfiltNo, noFilters=filters_per_frames, realfiltNo=filtNo
Int TEncAdaptiveLoopFilter::xcodeAuxInfo(int filtNo, int noFilters, int varIndTab[NO_VAR_BINS], int frNo, int createBitstream,int realfiltNo, ALFParam* ALFp)
{
  int i, filterPattern[NO_VAR_BINS], startSecondFilter=0, bitCt=0;
  Int64 NewbitCt;
  
  //send realfiltNo (tap related)
  ALFp->realfiltNo = realfiltNo;
  ALFp->filtNo = filtNo;
  
  if(filtNo >= 0)
  {
    // decide startSecondFilter and filterPattern
    if(noFilters > 1)
    {
      memset(filterPattern, 0, NO_VAR_BINS * sizeof(int)); 
      for(i = 1; i < NO_VAR_BINS; ++i)
      {
        if(varIndTab[i] != varIndTab[i-1])
        {
          filterPattern[i] = 1;
          startSecondFilter = i;
        }
      }
      memcpy (ALFp->filterPattern, filterPattern, NO_VAR_BINS * sizeof(int));
      ALFp->startSecondFilter = startSecondFilter;
    }
    
    //send noFilters (filters_per_frame)
    //0: filters_per_frame = 1
    //1: filters_per_frame = 2
    //2: filters_per_frame > 2 (exact number from filterPattern)

    ALFp->noFilters = min(noFilters-1,2);
    if (noFilters<=0) printf("error\n");
  }
  m_pcDummyEntropyCoder->codeAuxCountBit(ALFp, &NewbitCt);
  bitCt = (int) NewbitCt;
  return(bitCt);
}

Int   TEncAdaptiveLoopFilter::xcodeFilterCoeff(int **pDiffQFilterCoeffIntPP, int fl, int sqrFiltLength, 
                                               int filters_per_group, int createBitstream)
{
  int i, k, kMin, kStart, minBits, ind, scanPos, maxScanVal, coeffVal, len = 0,
  *pDepthInt=NULL, kMinTab[MAX_SQR_FILT_LENGTH], bitsCoeffScan[MAX_SCAN_VAL][MAX_EXP_GOLOMB],
  minKStart, minBitsKStart, bitsKStart;
  
#if STAR_CROSS_SHAPES_LUMA
  pDepthInt = pDepthIntTabShapes[fl];
#else
  pDepthInt = pDepthIntTab[fl-2];
#endif
  
  maxScanVal = 0;
  for(i = 0; i < sqrFiltLength; i++)
  {
    maxScanVal = max(maxScanVal, pDepthInt[i]);
  }
  
  // vlc for all
  memset(bitsCoeffScan, 0, MAX_SCAN_VAL * MAX_EXP_GOLOMB * sizeof(int));
  for(ind=0; ind<filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {     
      scanPos=pDepthInt[i]-1;
      coeffVal=abs(pDiffQFilterCoeffIntPP[ind][i]);
      for (k=1; k<15; k++)
      {
        bitsCoeffScan[scanPos][k]+=lengthGolomb(coeffVal, k);
      }
    }
  }
  
  minBitsKStart = 0;
  minKStart = -1;
  for(k = 1; k < 8; k++)
  { 
    bitsKStart = 0; 
    kStart = k;
    for(scanPos = 0; scanPos < maxScanVal; scanPos++)
    {
      kMin = kStart; 
      minBits = bitsCoeffScan[scanPos][kMin];
      
      if(bitsCoeffScan[scanPos][kStart+1] < minBits)
      {
        kMin = kStart + 1; 
        minBits = bitsCoeffScan[scanPos][kMin];
      }
      kStart = kMin;
      bitsKStart += minBits;
    }
    if((bitsKStart < minBitsKStart) || (k == 1))
    {
      minBitsKStart = bitsKStart;
      minKStart = k;
    }
  }
  
  kStart = minKStart; 
  for(scanPos = 0; scanPos < maxScanVal; scanPos++)
  {
    kMin = kStart; 
    minBits = bitsCoeffScan[scanPos][kMin];
    
    if(bitsCoeffScan[scanPos][kStart+1] < minBits)
    {
      kMin = kStart + 1; 
      minBits = bitsCoeffScan[scanPos][kMin];
    }
    
    kMinTab[scanPos] = kMin;
    kStart = kMin;
  }
  
  // Coding parameters
  //  len += lengthFilterCodingParams(minKStart, maxScanVal, kMinTab, createBitstream);
  len += (3 + maxScanVal);
  
  // Filter coefficients
  len += lengthFilterCoeffs(sqrFiltLength, filters_per_group, pDepthInt, pDiffQFilterCoeffIntPP, 
                            kMinTab, createBitstream);
  
  return len;
}

Int TEncAdaptiveLoopFilter::lengthGolomb(int coeffVal, int k)
{
  int m = 2 << (k - 1);
  int q = coeffVal / m;
  if(coeffVal != 0)
    return(q + 2 + k);
  else
    return(q + 1 + k);
}

Int TEncAdaptiveLoopFilter::lengthPredFlags(int force0, int predMethod, int codedVarBins[NO_VAR_BINS], 
                                            int filters_per_group, int createBitstream)
{
  int bit_cnt = 0;
  
  if(force0)
    bit_cnt = 2 + filters_per_group;
  else
    bit_cnt = 2;
  return bit_cnt;
  
}
//important
Int TEncAdaptiveLoopFilter::lengthFilterCoeffs(int sqrFiltLength, int filters_per_group, int pDepthInt[], 
                                               int **FilterCoeff, int kMinTab[], int createBitstream)
{
  int ind, scanPos, i;
  int bit_cnt = 0;
  
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      scanPos = pDepthInt[i] - 1;
      bit_cnt += lengthGolomb(abs(FilterCoeff[ind][i]), kMinTab[scanPos]);
    }
  }
  return bit_cnt;
}

Void   TEncAdaptiveLoopFilter::xEncALFLuma_qc ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost )
{
  //pcPicDec: extended decoded
  //pcPicRest: original decoded: filtered signal will be stored
  
  UInt64  uiRate;
  UInt64  uiDist;
  Double dCost;
#if !MQT_ALF_NPASS
  Int    Height = pcPicOrg->getHeight();
  Int    Width = pcPicOrg->getWidth();
#endif
  Int    LumaStride = pcPicOrg->getStride();
  imgpel* pOrg = (imgpel*) pcPicOrg->getLumaAddr();
  imgpel* pRest = (imgpel*) pcPicRest->getLumaAddr();
  imgpel* pDec = (imgpel*) pcPicDec->getLumaAddr();

#if !STAR_CROSS_SHAPES_LUMA
  Int tap               = ALF_MIN_NUM_TAP;
  m_pcTempAlfParam->tap = tap;
#if TI_ALF_MAX_VSIZE_7
  m_pcTempAlfParam->tapV = TComAdaptiveLoopFilter::ALFTapHToTapV(m_pcTempAlfParam->tap);
  m_pcTempAlfParam->num_coeff = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(m_pcTempAlfParam->tap);
#else
  m_pcTempAlfParam->num_coeff = (Int)tap*tap/4 + 2; 
#endif
#endif

#if MQT_BA_RA


#if MQT_ALF_NPASS
  static Bool   bFirst = true;
  static Int*   bestVarIndTab;
  if(bFirst)
  {
    if(m_iALFEncodePassReduction)
    {
      bestVarIndTab = new Int[NO_VAR_BINS];
    }
    bFirst = false;
  }
#endif

  Double    dMinMethodCost  = MAX_DOUBLE;
  UInt64    uiMinMethodDist = MAX_UINT;
  UInt64    uiMinMethodRate = MAX_UINT;
  Int       iBestClassMethod = ALF_RA;
  Double    adExtraCostReduction[NUM_ALF_CLASS_METHOD];
  ALFParam  cFrmAlfParam        [NUM_ALF_CLASS_METHOD];
  ALFParam* pcAlfParam = NULL;

  for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
  {
    pcAlfParam = &(cFrmAlfParam[i]);
    allocALFParam(pcAlfParam);

    pcAlfParam->alf_flag        = 1;
    pcAlfParam->chroma_idc      = 0;
    pcAlfParam->cu_control_flag = 0;
#if !STAR_CROSS_SHAPES_LUMA
    pcAlfParam->tap = tap;
#if TI_ALF_MAX_VSIZE_7
    pcAlfParam->tapV = TComAdaptiveLoopFilter::ALFTapHToTapV(pcAlfParam->tap);
    pcAlfParam->num_coeff = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(pcAlfParam->tap);
#else
    pcAlfParam->num_coeff = (Int)tap*tap/4 + 2; 
#endif
#endif

    switch(i)
    {
    case ALF_RA:
      {
        adExtraCostReduction[i] = (double)(m_im_height * m_im_width) * m_dLambdaLuma * 2.0 / 4096.0;
      }
      break;
    case ALF_BA:
      {
        adExtraCostReduction[i] = 0.0;
      }
      break;
    default:
      {
        printf("Not a support adaptation method\n");
        assert(0);
        exit(-1);
      }
    }
  }


  for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
  {
    m_uiVarGenMethod = i;

    pcAlfParam       = &(cFrmAlfParam[m_uiVarGenMethod]);
    m_varImg         = m_varImgMethods[m_uiVarGenMethod];
  
    pcAlfParam->alf_pcr_region_flag = m_uiVarGenMethod;

#if MQT_ALF_NPASS
    if(m_iALFEncodePassReduction)
    {
      m_aiFilterCoeffSaved = m_aiFilterCoeffSavedMethods[m_uiVarGenMethod];
#if STAR_CROSS_SHAPES_LUMA
      m_iPreviousFilterShape = m_iPreviousFilterShapeMethods[m_uiVarGenMethod];
#endif
    }

    setInitialMask(pcPicOrg, pcPicDec);
#else
    for (Int y=0; y<Height; y++)
    {
      for (Int x=0; x<Width; x++)
      {
        m_maskImg[y][x] = 1;
      }
    }
#if MTK_NONCROSS_INLOOP_FILTER
    if(!m_bUseNonCrossALF)
      calcVar(0, 0, m_varImg, pDec, 9/2, VAR_SIZE, Height, Width, LumaStride);
    else
      calcVarforSlices(m_varImg, pDec, 9/2, VAR_SIZE, LumaStride);
#else
    calcVar(m_varImg, pDec, 9/2, VAR_SIZE, Height, Width, LumaStride);
#endif
#endif

#if MQT_ALF_NPASS
    if(m_iALFEncodePassReduction == 0)
    {
#endif
#if STAR_CROSS_SHAPES_LUMA
      static Int best_filter_shape = 0;
      if (m_uiVarGenMethod == 0)
      {
        UInt64 MinRate_Shape0 = MAX_INT;
        UInt64 MinDist_Shape0 = MAX_INT;
        Double MinCost_Shape0 = MAX_DOUBLE;
          
        UInt64 MinRate_Shape1 = MAX_INT;
        UInt64 MinDist_Shape1 = MAX_INT;
        Double MinCost_Shape1 = MAX_DOUBLE;
          
        for (Int filter_shape = 0; filter_shape < 2 ;filter_shape ++)
        {
          pcAlfParam->realfiltNo = filter_shape;
          pcAlfParam->num_coeff = m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filter_shape];
          xFirstFilteringFrameLuma(pOrg, pDec, (imgpel*)m_pcPicYuvTmp->getLumaAddr(), pcAlfParam, pcAlfParam->realfiltNo, LumaStride);
          xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, pcAlfParam, uiRate, uiDist, dCost);
          if (filter_shape == 0)
          {
            // copy Shape0
            MinRate_Shape0 = uiRate;
            MinDist_Shape0 = uiDist;
            MinCost_Shape0 = dCost;
            m_pcPicYuvTmp->copyToPicLuma(pcPicYuvRecShape0);
            copyALFParam(pcAlfParamShape0, pcAlfParam);
          }
          else //if (filter_shape == 1)
          {
            // copy Shape1
            MinRate_Shape1 = uiRate;
            MinDist_Shape1 = uiDist;
            MinCost_Shape1  = dCost;
            m_pcPicYuvTmp->copyToPicLuma(pcPicYuvRecShape1);
            copyALFParam(pcAlfParamShape1, pcAlfParam);
          }
        }
          
        if (MinCost_Shape0 <= MinCost_Shape1)
        {
          pcPicYuvRecShape0->copyToPicLuma(m_pcPicYuvTmp);
          copyALFParam(pcAlfParam, pcAlfParamShape0);
          uiRate = MinRate_Shape0;
          uiDist = MinDist_Shape0;
          dCost = MinCost_Shape0;
          best_filter_shape = 0;
        }
        else //if (MinCost_Shape1 < MinCost_Shape0)
        {
          pcPicYuvRecShape1->copyToPicLuma(m_pcPicYuvTmp);
          copyALFParam(pcAlfParam, pcAlfParamShape1);
          uiRate = MinRate_Shape1;
          uiDist = MinDist_Shape1;
          dCost = MinCost_Shape1;
          best_filter_shape = 1;
        }
      }
      else
      {
        pcAlfParam->realfiltNo = best_filter_shape;
        pcAlfParam->num_coeff = m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[pcAlfParam->realfiltNo];
        xFirstFilteringFrameLuma(pOrg, pDec, (imgpel*)m_pcPicYuvTmp->getLumaAddr(), pcAlfParam, pcAlfParam->realfiltNo, LumaStride);
        xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, pcAlfParam, uiRate, uiDist, dCost);
      }
#else
      xFirstFilteringFrameLuma(pOrg, pDec, (imgpel*)m_pcPicYuvTmp->getLumaAddr(), pcAlfParam, pcAlfParam->tap, LumaStride); 
      xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, pcAlfParam, uiRate, uiDist, dCost); 
#endif      
#if MQT_ALF_NPASS
    }
    else
    {
      xFirstEstimateFilteringFrameLumaAllTap(pOrg, pDec, LumaStride, pcAlfParam, uiRate, uiDist, dCost);
    }
#endif

    dCost -= adExtraCostReduction[m_uiVarGenMethod];

    if(dCost < dMinMethodCost)
    {
      iBestClassMethod = m_uiVarGenMethod;
      dMinMethodCost = dCost;
      uiMinMethodRate= uiRate;
      uiMinMethodDist = uiDist;

#if MQT_ALF_NPASS
      if(m_iALFEncodePassReduction == 0)
      {
#endif
        m_pcPicYuvTmp->copyToPicLuma(pcPicRest);
#if MQT_ALF_NPASS
      }
      else
      {
        ::memcpy(bestVarIndTab, m_varIndTab, sizeof(Int)*NO_VAR_BINS);
      }
#endif

    }  
  }

  m_uiVarGenMethod = iBestClassMethod;
  dMinMethodCost += adExtraCostReduction[m_uiVarGenMethod];
  m_varImg= m_varImgMethods[m_uiVarGenMethod];

  pcAlfParam = &(cFrmAlfParam[m_uiVarGenMethod]);

  ALFParam  cAlfParamWithBestMethod;
  allocALFParam(&cAlfParamWithBestMethod);  


#if MQT_ALF_NPASS
  if(m_iALFEncodePassReduction ==0)
  {
#endif
    copyALFParam(&cAlfParamWithBestMethod, pcAlfParam); 
#if MQT_ALF_NPASS
  }
  else
  {
    ::memcpy(m_varIndTab, bestVarIndTab, sizeof(Int)*NO_VAR_BINS);
    m_aiFilterCoeffSaved = m_aiFilterCoeffSavedMethods[m_uiVarGenMethod];

    cAlfParamWithBestMethod.alf_flag = 1;
    cAlfParamWithBestMethod.cu_control_flag = 0;
    cAlfParamWithBestMethod.chroma_idc = 0;
    cAlfParamWithBestMethod.alf_pcr_region_flag = m_uiVarGenMethod;
#if STAR_CROSS_SHAPES_LUMA
    cAlfParamWithBestMethod.realfiltNo= pcAlfParam->realfiltNo;
    cAlfParamWithBestMethod.num_coeff = m_sqrFiltLengthTab[cAlfParamWithBestMethod.realfiltNo]; 
#else
    cAlfParamWithBestMethod.tap = pcAlfParam->tap;
    cAlfParamWithBestMethod.num_coeff = pcAlfParam->num_coeff;
#if TI_ALF_MAX_VSIZE_7
    cAlfParamWithBestMethod.tapV = pcAlfParam->tapV;
#endif
#endif
    reconstructFilterCoeffs( pcAlfParam, m_filterCoeffSym, NUM_BITS);
    xcalcPredFilterCoeff(pcAlfParam->realfiltNo);
#if MTK_NONCROSS_INLOOP_FILTER
    if(!m_bUseNonCrossALF)
      xfilterFrame_en(0, 0, m_im_height, m_im_width, pDec, pRest, pcAlfParam->realfiltNo, LumaStride);
    else
      xfilterSlices_en(pDec, pRest, pcAlfParam->realfiltNo, LumaStride);
#else
    xfilterFrame_en(pDec, pRest, pcAlfParam->realfiltNo, LumaStride);
#endif

    xcodeFiltCoeff(m_filterCoeffSym, pcAlfParam->realfiltNo, m_varIndTab, pcAlfParam->filters_per_group,0, &cAlfParamWithBestMethod);


    xCalcRDCost(pcPicOrg, pcPicRest, &cAlfParamWithBestMethod, uiMinMethodRate, uiMinMethodDist, dMinMethodCost);

  }
#endif

  if(dMinMethodCost < rdMinCost )
  {
    ruiMinRate = uiMinMethodRate;
    ruiMinDist = uiMinMethodDist;
    rdMinCost =  dMinMethodCost;
    copyALFParam(m_pcBestAlfParam, &cAlfParamWithBestMethod); 
  }

  freeALFParam(&cAlfParamWithBestMethod);
  for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
  {
    freeALFParam(&cFrmAlfParam[i]);
  }




#else  

#if MQT_ALF_NPASS
  setInitialMask(pcPicOrg, pcPicDec);
#else
  for (Int i=0; i<Height; i++)
  {
    for (Int j=0; j<Width; j++)
    {
      m_maskImg[i][j] = 1;
    }
  }
#if STAR_CROSS_SHAPES_LUMA
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
  {
    calcVar(0, 0, m_varImg, pDec, FILTER_LENGTH/2, VAR_SIZE, Height, Width, LumaStride);
  }
  else
  {
    calcVarforSlices(m_varImg, pDec, FILTER_LENGTH/2, VAR_SIZE, LumaStride);
  }
#else
  calcVar(m_varImg, pDec, FILTER_LENGTH/2, VAR_SIZE, Height, Width, LumaStride);
#endif
#else
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
    calcVar(0, 0, m_varImg, pDec, 9/2, VAR_SIZE, Height, Width, LumaStride);
  else
    calcVarforSlices(m_varImg, pDec, 9/2, VAR_SIZE, LumaStride);
#else
  calcVar(m_varImg, pDec, 9/2, VAR_SIZE, Height, Width, LumaStride);
#endif
#endif
#endif

#if STAR_CROSS_SHAPES_LUMA
  if(m_iALFEncodePassReduction == 0)
  {
    UInt64 MinRate_Shape0 = MAX_INT;
    UInt64 MinDist_Shape0 = MAX_INT;
    Double MinCost_Shape0 = MAX_DOUBLE;

    UInt64 MinRate_Shape1 = MAX_INT;
    UInt64 MinDist_Shape1 = MAX_INT;
    Double dMinCost_Shape1 = MAX_DOUBLE;

    for (Int filter_shape = 0; filter_shape < 2 ;filter_shape ++)
    {
      m_pcTempAlfParam->realfiltNo = filter_shape;
      m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filter_shape];  
      xFirstFilteringFrameLuma(pOrg, pDec, pRest, m_pcTempAlfParam, m_pcTempAlfParam->realfiltNo, LumaStride); 
      xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, m_pcTempAlfParam, uiRate, uiDist, dCost); 
      if (filter_shape == 0)
      {    
        // copy Shape0
        MinRate_Shape0 = uiRate;
        MinDist_Shape0 = uiDist;
        MinCost_Shape0 = dCost;
        m_pcPicYuvTmp->copyToPicLuma(pcPicYuvRecShape0);
        copyALFParam(pcAlfParamShape0, m_pcTempAlfParam);
      }
      else //if (filter_shape == 1) 
      {
        // copy Shape1
        MinRate_Shape1 = uiRate;
        MinDist_Shape1 = uiDist;
        MinCost_Shape1 = dCost;
        m_pcPicYuvTmp->copyToPicLuma(pcPicYuvRecShape1);
        copyALFParam(pcAlfParamShape1, m_pcTempAlfParam);
      }
    }
    if (MinCost_Shape0 <= MinCost_Shape1)
    {
      pcPicYuvRecShape0->copyToPicLuma(m_pcPicYuvTmp);
      copyALFParam(m_pcTempAlfParam, pcAlfParamShape0);
      uiRate = MinRate_Shape0;
      uiDist = MinDist_Shape0;
      dCost = MinCost_Shape0;
    }
    else //if (MinCost_Shape1 < MinCost_Shape0)
    {
      pcPicYuvRecShape1->copyToPicLuma(m_pcPicYuvTmp);
      copyALFParam(m_pcTempAlfParam, pcAlfParamShape1);
      uiRate = MinRate_Shape1;
      uiDist = MinDist_Shape1;
      dCost = MinCost_Shape1;
    }
  }
  else
  {
    xFirstFilteringFrameLumaAllTap(pOrg, pDec, pRest, LumaStride);
    xCalcRDCost(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiRate, uiDist, dCost); // change this function final coding 
  }
#else
#if MQT_ALF_NPASS
  if(m_iALFEncodePassReduction)
  {
    xFirstFilteringFrameLumaAllTap(pOrg, pDec, pRest, LumaStride);
  }
  else
#endif
    xFirstFilteringFrameLuma(pOrg, pDec, pRest, m_pcTempAlfParam, m_pcTempAlfParam->tap, LumaStride); 

  xCalcRDCost(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiRate, uiDist, dCost); // change this function final coding 
#endif
  
  if( dCost < rdMinCost)
  {
    ruiMinRate = uiRate;
    ruiMinDist = uiDist;
    rdMinCost = dCost;
    copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam); 
  } 

#endif
}

Void   TEncAdaptiveLoopFilter::xFirstFilteringFrameLuma(imgpel* ImgOrg, imgpel* ImgDec, imgpel* ImgRest, ALFParam* ALFp, Int tap, Int Stride)
{
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
    xstoreInBlockMatrix(0, 0, m_im_height, m_im_width, true, true, ImgOrg, ImgDec, tap, Stride);
  else
    xstoreInBlockMatrixforSlices(ImgOrg, ImgDec, tap, Stride);
#else
  xstoreInBlockMatrix(ImgOrg, ImgDec, tap, Stride);
#endif


  xFilteringFrameLuma_qc(ImgOrg, ImgDec, ImgRest, ALFp, tap, Stride);
}


#if MTK_NONCROSS_INLOOP_FILTER
Void   TEncAdaptiveLoopFilter::xstoreInBlockMatrix(Int ypos, Int xpos, Int iheight, Int iwidth, Bool bResetBlockMatrix, Bool bSymmCopyBlockMatrix, imgpel* ImgOrg, imgpel* ImgDec, Int tap, Int Stride)
#else
Void   TEncAdaptiveLoopFilter::xstoreInBlockMatrix(imgpel* ImgOrg, imgpel* ImgDec, Int tap, Int Stride)
#endif
{
#if MQT_BA_RA
  Int var_step_size_w = VAR_SIZE_W;
  Int var_step_size_h = VAR_SIZE_H;
#endif

  Int i,j,k,l,varInd,ii,jj;
#if STAR_CROSS_SHAPES_LUMA
  Int sqrFiltLength = m_pcTempAlfParam->num_coeff;
  Int fl2 = FILTER_LENGTH/2; //extended size at each side of the frame
  Int flH = 0;// horizontal_tap / 2
  Int flV = 0;// vertical_tap / 2
  Int m = 0;
#else
  Int fl =tap/2;
#if TI_ALF_MAX_VSIZE_7
  Int flV = TComAdaptiveLoopFilter::ALFFlHToFlV(fl);
  Int sqrFiltLength = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(tap);
#else
  Int sqrFiltLength=(((tap*tap)/4 + 1) + 1);
#endif
  Int fl2=9/2; //extended size at each side of the frame
#endif
  Int ELocal[MAX_SQR_FILT_LENGTH];
  Int yLocal;
  Int *p_pattern;
  Int filtNo =2; 
  double **E,*yy;
#if MTK_NONCROSS_INLOOP_FILTER
  static Int count_valid;
#else
  Int count_valid=0;
#endif
#if STAR_CROSS_SHAPES_LUMA
  filtNo=tap;
  if (tap == 0)
  {
    flH   = 2;
    flV   = 2;
  }
  else //if (tap == 1 || tap == 2)
  {
    flH   = 5;
    flV   = 2;
  }
#else
  if (tap==9)
    filtNo =0;
  else if (tap==7)
    filtNo =1;
#endif
  
#if MQT_ALF_NPASS && MQT_BA_RA
  imgpel regionOfInterested = (m_iDesignCurrentFilter ==1)?(1):(0);
#endif

#if STAR_CROSS_SHAPES_LUMA
  p_pattern= patternTabShapes[filtNo];
#else
  p_pattern= m_patternTab[filtNo];
#endif
  
#if MTK_NONCROSS_INLOOP_FILTER
  if(bResetBlockMatrix)
  {
    count_valid = 0;
#endif
  memset( m_pixAcc, 0,sizeof(double)*NO_VAR_BINS);
  for (varInd=0; varInd<NO_VAR_BINS; varInd++)
  {
    memset(m_yGlobalSym[filtNo][varInd],0,sizeof(double)*MAX_SQR_FILT_LENGTH);
    for (k=0; k<sqrFiltLength; k++)
    {
      memset(m_EGlobalSym[filtNo][varInd][k],0,sizeof(double)*MAX_SQR_FILT_LENGTH);
    }
  }
  for (i = fl2; i < m_im_height+fl2; i++)
  {
    for (j = fl2; j < m_im_width+fl2; j++)
    {
#if MQT_ALF_NPASS && MQT_BA_RA
      if (m_maskImg[i-fl2][j-fl2] == regionOfInterested)
#else
      if (m_maskImg[i-fl2][j-fl2] == 1)
#endif
      {
        count_valid++;
      }
    }
  }
#if MTK_NONCROSS_INLOOP_FILTER
  }
#endif

#if STAR_CROSS_SHAPES_LUMA
  if (tap == 0)
  {
#if MTK_NONCROSS_INLOOP_FILTER
    for (i= ypos; i< ypos + iheight; i++)
    {
      for (j= xpos; j< xpos + iwidth; j++)
      {
#else
    for (i=0; i<m_im_height; i++)
    {
      for (j=0; j<m_im_width; j++)
      {
#endif
#if MQT_ALF_NPASS
#if MQT_BA_RA
        if (m_maskImg[i][j] != regionOfInterested && count_valid > 0)
        {

        }
        else
        {
#else
        Int condition = (m_maskImg[i][j] == 1);
        if (m_iDesignCurrentFilter)
        {
          condition = (m_maskImg[i][j] == 0 && count_valid > 0);
        }
        if(!condition)
        {
#endif
#else
        if (m_maskImg[i][j] == 0 && count_valid > 0)
        { 
        }
        else
        {
#endif
#if MQT_BA_RA
          varInd = m_varImg[i/var_step_size_h][j/var_step_size_w];
#else
          varInd=min(m_varImg[i][j], NO_VAR_BINS-1);
#endif
          k=0; 
          memset(ELocal, 0, sqrFiltLength*sizeof(int));
          
          for (ii=-flV; ii<0; ii++)
          {
            m = (ii == -flV)? 2: 1;
            for (jj=-m; jj<=m; jj++)
            {
              if ((m == 1) || (m == 2 && jj%2 == 0))
              {
                ELocal[p_pattern[k++]]+=(ImgDec[(i+ii)*Stride + (j+jj)]+ImgDec[(i-ii)*Stride + (j-jj)]);
              }
            }
          }
          
          for (jj=-flH; jj<0; jj++)
            ELocal[p_pattern[k++]]+=(ImgDec[(i)*Stride + (j+jj)]+ImgDec[(i)*Stride + (j-jj)]);

          ELocal[p_pattern[k++]]+=ImgDec[(i)*Stride + (j)];
          ELocal[sqrFiltLength-1]=1;
          yLocal=ImgOrg[(i)*Stride + (j)];

          m_pixAcc[varInd]+=(yLocal*yLocal);
          E= m_EGlobalSym[filtNo][varInd];
          yy= m_yGlobalSym[filtNo][varInd];

          for (k=0; k<sqrFiltLength; k++)
          {
            for (l=k; l<sqrFiltLength; l++)
              E[k][l]+=(double)(ELocal[k]*ELocal[l]);
            yy[k]+=(double)(ELocal[k]*yLocal);
          }
        }
      }
    }
  }
  else if (tap == 1)
  {
#if MTK_NONCROSS_INLOOP_FILTER
    for (i= ypos; i< ypos + iheight; i++)
    {
      for (j= xpos; j< xpos + iwidth; j++)
      {
#else
    for (i=0; i<m_im_height; i++)
    {
      for (j=0; j<m_im_width; j++)
      {
#endif
#if MQT_ALF_NPASS
#if MQT_BA_RA
        if (m_maskImg[i][j] != regionOfInterested && count_valid > 0)
        {

        }
        else
        {

#else
        Int condition = (m_maskImg[i][j] == 1);
        if (m_iDesignCurrentFilter)
        {
          condition = (m_maskImg[i][j] == 0 && count_valid > 0);
        }
        if(!condition)
        {
#endif
#else
        if (m_maskImg[i][j] == 0 && count_valid > 0)
        {

        }
        else
        {
#endif
#if MQT_BA_RA
          varInd = m_varImg[i/var_step_size_h][j/var_step_size_w];
#else
          varInd= min(m_varImg[i][j], NO_VAR_BINS-1);
#endif
          k=0; 
          memset(ELocal, 0, sqrFiltLength*sizeof(int));
          
          for (ii=-flV; ii<0; ii++)
            ELocal[p_pattern[k++]]+=(ImgDec[(i+ii)*Stride + j]+ImgDec[(i-ii)*Stride + j]);
          
          for (jj=-flH; jj<0; jj++)
            ELocal[p_pattern[k++]]+=(ImgDec[(i)*Stride + (j+jj)]+ImgDec[(i)*Stride + (j-jj)]);

          ELocal[p_pattern[k++]]+=ImgDec[(i)*Stride + (j)];
          ELocal[sqrFiltLength-1]=1;
          yLocal=ImgOrg[(i)*Stride + (j)];

          m_pixAcc[varInd]+=(yLocal*yLocal);
          E= m_EGlobalSym[filtNo][varInd];
          yy= m_yGlobalSym[filtNo][varInd];

          for (k=0; k<sqrFiltLength; k++)
          {
            for (l=k; l<sqrFiltLength; l++)
              E[k][l]+=(double)(ELocal[k]*ELocal[l]);
            yy[k]+=(double)(ELocal[k]*yLocal);
          }
        }
      }
    }
  }
  else //if (tap == 2)
  {
#if MTK_NONCROSS_INLOOP_FILTER
    for (i= ypos; i< ypos + iheight; i++)
    {
      for (j= xpos; j< xpos + iwidth; j++)
      {
#else
    for (i=0; i<m_im_height; i++)
    {
      for (j=0; j<m_im_width; j++)
      {
#endif
#if MQT_ALF_NPASS
#if MQT_BA_RA
        if (m_maskImg[i][j] != regionOfInterested && count_valid > 0)
        {

        }
        else
        {

#else
        Int condition = (m_maskImg[i][j] == 1);
        if (m_iDesignCurrentFilter)
        {
          condition = (m_maskImg[i][j] == 0 && count_valid > 0);
        }
        if(!condition)
        {
#endif
#else
        if (m_maskImg[i][j] == 0 && count_valid > 0)
        {

        }
        else
        {
#endif
#if MQT_BA_RA
          varInd = m_varImg[i/var_step_size_h][j/var_step_size_w];
#else
          varInd= min(m_varImg[i][j], NO_VAR_BINS-1);
#endif
          k=0; 
          memset(ELocal, 0, sqrFiltLength*sizeof(int));
          
          for (ii=-flV; ii<0; ii++)
          {
            for (jj=-flH; jj<=flH; jj++)
              ELocal[p_pattern[k++]]+=(ImgDec[(i+ii)*Stride + (j+jj)]+ImgDec[(i-ii)*Stride + (j-jj)]);
          }
          for (jj=-flH; jj<0; jj++)
            ELocal[p_pattern[k++]]+=(ImgDec[(i)*Stride + (j+jj)]+ImgDec[(i)*Stride + (j-jj)]);

          ELocal[p_pattern[k++]]+=ImgDec[(i)*Stride + (j)];
          ELocal[sqrFiltLength-1]=1;
          yLocal=ImgOrg[(i)*Stride + (j)];

          m_pixAcc[varInd]+=(yLocal*yLocal);
          E= m_EGlobalSym[filtNo][varInd];
          yy= m_yGlobalSym[filtNo][varInd];

          for (k=0; k<sqrFiltLength; k++)
          {
            for (l=k; l<sqrFiltLength; l++)
              E[k][l]+=(double)(ELocal[k]*ELocal[l]);
            yy[k]+=(double)(ELocal[k]*yLocal);
          }
        }
      }
    }
  }
#else
  {
#if MTK_NONCROSS_INLOOP_FILTER
    for (i= ypos; i< ypos + iheight; i++)
    {
      for (j= xpos; j< xpos + iwidth; j++)
      {
#else
    for (i=0; i<m_im_height; i++)
    {
      for (j=0; j<m_im_width; j++)
      {
#endif
#if MQT_ALF_NPASS
#if MQT_BA_RA
        if (m_maskImg[i][j] != regionOfInterested && count_valid > 0)
        {

        }
        else
        {

#else
        Int condition = (m_maskImg[i][j] == 1);
        if (m_iDesignCurrentFilter)
        {
          condition = (m_maskImg[i][j] == 0 && count_valid > 0);
        }
        if(!condition)
        {
#endif

#else
        if (m_maskImg[i][j] == 0 && count_valid > 0)
        {

        }
        else
        {
#endif
#if MQT_BA_RA
          varInd = m_varImg[i/var_step_size_h][j/var_step_size_w];
#else
          varInd=min(m_varImg[i][j], NO_VAR_BINS-1);
#endif
          k=0; 
          memset(ELocal, 0, sqrFiltLength*sizeof(int));
#if TI_ALF_MAX_VSIZE_7
          for (ii = -flV; ii < 0; ii++)
#else
          for (ii=-fl; ii<0; ii++)
#endif
          {
            for (jj=-fl-ii; jj<=fl+ii; jj++)
            {  
              ELocal[p_pattern[k++]]+=(ImgDec[(i+ii)*Stride + (j+jj)]+ImgDec[(i-ii)*Stride + (j-jj)]);
            }
          }
          for (jj=-fl; jj<0; jj++)
            ELocal[p_pattern[k++]]+=(ImgDec[(i)*Stride + (j+jj)]+ImgDec[(i)*Stride + (j-jj)]);
          ELocal[p_pattern[k++]]+=ImgDec[(i)*Stride + (j)];
          ELocal[sqrFiltLength-1]=1;
          yLocal=ImgOrg[(i)*Stride + (j)];

          m_pixAcc[varInd]+=(yLocal*yLocal);
          E= m_EGlobalSym[filtNo][varInd];
          yy= m_yGlobalSym[filtNo][varInd];

          for (k=0; k<sqrFiltLength; k++)
          {
            for (l=k; l<sqrFiltLength; l++)
              E[k][l]+=(double)(ELocal[k]*ELocal[l]);
            yy[k]+=(double)(ELocal[k]*yLocal);
          }
        }
      }
    }
  }
#endif

#if MTK_NONCROSS_INLOOP_FILTER
  if(bSymmCopyBlockMatrix)
  {
#endif

  // Matrix EGlobalSeq is symmetric, only part of it is calculated
  for (varInd=0; varInd<NO_VAR_BINS; varInd++)
  {
    double **pE = m_EGlobalSym[filtNo][varInd];
    for (k=1; k<sqrFiltLength; k++)
    {
      for (l=0; l<k; l++)
      {
        pE[k][l]=pE[l][k];
      }
    }
  }
#if MTK_NONCROSS_INLOOP_FILTER
  }
#endif

}

Void   TEncAdaptiveLoopFilter::xFilteringFrameLuma_qc(imgpel* ImgOrg, imgpel* imgY_pad, imgpel* ImgFilt, ALFParam* ALFp, Int tap, Int Stride)
{
  int  filtNo,filters_per_fr;
  static double **ySym, ***ESym;
  int lambda_val = (Int) m_dLambdaLuma;
  lambda_val = lambda_val * (1<<(2*g_uiBitIncrement));
#if STAR_CROSS_SHAPES_LUMA
  if (ALFp->realfiltNo == 0)
  {
    filtNo = 0;
  }
  else //if (ALFp->realfiltNo == 1)
  {
    filtNo = 1;
  }
#else
  if (tap==9)
    filtNo =0;
  else if (tap==7)
    filtNo =1;
  else
    filtNo=2;
#endif
  
  ESym=m_EGlobalSym[filtNo];  
  ySym=m_yGlobalSym[filtNo];
  
  xfindBestFilterVarPred(ySym, ESym, m_pixAcc, m_filterCoeffSym, m_filterCoeffSymQuant, filtNo, &filters_per_fr,
                         m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambda_val);
  
  // g_filterCoeffPrevSelected = g_filterCoeffSym
  xcalcPredFilterCoeff(filtNo);
  
  //filter the frame with g_filterCoeffPrevSelected
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
    xfilterFrame_en(0, 0, m_im_height, m_im_width, imgY_pad, ImgFilt, filtNo, Stride);
  else
    xfilterSlices_en(imgY_pad, ImgFilt, filtNo, Stride);
#else
  xfilterFrame_en(imgY_pad, ImgFilt, filtNo, Stride);
#endif

  xcodeFiltCoeff(m_filterCoeffSymQuant, filtNo, m_varIndTab, filters_per_fr,0, ALFp);
}

#if MTK_NONCROSS_INLOOP_FILTER
Void TEncAdaptiveLoopFilter::xfilterFrame_en(int ypos, int xpos, int iheight, int iwidth, imgpel* ImgDec, imgpel* ImgRest,int filtNo, int Stride)
#else
Void TEncAdaptiveLoopFilter::xfilterFrame_en(imgpel* ImgDec, imgpel* ImgRest,int filtNo, int Stride)
#endif
{
#if MQT_BA_RA
  imgpel *imgY_rec = ImgDec;
  imgpel *p_imgY_pad, *p_imgY_pad0;
  int var_step_size_w = VAR_SIZE_W;
  int var_step_size_h = VAR_SIZE_H;
  int i,j,y,x;
  int fl, sqrFiltLength;
#else
  int i,j,ii,jj,y,x;
  int  *pattern; 
#if STAR_CROSS_SHAPES_LUMA
  int fl, sqrFiltLength;
#else
  int fl, fl_temp, sqrFiltLength;
#endif
#endif
  int pixelInt;
  int offset = (1<<(NUM_BITS - 2));
  
#if !MQT_BA_RA
#if STAR_CROSS_SHAPES_LUMA
  Int flH = 0;// horizontal_tap / 2
  Int flV = 0;// vertical_tap / 2
  Int m = 0;
  pattern = patternTabFiltShapes[filtNo];
  if (filtNo == 0)
  {
    flH = 2;
    flV = 2;    
  }
  else //if (filtNo == 1)
  {
    flH = 5;
    flV = 2;    
  }
#else 
  pattern=m_patternTab_filt[filtNo];
  fl_temp=m_flTab[filtNo];
#if TI_ALF_MAX_VSIZE_7
  Int fl_tempV = TComAdaptiveLoopFilter::ALFFlHToFlV(fl_temp);
#endif
#endif
#endif
  sqrFiltLength=MAX_SQR_FILT_LENGTH;  fl=FILTER_LENGTH/2;
  
#if MTK_NONCROSS_INLOOP_FILTER
  for (y= ypos, i = fl+ ypos; i < ypos+ iheight+ fl; i++, y++)
  {
    for (x= xpos, j = fl+ xpos; j < xpos+ iwidth+ fl; j++, x++)
    {
#else
  for (y=0, i = fl; i < m_im_height+fl; i++, y++)
  {
    for (x=0, j = fl; j < m_im_width+fl; j++, x++)
    {
#endif
#if MQT_BA_RA
      int varInd=m_varImg[y/var_step_size_h][x/var_step_size_w];
#else
      int varInd=m_varImg[i-fl][j-fl];
#if !STAR_CROSS_SHAPES_LUMA
      imgpel *im1,*im2;
#endif
#endif
      int *coef = m_filterCoeffPrevSelected[varInd];
#if !MQT_BA_RA
#if !STAR_CROSS_SHAPES_LUMA
      pattern=m_patternTab_filt[filtNo];
#endif
#endif
      pixelInt= m_filterCoeffPrevSelected[varInd][sqrFiltLength-1]; 

#if MQT_BA_RA
#if STAR_CROSS_SHAPES_LUMA
      if (filtNo == 0)
      {
        p_imgY_pad = imgY_rec + (i-fl+2)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-2)*Stride;
        pixelInt += coef[3]* (p_imgY_pad[j-fl+2]+p_imgY_pad0[j-fl-2]);
        pixelInt += coef[5]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);
        pixelInt += coef[7]* (p_imgY_pad[j-fl-2]+p_imgY_pad0[j-fl+2]);

        p_imgY_pad = imgY_rec + (i-fl+1)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-1)*Stride;

        pixelInt += coef[15]* (p_imgY_pad[j-fl+1]+p_imgY_pad0[j-fl-1]);
        pixelInt += coef[16]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);
        pixelInt += coef[17]* (p_imgY_pad[j-fl-1]+p_imgY_pad0[j-fl+1]);

        p_imgY_pad = imgY_rec + (i-fl)*Stride;
        pixelInt += coef[25]* (p_imgY_pad[j-fl+2]+p_imgY_pad[j-fl-2]);
        pixelInt += coef[26]* (p_imgY_pad[j-fl+1]+p_imgY_pad[j-fl-1]);
        pixelInt += coef[27]* (p_imgY_pad[j-fl]);
      }
      else //(filtNo == 1)
      {
        p_imgY_pad = imgY_rec + (i-fl+2)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-2)*Stride;
        pixelInt += coef[5]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);

        p_imgY_pad = imgY_rec + (i-fl+1)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-1)*Stride;
        pixelInt += coef[16]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);

        p_imgY_pad = imgY_rec + (i-fl)*Stride;
        pixelInt += coef[22]* (p_imgY_pad[j-fl+5]+p_imgY_pad[j-fl-5]);
        pixelInt += coef[23]* (p_imgY_pad[j-fl+4]+p_imgY_pad[j-fl-4]);
        pixelInt += coef[24]* (p_imgY_pad[j-fl+3]+p_imgY_pad[j-fl-3]);
        pixelInt += coef[25]* (p_imgY_pad[j-fl+2]+p_imgY_pad[j-fl-2]);
        pixelInt += coef[26]* (p_imgY_pad[j-fl+1]+p_imgY_pad[j-fl-1]);
        pixelInt += coef[27]* (p_imgY_pad[j-fl]);
      }
#else
      if (filtNo == 2) //5x5
      {
        pixelInt += coef[22]* (imgY_rec[(i-fl+2)*Stride + j-fl]+imgY_rec[(i-fl-2)*Stride + j-fl]);

        pixelInt += coef[30]* (imgY_rec[(i-fl+1)*Stride + j-fl+1]+imgY_rec[(i-fl-1)*Stride + j-fl-1]);
        pixelInt += coef[31]* (imgY_rec[(i-fl+1)*Stride + j-fl]  +imgY_rec[(i-fl-1)*Stride + j-fl]);
        pixelInt += coef[32]* (imgY_rec[(i-fl+1)*Stride + j-fl-1]+imgY_rec[(i-fl-1)*Stride + j-fl+1]);

        pixelInt += coef[38]* (imgY_rec[(i-fl)*Stride + j-fl-2]+imgY_rec[(i-fl)*Stride + j-fl+2]);
        pixelInt += coef[39]* (imgY_rec[(i-fl)*Stride + j-fl-1]+imgY_rec[(i-fl)*Stride + j-fl+1]);
        pixelInt += coef[40]* (imgY_rec[(i-fl)*Stride + j-fl]);
      }
      else if (filtNo == 1) //7x7
      {
        pixelInt += coef[13]* (imgY_rec[(i-fl+3)*Stride + j-fl]+imgY_rec[(i-fl-3)*Stride + j-fl]);

        p_imgY_pad = imgY_rec + (i-fl+2)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-2)*Stride;
        pixelInt += coef[21]* (p_imgY_pad[j-fl+1]+p_imgY_pad0[j-fl-1]);
        pixelInt += coef[22]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);
        pixelInt += coef[23]* (p_imgY_pad[j-fl-1]+p_imgY_pad0[j-fl+1]);

        p_imgY_pad = imgY_rec + (i-fl+1)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-1)*Stride;
        pixelInt += coef[29]* (p_imgY_pad[j-fl+2]+p_imgY_pad0[j-fl-2]);
        pixelInt += coef[30]* (p_imgY_pad[j-fl+1]+p_imgY_pad0[j-fl-1]);
        pixelInt += coef[31]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);
        pixelInt += coef[32]* (p_imgY_pad[j-fl-1]+p_imgY_pad0[j-fl+1]);
        pixelInt += coef[33]* (p_imgY_pad[j-fl-2]+p_imgY_pad0[j-fl+2]);

        p_imgY_pad = imgY_rec + (i-fl)*Stride;
        pixelInt += coef[37]* (p_imgY_pad[j-fl+3]+p_imgY_pad[j-fl-3]);
        pixelInt += coef[38]* (p_imgY_pad[j-fl+2]+p_imgY_pad[j-fl-2]);
        pixelInt += coef[39]* (p_imgY_pad[j-fl+1]+p_imgY_pad[j-fl-1]);
        pixelInt += coef[40]* (p_imgY_pad[j-fl]);

      }
      else
      {
#if !TI_ALF_MAX_VSIZE_7
        pixelInt += coef[4]* (imgY_rec[(i-fl+4)*Stride + j-fl]+imgY_rec[(i-fl-4)*Stride + j-fl]);
#endif          
        p_imgY_pad = imgY_rec + (i-fl+3)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-3)*Stride;
        pixelInt += coef[12]* (p_imgY_pad[j-fl+1]+p_imgY_pad0[j-fl-1]);
        pixelInt += coef[13]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);
        pixelInt += coef[14]* (p_imgY_pad[j-fl-1]+p_imgY_pad0[j-fl+1]);

        p_imgY_pad = imgY_rec + (i-fl+2)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-2)*Stride;
        pixelInt += coef[20]* (p_imgY_pad[j-fl+2]+p_imgY_pad0[j-fl-2]);
        pixelInt += coef[21]* (p_imgY_pad[j-fl+1]+p_imgY_pad0[j-fl-1]);
        pixelInt += coef[22]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);
        pixelInt += coef[23]* (p_imgY_pad[j-fl-1]+p_imgY_pad0[j-fl+1]);
        pixelInt += coef[24]* (p_imgY_pad[j-fl-2]+p_imgY_pad0[j-fl+2]);

        p_imgY_pad = imgY_rec + (i-fl+1)*Stride;
        p_imgY_pad0 = imgY_rec + (i-fl-1)*Stride;
        pixelInt += coef[28]* (p_imgY_pad[j-fl+3]+p_imgY_pad0[j-fl-3]);
        pixelInt += coef[29]* (p_imgY_pad[j-fl+2]+p_imgY_pad0[j-fl-2]);
        pixelInt += coef[30]* (p_imgY_pad[j-fl+1]+p_imgY_pad0[j-fl-1]);
        pixelInt += coef[31]* (p_imgY_pad[j-fl]+p_imgY_pad0[j-fl]);
        pixelInt += coef[32]* (p_imgY_pad[j-fl-1]+p_imgY_pad0[j-fl+1]);
        pixelInt += coef[33]* (p_imgY_pad[j-fl-2]+p_imgY_pad0[j-fl+2]);
        pixelInt += coef[34]* (p_imgY_pad[j-fl-3]+p_imgY_pad0[j-fl+3]);

        p_imgY_pad = imgY_rec + (i-fl)*Stride;
        pixelInt += coef[36]* (p_imgY_pad[j-fl+4]+p_imgY_pad[j-fl-4]);
        pixelInt += coef[37]* (p_imgY_pad[j-fl+3]+p_imgY_pad[j-fl-3]);
        pixelInt += coef[38]* (p_imgY_pad[j-fl+2]+p_imgY_pad[j-fl-2]);
        pixelInt += coef[39]* (p_imgY_pad[j-fl+1]+p_imgY_pad[j-fl-1]);
        pixelInt += coef[40]* (p_imgY_pad[j-fl]);

      }
#endif
#else
#if STAR_CROSS_SHAPES_LUMA
  if (filtNo == 0)
  {
    for (y=0, i = fl; i < m_im_height+fl; i++, y++)
    {
      for (x=0, j = fl; j < m_im_width+fl; j++, x++)
      {
        int varInd=m_varImg[i-fl][j-fl];
        imgpel *im1,*im2;
        int *coef = m_filterCoeffPrevSelected[varInd];
        pattern = patternTabFiltShapes[filtNo];
        pixelInt= m_filterCoeffPrevSelected[varInd][sqrFiltLength-1]; 
        for (ii=-flV; ii<0; ii++)
        {
           m = (ii == -flV)? 2: 1;
           im1= &(ImgDec[(y+ii)*Stride + x-m]);
           im2= &(ImgDec[(y-ii)*Stride + x+m]);
           for (jj=-m; jj<=m; jj++,im1++,im2--)
             if ((m == 1) || (m == 2 && jj%2 == 0))
             {
               pixelInt+=((*im1+ *im2)*coef[*(pattern++)]);
             }
        }
        im1= &(ImgDec[y*Stride + x-flH]);
        im2= &(ImgDec[y*Stride + x+flH]);    
        for (jj=-flH; jj<0; jj++,im1++,im2--)
          pixelInt+=((*im1+ *im2)*coef[*(pattern++)]);
        pixelInt+=(ImgDec[y*Stride + x]*coef[*(pattern++)]);

        pixelInt=(int)((pixelInt+offset) >> (NUM_BITS - 1));
        ImgRest[y*Stride + x] = Clip3(0, g_uiIBDI_MAX, pixelInt);
      }
    }
  }
  else //if (filtNo == 1)
  {
    for (y=0, i = fl; i < m_im_height+fl; i++, y++)
    {
      for (x=0, j = fl; j < m_im_width+fl; j++, x++)
      {
        int varInd=m_varImg[i-fl][j-fl];
        imgpel *im1,*im2;
        int *coef = m_filterCoeffPrevSelected[varInd];
        pattern = patternTabFiltShapes[filtNo];
        pixelInt= m_filterCoeffPrevSelected[varInd][sqrFiltLength-1];
        for (ii=-flV; ii<0; ii++)
        {
          im1= &(ImgDec[(y+ii)*Stride + x]);
          im2= &(ImgDec[(y-ii)*Stride + x]);
          pixelInt+=((*im1+ *im2)*coef[*(pattern++)]);
        }
        im1= &(ImgDec[y*Stride + x-flH]);
        im2= &(ImgDec[y*Stride + x+flH]);    
        for (jj=-flH; jj<0; jj++,im1++,im2--)
          pixelInt+=((*im1+ *im2)*coef[*(pattern++)]);
        pixelInt+=(ImgDec[y*Stride + x]*coef[*(pattern++)]);

        pixelInt=(int)((pixelInt+offset) >> (NUM_BITS - 1));
        ImgRest[y*Stride + x] = Clip3(0, g_uiIBDI_MAX, pixelInt);
      }
    }
  }
#else
#if TI_ALF_MAX_VSIZE_7
      for (ii = -fl_tempV; ii < 0; ii++)
#else
      for (ii=-fl_temp; ii<0; ii++)
#endif
      {
        im1= &(ImgDec[(y+ii)*Stride + x-fl_temp-ii]);
        im2= &(ImgDec[(y-ii)*Stride + x+fl_temp+ii]);
        for (jj=-fl_temp-ii; jj<=fl_temp+ii; jj++,im1++,im2--)
          pixelInt+=((*im1+ *im2)*coef[*(pattern++)]);
      }
      im1= &(ImgDec[y*Stride + x-fl_temp]);
      im2= &(ImgDec[y*Stride + x+fl_temp]);
      for (jj=-fl_temp; jj<0; jj++,im1++,im2--)
        pixelInt+=((*im1+ *im2)*coef[*(pattern++)]);
      pixelInt+=(ImgDec[y*Stride + x]*coef[*(pattern++)]);
#endif  
#endif
      pixelInt=(int)((pixelInt+offset) >> (NUM_BITS - 1));
      ImgRest[y*Stride + x] = Clip( pixelInt );
    }
  }
}

Void TEncAdaptiveLoopFilter::xfindBestFilterVarPred(double **ySym, double ***ESym, double *pixAcc, Int **filterCoeffSym, Int **filterCoeffSymQuant, Int filtNo, Int *filters_per_fr_best, Int varIndTab[], imgpel **imgY_rec, imgpel **varImg, imgpel **maskImg, imgpel **imgY_pad, double lambda_val)
{
  int filters_per_fr, firstFilt, forceCoeff0,
  interval[NO_VAR_BINS][2], intervalBest[NO_VAR_BINS][2];
  int i, k, varInd;
  static double ***E_temp, **y_temp, *pixAcc_temp;
  static int **FilterCoeffQuantTemp;
  double  error, lambda, lagrangian, lagrangianMin;
  
  int sqrFiltLength;
  int *weights;
  int numBits, coeffBits;
  double errorForce0CoeffTab[NO_VAR_BINS][2];
  int  codedVarBins[NO_VAR_BINS], createBistream /*, forceCoeff0 */;
  static int first=0;
  
  lambda = lambda_val;
  sqrFiltLength=MAX_SQR_FILT_LENGTH;
  
  if (first==0)
  {
    initMatrix3D_double(&E_temp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);
    initMatrix_double(&y_temp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); 
    pixAcc_temp = (double *) calloc(NO_VAR_BINS, sizeof(double));
    initMatrix_int(&FilterCoeffQuantTemp, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);
    first=1;
  }
  
#if STAR_CROSS_SHAPES_LUMA
  sqrFiltLength= m_sqrFiltLengthTab[filtNo] ;
  // in HM-3.x fl specifies the filter tap - in our proposal fl specifies the filter number (0: Cross5x5 - 1: Cross11x5)
  Int fl = filtNo;
  weights = weightsTabShapes[filtNo];               
#else   
  sqrFiltLength=m_sqrFiltLengthTab[filtNo];   
  Int fl = m_flTab[filtNo];
  weights=m_weightsTab[filtNo];               
#endif
  
  memcpy(pixAcc_temp,pixAcc,sizeof(double)*NO_VAR_BINS);
  for (varInd=0; varInd<NO_VAR_BINS; varInd++)
  {
    memcpy(y_temp[varInd],ySym[varInd],sizeof(double)*sqrFiltLength);
    for (k=0; k<sqrFiltLength; k++)
      memcpy(E_temp[varInd][k],ESym[varInd][k],sizeof(double)*sqrFiltLength);
  }
  
  // zero all variables 
  memset(varIndTab,0,sizeof(int)*NO_VAR_BINS);
  
  for(i = 0; i < NO_VAR_BINS; i++)
  {
    memset(filterCoeffSym[i],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
    memset(filterCoeffSymQuant[i],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
  }
  
  firstFilt=1;  lagrangianMin=0;
  filters_per_fr=NO_FILTERS;
  
  while(filters_per_fr>=1)
  {
    findFilterGroupingError(E_temp, y_temp, pixAcc_temp, interval, sqrFiltLength, filters_per_fr);
    findFilterCoeff(E_temp, y_temp, pixAcc_temp, filterCoeffSym, filterCoeffSymQuant, interval,
                    varIndTab, sqrFiltLength, filters_per_fr, weights, numBits=NUM_BITS,  errorForce0CoeffTab);
    lagrangian=xfindBestCoeffCodMethod(codedVarBins, &forceCoeff0, filterCoeffSymQuant, fl, 
                                       sqrFiltLength, filters_per_fr, errorForce0CoeffTab, &error, lambda);
    
    if (lagrangian<lagrangianMin || firstFilt==1)
    {
      firstFilt=0;
      lagrangianMin=lagrangian;

      (*filters_per_fr_best)=filters_per_fr;
      memcpy(intervalBest, interval, NO_VAR_BINS*2*sizeof(int));
    }
    filters_per_fr--;
  }
  
  findFilterCoeff(E_temp, y_temp, pixAcc_temp, filterCoeffSym, filterCoeffSymQuant, intervalBest,
                  varIndTab, sqrFiltLength, (*filters_per_fr_best), weights, numBits=NUM_BITS, errorForce0CoeffTab);
  
  xfindBestCoeffCodMethod(codedVarBins, &forceCoeff0, filterCoeffSymQuant, fl, sqrFiltLength, 
                          (*filters_per_fr_best), errorForce0CoeffTab, &error, lambda);
  
  if (forceCoeff0==1 && (*filters_per_fr_best)==1)
  {
    coeffBits = xcodeAuxInfo(-1, (*filters_per_fr_best), varIndTab, 0, createBistream=0,filtNo, m_tempALFp);
  }
  else
  {
    coeffBits = xcodeAuxInfo(filtNo, (*filters_per_fr_best), varIndTab, 0, createBistream=0,filtNo, m_tempALFp);
  }

  if (forceCoeff0==0)
  {
    coeffBits += xsendAllFiltersPPPred(filterCoeffSymQuant, fl, sqrFiltLength, 
      (*filters_per_fr_best), createBistream=0, m_tempALFp);
  }
  else
  {
    if ((*filters_per_fr_best)==1)
    {
      for(varInd=0; varInd<(*filters_per_fr_best); varInd++)
      {
        memset(filterCoeffSym[varInd],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
        memset(filterCoeffSymQuant[varInd],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
      }
    }
    else
    {
      coeffBits += xsendAllFiltersPPPredForce0(filterCoeffSymQuant, fl, sqrFiltLength, 
        (*filters_per_fr_best), codedVarBins, createBistream=0, m_tempALFp);

      for(varInd=0; varInd<(*filters_per_fr_best); varInd++)
      {
        if (codedVarBins[varInd]==0)
        {
          memset(filterCoeffSym[varInd],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
          memset(filterCoeffSymQuant[varInd],0,sizeof(int)*MAX_SQR_FILT_LENGTH);
        }
      }
    }
  }
}


Void TEncAdaptiveLoopFilter::xcalcPredFilterCoeff(Int filtNo)
{
  int *patternMap, varInd, i, k;
#if STAR_CROSS_SHAPES_LUMA
  patternMap = patternMapTabShapes[filtNo];
#if MQT_ALF_NPASS && !MQT_BA_RA
  if (m_iALFEncodePassReduction && (!m_iUsePreviousFilter || !m_iDesignCurrentFilter))
  {
    if((m_iCurrentPOC%m_iGOPSize) == 0)
    {
      m_iPreviousFilterShape[0]= m_iPreviousFilterShape[m_iGOPSize]; 
      m_iPreviousFilterShape[m_iGOPSize]= filtNo; 
    }
    else
    {
      m_iPreviousFilterShape[m_iCurrentPOC%m_iGOPSize] = filtNo;
    }
  }
#endif
#else
  patternMap=m_patternMapTab[filtNo];
#endif
  for(varInd=0; varInd<NO_VAR_BINS; ++varInd)
  {
    k=0;
    for(i = 0; i < MAX_SQR_FILT_LENGTH; i++)
    {
      if (patternMap[i]>0)
      {
        m_filterCoeffPrevSelected[varInd][i]=m_filterCoeffSym[m_varIndTab[varInd]][k];
        k++;
      }
      else
      {
        m_filterCoeffPrevSelected[varInd][i]=0;
      }
#if MQT_ALF_NPASS && !MQT_BA_RA
      if (m_iALFEncodePassReduction && (!m_iUsePreviousFilter || !m_iDesignCurrentFilter))
      {
        if((m_iCurrentPOC%m_iGOPSize) == 0)
        {
          m_aiFilterCoeffSaved[0][varInd][i] = m_aiFilterCoeffSaved[m_iGOPSize][varInd][i]; 
          m_aiFilterCoeffSaved[m_iGOPSize][varInd][i] = m_filterCoeffPrevSelected[varInd][i]; 
        }
        else
        {
          m_aiFilterCoeffSaved[m_iCurrentPOC%m_iGOPSize][varInd][i] = m_filterCoeffPrevSelected[varInd][i];
        }
      }
#endif

    }
  }
}

#if MQT_ALF_NPASS
/** code filter coefficients
 * \param filterCoeffSymQuant filter coefficients buffer
 * \param filtNo filter No.
 * \param varIndTab[] merge index information
 * \param filters_per_fr_best the number of filters used in this picture
 * \param frNo 
 * \param ALFp ALF parameters
 * \returns bitrate
 */

UInt TEncAdaptiveLoopFilter::xcodeFiltCoeff(Int **filterCoeffSymQuant, Int filtNo, Int varIndTab[], Int filters_per_fr_best, Int frNo, ALFParam* ALFp)
#else
Void TEncAdaptiveLoopFilter::xcodeFiltCoeff(Int **filterCoeffSymQuant, Int filtNo, Int varIndTab[], Int filters_per_fr_best, Int frNo, ALFParam* ALFp)
#endif
{
#if STAR_CROSS_SHAPES_LUMA
  Int varInd, forceCoeff0, codedVarBins[NO_VAR_BINS], coeffBits, createBistream;   
  Int sqrFiltLength = m_sqrFiltLengthTab[filtNo] ; 
  // in HM-3.x fl specifies the filter tap - in our proposal fl specifies the filter number (0: Cross5x5 - 1: Cross11x5)
  Int fl = filtNo;
#else
  int varInd, forceCoeff0, codedVarBins[NO_VAR_BINS], coeffBits, createBistream,   sqrFiltLength=m_sqrFiltLengthTab[filtNo], 
  fl=m_flTab[filtNo];
#endif 
  
  ALFp->filters_per_group_diff = filters_per_fr_best;
  ALFp->filters_per_group = filters_per_fr_best;
  
  for(varInd=0; varInd<filters_per_fr_best; varInd++)
  {
    codedVarBins[varInd] = 1;
  }
  memcpy (ALFp->codedVarBins, codedVarBins, sizeof(int)*NO_VAR_BINS);
  forceCoeff0=0;
  for(varInd=0; varInd<filters_per_fr_best; varInd++)
  {
    if (codedVarBins[varInd] == 0)
    {
      forceCoeff0=1;
      break;
    }
  }

  if (forceCoeff0==1 && filters_per_fr_best==1)
  {
    coeffBits = xcodeAuxInfo(-1, filters_per_fr_best, varIndTab, frNo, createBistream=1,filtNo, ALFp);
  }
  else
  {
    coeffBits = xcodeAuxInfo(filtNo, filters_per_fr_best, varIndTab, frNo, createBistream=1,filtNo, ALFp);
  }
  
  ALFp->forceCoeff0 = forceCoeff0;
  ALFp->predMethod = 0;
  ALFp->num_coeff = sqrFiltLength;
  ALFp->realfiltNo=filtNo;
  if (filters_per_fr_best <= 1)
  {
    ALFp->forceCoeff0 = 0;
    ALFp->predMethod = 0;
  }
  
  if (forceCoeff0==0) 
  {
    coeffBits += xsendAllFiltersPPPred(filterCoeffSymQuant, fl, sqrFiltLength, 
                                       filters_per_fr_best, createBistream=1, ALFp);
  }
  else if (filters_per_fr_best>1)
  {
    coeffBits += xsendAllFiltersPPPredForce0(filterCoeffSymQuant, fl, sqrFiltLength, 
                                             filters_per_fr_best, codedVarBins, createBistream=1, ALFp);
  }

#if MQT_ALF_NPASS
  return (UInt)coeffBits;
#endif
}


/** set ALF CU control flags
 * \param uiAlfCtrlDepth ALF CU control depth
 * \param pcPicOrg picture of original signal
 * \param pcPicDec picture before filtering
 * \param pcPicRest picture after filtering
 * \return ruiDist distortion after CU control
 * \return pAlfParam ALF parameters (CU control flags & number of CU control flags will be assigned)
 */
Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlags_qc(UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, ALFParam *pAlfParam)
{
  ruiDist = 0;
  pAlfParam->num_alf_cu_flag = 0;
  
  for( UInt uiCUAddr = 0; uiCUAddr < m_pcPic->getNumCUsInFrame() ; uiCUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( uiCUAddr );
    xSetCUAlfCtrlFlag_qc(pcCU, 0, 0, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist, pAlfParam);
  }

#if MTK_NONCROSS_INLOOP_FILTER
  if(m_uiNumSlicesInPic > 1)
  {
    getCtrlFlagsForSlices(true, (Int)uiAlfCtrlDepth);
    transferCtrlFlagsToAlfParam(pAlfParam->num_alf_cu_flag, pAlfParam->alf_cu_flag);
  }
#endif

}

Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlag_qc(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, ALFParam *pAlfParam)
{
  Bool bBoundary = false;
  UInt uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiRPelX   = uiLPelX + (g_uiMaxCUWidth>>uiDepth)  - 1;
  UInt uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  UInt uiBPelY   = uiTPelY + (g_uiMaxCUHeight>>uiDepth) - 1;
  
  if( ( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() ) || ( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() ) )
  {
    bBoundary = true;
  }
  
  if( ( ( uiDepth < pcCU->getDepth( uiAbsPartIdx ) ) && ( uiDepth < (g_uiMaxCUDepth-g_uiAddCUDepth) ) ) || bBoundary )
  {
    UInt uiQNumParts = ( m_pcPic->getNumPartInCU() >> (uiDepth<<1) )>>2;
    for ( UInt uiPartUnitIdx = 0; uiPartUnitIdx < 4; uiPartUnitIdx++, uiAbsPartIdx+=uiQNumParts )
    {
      uiLPelX   = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
      uiTPelY   = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
      
      if( ( uiLPelX < pcCU->getSlice()->getSPS()->getWidth() ) && ( uiTPelY < pcCU->getSlice()->getSPS()->getHeight() ) )
        xSetCUAlfCtrlFlag_qc(pcCU, uiAbsPartIdx, uiDepth+1, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist, pAlfParam);
    }
    return;
  }
  
  if( uiDepth > uiAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, uiAlfCtrlDepth))
  {
    return;
  }
  
  UInt uiCUAddr = pcCU->getAddr();
  UInt64 uiRecSSD = 0;
  UInt64 uiFiltSSD = 0;
  
  Int iWidth;
  Int iHeight;
  UInt uiSetDepth;
  
  if (uiDepth > uiAlfCtrlDepth && pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, uiAlfCtrlDepth))
  {
    iWidth = g_uiMaxCUWidth >> uiAlfCtrlDepth;
    iHeight = g_uiMaxCUHeight >> uiAlfCtrlDepth;
    
    uiRPelX   = uiLPelX + iWidth  - 1;
    uiBPelY   = uiTPelY + iHeight - 1;

    if( uiRPelX >= pcCU->getSlice()->getSPS()->getWidth() )
    {
      iWidth = pcCU->getSlice()->getSPS()->getWidth() - uiLPelX;
    }
    
    if( uiBPelY >= pcCU->getSlice()->getSPS()->getHeight() )
    {
      iHeight = pcCU->getSlice()->getSPS()->getHeight() - uiTPelY;
    }
    
    uiSetDepth = uiAlfCtrlDepth;
  }
  else
  {
    iWidth = pcCU->getWidth(uiAbsPartIdx);
    iHeight = pcCU->getHeight(uiAbsPartIdx);
    uiSetDepth = uiDepth;
  }
  
  Pel* pOrg = pcPicOrg->getLumaAddr(uiCUAddr, uiAbsPartIdx);
  Pel* pRec = pcPicDec->getLumaAddr(uiCUAddr, uiAbsPartIdx);
  Pel* pFilt = pcPicRest->getLumaAddr(uiCUAddr, uiAbsPartIdx);
  
  uiRecSSD  += xCalcSSD( pOrg, pRec,  iWidth, iHeight, pcPicOrg->getStride() );
  uiFiltSSD += xCalcSSD( pOrg, pFilt, iWidth, iHeight, pcPicOrg->getStride() );
  
  if (uiFiltSSD < uiRecSSD)
  {
    ruiDist += uiFiltSSD;
    pcCU->setAlfCtrlFlagSubParts(1, uiAbsPartIdx, uiSetDepth);
    pAlfParam->alf_cu_flag[pAlfParam->num_alf_cu_flag]=1;
    for (int i=uiTPelY ;i<=min(uiBPelY,(unsigned int)(pcPicOrg->getHeight()-1))  ;i++)
    {
      for (int j=uiLPelX ;j<=min(uiRPelX,(unsigned int)(pcPicOrg->getWidth()-1)) ;j++)
      { 
        m_maskImg[i][j]=1;
      }
    }
  }
  else
  {
    ruiDist += uiRecSSD;
    pcCU->setAlfCtrlFlagSubParts(0, uiAbsPartIdx, uiSetDepth);
    pAlfParam->alf_cu_flag[pAlfParam->num_alf_cu_flag]=0;
    for (int i=uiTPelY ;i<=min(uiBPelY,(unsigned int)(pcPicOrg->getHeight()-1))  ;i++)
    {
      for (int j=uiLPelX ;j<=min(uiRPelX,(unsigned int)(pcPicOrg->getWidth()-1)) ;j++)
      { 
        m_maskImg[i][j]=0;
      }
    }
  }
  pAlfParam->num_alf_cu_flag++;
}

Void TEncAdaptiveLoopFilter::xReDesignFilterCoeff_qc(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Bool bReadCorr)
{
#if STAR_CROSS_SHAPES_LUMA
  Int tap = m_pcTempAlfParam->realfiltNo;
#else  
  Int tap = m_pcTempAlfParam->tap;
#endif
  Int    LumaStride = pcPicOrg->getStride();
  imgpel* pOrg = (imgpel*)pcPicOrg->getLumaAddr();
  imgpel* pDec = (imgpel*)pcPicDec->getLumaAddr();
  imgpel* pRest = (imgpel*)pcPicRest->getLumaAddr();
  xFirstFilteringFrameLuma(pOrg, pDec, pRest, m_pcTempAlfParam, tap, LumaStride); 
  
#if MQT_ALF_NPASS && MQT_BA_RA
  if (m_iALFEncodePassReduction)
  {
    if(!m_iUsePreviousFilter)
    {
#if STAR_CROSS_SHAPES_LUMA
      saveFilterCoeffToBuffer(m_filterCoeffPrevSelected, m_pcTempAlfParam->realfiltNo);
#else
      saveFilterCoeffToBuffer(m_filterCoeffPrevSelected);
#endif
    }
  }
#endif



}
Void TEncAdaptiveLoopFilter::xCUAdaptiveControl_qc(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost)
{
#if MQT_ALF_NPASS
  imgpel** maskImgTemp;

  if(m_iALFEncodePassReduction == 2)
  {
    get_mem2Dpel(&maskImgTemp, m_im_height, m_im_width);
  }
#endif

  m_pcEntropyCoder->setAlfCtrl(true);
  
  UInt uiBestDepth = 0;
  
  ALFParam cFrmAlfParam;
  allocALFParam(&cFrmAlfParam);
  copyALFParam(&cFrmAlfParam, m_pcBestAlfParam);
  
  for (UInt uiDepth = 0; uiDepth < g_uiMaxCUDepth; uiDepth++)
  {
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
    pcPicRest->copyToPicLuma(m_pcPicYuvTmp);
    copyALFParam(m_pcTempAlfParam, &cFrmAlfParam);
    m_pcTempAlfParam->cu_control_flag = 1;
    
#if MQT_ALF_NPASS
    for (UInt uiRD = 0; uiRD <= m_iALFNumOfRedesign; uiRD++)
#else
    for (UInt uiRD = 0; uiRD <= ALF_NUM_OF_REDESIGN; uiRD++)
#endif
    {
      if (uiRD)
      {
        // re-design filter coefficients
        xReDesignFilterCoeff_qc(pcPicOrg, pcPicDec, m_pcPicYuvTmp, true); //use filtering of mine
      }
      
      UInt64 uiRate, uiDist;
      Double dCost;
     //m_pcPicYuvTmp: filtered signal, pcPicDec: orig reconst
      xSetCUAlfCtrlFlags_qc(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, m_pcTempAlfParam); //set up varImg here
      
      xCalcRDCost(m_pcTempAlfParam, uiRate, uiDist, dCost);
      
      if (dCost < rdMinCost)
      {
        uiBestDepth = uiDepth;
        rdMinCost = dCost;
        ruiMinDist = uiDist;
        ruiMinRate = uiRate;
        m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
        //save maskImg
        xCopyTmpAlfCtrlFlagsFrom();
#if MQT_ALF_NPASS
        if(m_iALFEncodePassReduction == 2)
        {
          ::memcpy(maskImgTemp[0], m_maskImg[0], sizeof(imgpel)*m_im_height* m_im_width);
        }
#endif
      }
    }
  }
  
  if (m_pcBestAlfParam->cu_control_flag)
  {
#if MQT_ALF_NPASS
    if(m_iALFEncodePassReduction == 2)
    {
      UInt uiDepth = uiBestDepth;
      ::memcpy(m_maskImg[0], maskImgTemp[0], sizeof(imgpel)*m_im_height* m_im_width);
      xCopyTmpAlfCtrlFlagsTo();
  
      copyALFParam(&cFrmAlfParam, m_pcBestAlfParam);

      m_pcEntropyCoder->setAlfCtrl(true);
      m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
      copyALFParam(m_pcTempAlfParam, &cFrmAlfParam);

      xReDesignFilterCoeff_qc(pcPicOrg, pcPicDec, m_pcPicYuvTmp, true); //use filtering of mine

      UInt64 uiRate, uiDist;
      Double dCost;

      xSetCUAlfCtrlFlags_qc(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, m_pcTempAlfParam); //set up varImg here

      xCalcRDCost(m_pcTempAlfParam, uiRate, uiDist, dCost);

      if (dCost < rdMinCost)
      {
        rdMinCost = dCost;
        ruiMinDist = uiDist;
        ruiMinRate = uiRate;
        m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
        xCopyTmpAlfCtrlFlagsFrom();
      }
    }
#endif

    m_pcEntropyCoder->setAlfCtrl(true);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiBestDepth);
    xCopyTmpAlfCtrlFlagsTo();

#if MTK_NONCROSS_INLOOP_FILTER
    if(m_uiNumSlicesInPic > 1)
    {
      getCtrlFlagsForSlices(true, (Int)uiBestDepth);
      transferCtrlFlagsToAlfParam(m_pcBestAlfParam->num_alf_cu_flag, m_pcBestAlfParam->alf_cu_flag);
    }
#endif

    m_pcPicYuvBest->copyToPicLuma(pcPicRest);//copy m_pcPicYuvBest to pcPicRest
    xCopyDecToRestCUs(pcPicDec, pcPicRest); //pcPicRest = pcPicDec
  }
  else
  {
    m_pcEntropyCoder->setAlfCtrl(false);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(0);
#if MTK_NONCROSS_INLOOP_FILTER
    if(m_uiNumSlicesInPic > 1)
    {
      getCtrlFlagsForSlices(false, 0);
    }
#endif

  }
  freeALFParam(&cFrmAlfParam);

#if MQT_ALF_NPASS
  if(m_iALFEncodePassReduction == 2)
  {
    free_mem2Dpel(maskImgTemp);
  }
#endif
}

#if !STAR_CROSS_SHAPES_LUMA
Void TEncAdaptiveLoopFilter::xFilterTapDecision_qc(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost)
{
#if MQT_ALF_NPASS
  if(m_iALFEncodePassReduction)
  {
    return;  // filter tap has been decided in xEncALFLuma_qc
  }
#endif

  // restriction for non-referenced B-slice
  if (m_eSliceType == B_SLICE && m_iPicNalReferenceIdc == 0)
  {
    return;
  }
  
  UInt64 uiRate, uiDist;
  Double dCost;
  
  if (m_pcBestAlfParam->cu_control_flag)
  {
    xCopyTmpAlfCtrlFlagsFrom();
  }
  
  Bool bChanged = false;
  for (Int iTap = ALF_MIN_NUM_TAP; iTap <= ALF_MAX_NUM_TAP; iTap += 2)
  {
    copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
    m_pcTempAlfParam->tap = iTap;
#if TI_ALF_MAX_VSIZE_7
    m_pcTempAlfParam->tapV = TComAdaptiveLoopFilter::ALFTapHToTapV(m_pcTempAlfParam->tap);
    m_pcTempAlfParam->num_coeff = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(m_pcTempAlfParam->tap); 
#else
    m_pcTempAlfParam->num_coeff = (Int)(iTap*iTap/4) + 2; 
#endif
    
    if (m_pcTempAlfParam->cu_control_flag)
    {
      xReDesignFilterCoeff_qc(pcPicOrg, pcPicDec, m_pcPicYuvTmp, false);
      xSetCUAlfCtrlFlags_qc(m_pcEntropyCoder->getMaxAlfCtrlDepth(), pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, m_pcTempAlfParam);
      xCalcRDCost(m_pcTempAlfParam, uiRate, uiDist, dCost);
    }

    else
    {
      Int    Height = pcPicOrg->getHeight();
      Int    Width = pcPicOrg->getWidth();
      for (Int i=0; i<Height; i++)
      {
        for (Int j=0; j<Width; j++)
        {
          m_maskImg[i][j] = 1;
        }
      }
      xReDesignFilterCoeff_qc(pcPicOrg, pcPicDec, m_pcPicYuvTmp, false);

      xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, m_pcTempAlfParam, uiRate, uiDist, dCost);
    }

    if (dCost < rdMinCost)
    {
      rdMinCost = dCost;
      ruiMinDist = uiDist;
      ruiMinRate = uiRate;
      m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
      copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
      bChanged = true;
      if (m_pcTempAlfParam->cu_control_flag)
      {
        xCopyTmpAlfCtrlFlagsFrom();
      }
    }
  }
  
  if (m_pcBestAlfParam->cu_control_flag)
  {
    xCopyTmpAlfCtrlFlagsTo();
#if MTK_NONCROSS_INLOOP_FILTER
    if(m_uiNumSlicesInPic > 1)
    {
      getCtrlFlagsForSlices(true, (Int)m_pcEntropyCoder->getMaxAlfCtrlDepth());
      transferCtrlFlagsToAlfParam(m_pcBestAlfParam->num_alf_cu_flag, m_pcBestAlfParam->alf_cu_flag);
    }
#endif

    if (bChanged)
    {
      m_pcPicYuvBest->copyToPicLuma(pcPicRest);
      xCopyDecToRestCUs(pcPicDec, pcPicRest);
    }
  }
  else if (m_pcBestAlfParam->tap > ALF_MIN_NUM_TAP)
  {
    m_pcPicYuvBest->copyToPicLuma(pcPicRest);
  }
  
  copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
}
#endif

#define ROUND(a)  (((a) < 0)? (int)((a) - 0.5) : (int)((a) + 0.5))
#define REG              0.0001
#define REG_SQR          0.0000001

//Find filter coeff related
Int TEncAdaptiveLoopFilter::gnsCholeskyDec(double **inpMatr, double outMatr[MAX_SQR_FILT_LENGTH][MAX_SQR_FILT_LENGTH], int noEq)
{ 
  int 
  i, j, k;     /* Looping Variables */
  double 
  scale;       /* scaling factor for each row */
  double 
  invDiag[MAX_SQR_FILT_LENGTH];  /* Vector of the inverse of diagonal entries of outMatr */
  
  
  /*
   *  Cholesky decomposition starts
   */
  
  for(i = 0; i < noEq; i++)
  {
    for(j = i; j < noEq; j++)
    {
      /* Compute the scaling factor */
      scale=inpMatr[i][j];
      if ( i > 0) for( k = i - 1 ; k >= 0 ; k--)
        scale -= outMatr[k][j] * outMatr[k][i];
      
      /* Compute i'th row of outMatr */
      if(i==j)
      {
        if(scale <= REG_SQR ) // if(scale <= 0 )  /* If inpMatr is singular */
        {
          return(0);
        }
        else              /* Normal operation */
          invDiag[i] =  1.0/(outMatr[i][i]=sqrt(scale));
      }
      else
      {
        outMatr[i][j] = scale*invDiag[i]; /* Upper triangular part          */
        outMatr[j][i] = 0.0;              /* Lower triangular part set to 0 */
      }                    
    }
  }
  return(1); /* Signal that Cholesky factorization is successfully performed */
}


Void TEncAdaptiveLoopFilter::gnsTransposeBacksubstitution(double U[MAX_SQR_FILT_LENGTH][MAX_SQR_FILT_LENGTH], double rhs[], double x[], int order)
{
  int 
  i,j;              /* Looping variables */
  double 
  sum;              /* Holds backsubstitution from already handled rows */
  
  /* Backsubstitution starts */
  x[0] = rhs[0]/U[0][0];               /* First row of U'                   */
  for (i = 1; i < order; i++)
  {         /* For the rows 1..order-1           */
    
    for (j = 0, sum = 0.0; j < i; j++) /* Backsubst already solved unknowns */
      sum += x[j]*U[j][i];
    
    x[i]=(rhs[i] - sum)/U[i][i];       /* i'th component of solution vect.  */
  }
}



Void  TEncAdaptiveLoopFilter::gnsBacksubstitution(double R[MAX_SQR_FILT_LENGTH][MAX_SQR_FILT_LENGTH], double z[MAX_SQR_FILT_LENGTH], int R_size, double A[MAX_SQR_FILT_LENGTH])
{
  int i, j;
  double sum;
  
  R_size--;
  
  A[R_size] = z[R_size] / R[R_size][R_size];
  
  for (i = R_size-1; i >= 0; i--)
  {
    for (j = i+1, sum = 0.0; j <= R_size; j++)
      sum += R[i][j] * A[j];
    
    A[i] = (z[i] - sum) / R[i][i];
  }
}


Int TEncAdaptiveLoopFilter::gnsSolveByChol(double **LHS, double *rhs, double *x, int noEq)
{
  assert(noEq > 0);

  double aux[MAX_SQR_FILT_LENGTH];     /* Auxiliary vector */
  double U[MAX_SQR_FILT_LENGTH][MAX_SQR_FILT_LENGTH];    /* Upper triangular Cholesky factor of LHS */
  int  i, singular;          /* Looping variable */
  
  /* The equation to be solved is LHSx = rhs */
  
  /* Compute upper triangular U such that U'*U = LHS */
  if(gnsCholeskyDec(LHS, U, noEq)) /* If Cholesky decomposition has been successful */
  {
    singular=1;
    /* Now, the equation is  U'*U*x = rhs, where U is upper triangular
     * Solve U'*aux = rhs for aux
     */
    gnsTransposeBacksubstitution(U, rhs, aux, noEq);         
    
    /* The equation is now U*x = aux, solve it for x (new motion coefficients) */
    gnsBacksubstitution(U, aux, noEq, x);   
    
  }
  else /* LHS was singular */ 
  {
    singular=0;
    
    /* Regularize LHS */
    for(i=0; i<noEq; i++)
    {
      LHS[i][i] += REG;
    }
    /* Compute upper triangular U such that U'*U = regularized LHS */
    singular = gnsCholeskyDec(LHS, U, noEq);
    if ( singular == 1 )
    {
      /* Solve  U'*aux = rhs for aux */  
      gnsTransposeBacksubstitution(U, rhs, aux, noEq);   
      
      /* Solve U*x = aux for x */
      gnsBacksubstitution(U, aux, noEq, x);      
    }
    else
    {
      x[ 0 ] = 1.0;
      for ( i = 1; i < noEq; i++ )
      {
        x[ i ] = 0.0;
      }
    }
  }  
  return(singular);
}


//////////////////////////////////////////////////////////////////////////////////////////


Void TEncAdaptiveLoopFilter::add_A(double **Amerged, double ***A, int start, int stop, int size)
{ 
  int
  i, j, ind;          /* Looping variable */
  
  for (i=0; i<size; i++)
  {
    for (j=0; j<size; j++)
    {
      Amerged[i][j]=0;
      for (ind=start; ind<=stop; ind++)
      {
        Amerged[i][j]+=A[ind][i][j];
      }
    }
  }
}

Void TEncAdaptiveLoopFilter::add_b(double *bmerged, double **b, int start, int stop, int size)
{ 
  int
  i, ind;          /* Looping variable */
  
  for (i=0; i<size; i++)
  {
    bmerged[i]=0;
    for (ind=start; ind<=stop; ind++)
    {
      bmerged[i]+=b[ind][i];
    }
  }
}

double TEncAdaptiveLoopFilter::calculateErrorCoeffProvided(double **A, double *b, double *c, int size)
{
  int i, j;
  double error, sum=0;
  
  error=0;
  for (i=0; i<size; i++)   //diagonal
  {
    sum=0;
    for (j=i+1; j<size; j++)
      sum+=(A[j][i]+A[i][j])*c[j];
    error+=(A[i][i]*c[i]+sum-2*b[i])*c[i];
  }
  
  return(error);
}

double TEncAdaptiveLoopFilter::calculateErrorAbs(double **A, double *b, double y, int size)
{
  int i;
  double error, sum;
  double c[MAX_SQR_FILT_LENGTH];
  
  gnsSolveByChol(A, b, c, size);
  
  sum=0;
  for (i=0; i<size; i++)
  {
    sum+=c[i]*b[i];
  }
  error=y-sum;
  
  return(error);
}

double TEncAdaptiveLoopFilter::mergeFiltersGreedy(double **yGlobalSeq, double ***EGlobalSeq, double *pixAccGlobalSeq, int intervalBest[NO_VAR_BINS][2], int sqrFiltLength, int noIntervals)
{
  int first, ind, ind1, ind2, i, j, bestToMerge ;
  double error, error1, error2, errorMin;
  static double pixAcc_temp, error_tab[NO_VAR_BINS],error_comb_tab[NO_VAR_BINS];
  static int indexList[NO_VAR_BINS], available[NO_VAR_BINS], noRemaining;
  if (noIntervals == NO_FILTERS)
  {
    noRemaining=NO_VAR_BINS;
    for (ind=0; ind<NO_VAR_BINS; ind++)
    {
      indexList[ind]=ind; 
      available[ind]=1;
      m_pixAcc_merged[ind]=pixAccGlobalSeq[ind];
      memcpy(m_y_merged[ind],yGlobalSeq[ind],sizeof(double)*sqrFiltLength);
      for (i=0; i<sqrFiltLength; i++)
      {
        memcpy(m_E_merged[ind][i],EGlobalSeq[ind][i],sizeof(double)*sqrFiltLength);
      }
    }
  }
  // Try merging different matrices
  if (noIntervals == NO_FILTERS)
  {
    for (ind=0; ind<NO_VAR_BINS; ind++)
    {
      error_tab[ind]=calculateErrorAbs(m_E_merged[ind], m_y_merged[ind], m_pixAcc_merged[ind], sqrFiltLength);
    }
    for (ind=0; ind<NO_VAR_BINS-1; ind++)
    {
      ind1=indexList[ind];
      ind2=indexList[ind+1];
      
      error1=error_tab[ind1];
      error2=error_tab[ind2];
      
      pixAcc_temp=m_pixAcc_merged[ind1]+m_pixAcc_merged[ind2];
      for (i=0; i<sqrFiltLength; i++)
      {
        m_y_temp[i]=m_y_merged[ind1][i]+m_y_merged[ind2][i];
        for (j=0; j<sqrFiltLength; j++)
        {
          m_E_temp[i][j]=m_E_merged[ind1][i][j]+m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1]=calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength)-error1-error2;
    }
  }
  while (noRemaining>noIntervals)
  {
    errorMin=0; first=1;
    bestToMerge = 0;
    for (ind=0; ind<noRemaining-1; ind++)
    {
      error = error_comb_tab[indexList[ind]];
      if ((error<errorMin || first==1))
      {
        errorMin=error;
        bestToMerge=ind;
        first=0;
      }
    }
    ind1=indexList[bestToMerge];
    ind2=indexList[bestToMerge+1];
    m_pixAcc_merged[ind1]+=m_pixAcc_merged[ind2];
    for (i=0; i<sqrFiltLength; i++)
    {
      m_y_merged[ind1][i]+=m_y_merged[ind2][i];
      for (j=0; j<sqrFiltLength; j++)
      {
        m_E_merged[ind1][i][j]+=m_E_merged[ind2][i][j];
      }
    }
    available[ind2]=0;
    
    //update error tables
    error_tab[ind1]=error_comb_tab[ind1]+error_tab[ind1]+error_tab[ind2];
    if (indexList[bestToMerge] > 0)
    {
      ind1=indexList[bestToMerge-1];
      ind2=indexList[bestToMerge];
      error1=error_tab[ind1];
      error2=error_tab[ind2];
      pixAcc_temp=m_pixAcc_merged[ind1]+m_pixAcc_merged[ind2];
      for (i=0; i<sqrFiltLength; i++)
      {
        m_y_temp[i]=m_y_merged[ind1][i]+m_y_merged[ind2][i];
        for (j=0; j<sqrFiltLength; j++)
        {
          m_E_temp[i][j]=m_E_merged[ind1][i][j]+m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1]=calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength)-error1-error2;
    }
    if (indexList[bestToMerge+1] < NO_VAR_BINS-1)
    {
      ind1=indexList[bestToMerge];
      ind2=indexList[bestToMerge+2];
      error1=error_tab[ind1];
      error2=error_tab[ind2];
      pixAcc_temp=m_pixAcc_merged[ind1]+m_pixAcc_merged[ind2];
      for (i=0; i<sqrFiltLength; i++)
      {
        m_y_temp[i]=m_y_merged[ind1][i]+m_y_merged[ind2][i];
        for (j=0; j<sqrFiltLength; j++)
        {
          m_E_temp[i][j]=m_E_merged[ind1][i][j]+m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1]=calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength)-error1-error2;
    }
    
    ind=0;
    for (i=0; i<NO_VAR_BINS; i++)
    {
      if (available[i]==1)
      {
        indexList[ind]=i;
        ind++;
      }
    }
    noRemaining--;
  }
  
  
  errorMin=0;
  for (ind=0; ind<noIntervals; ind++)
  {
    errorMin+=error_tab[indexList[ind]];
  }
  
  for (ind=0; ind<noIntervals-1; ind++)
  {
    intervalBest[ind][0]=indexList[ind]; intervalBest[ind][1]=indexList[ind+1]-1;
  }
  
  intervalBest[noIntervals-1][0]=indexList[noIntervals-1]; 
  intervalBest[noIntervals-1][1]=NO_VAR_BINS-1;
  
  return(errorMin);
}



double TEncAdaptiveLoopFilter::findFilterGroupingError(double ***EGlobalSeq, double **yGlobalSeq, double *pixAccGlobalSeq, int intervalBest[NO_VAR_BINS][2], int sqrFiltLength, int filters_per_fr)
{
  double error;
  
  // find best filters for each frame group
  error = 0;
  error += mergeFiltersGreedy(yGlobalSeq, EGlobalSeq, pixAccGlobalSeq, intervalBest, sqrFiltLength, filters_per_fr);
  
  return(error);
}


Void TEncAdaptiveLoopFilter::roundFiltCoeff(int *FilterCoeffQuan, double *FilterCoeff, int sqrFiltLength, int factor)
{
  int i;
  double diff; 
  int diffInt, sign; 
  
  for(i = 0; i < sqrFiltLength; i++)
  {
    sign               = (FilterCoeff[i]>0) ?  1: -1; 
    diff               = FilterCoeff[i]*sign; 
    diffInt            = (int)(diff*(double)factor+0.5); 
    FilterCoeffQuan[i] = diffInt*sign;
  }
}

Double TEncAdaptiveLoopFilter::QuantizeIntegerFilterPP(double *filterCoeff, int *filterCoeffQuant, double **E, double *y, int sqrFiltLength, int *weights, int bit_depth)
{
  double error;
  
  int factor = (1<<(bit_depth-1)), i; 
  int quantCoeffSum, minInd, targetCoeffSumInt, k, diff;
  double targetCoeffSum, errMin;
  
  gnsSolveByChol(E, y, filterCoeff, sqrFiltLength);
  targetCoeffSum=0;
  for (i=0; i<sqrFiltLength; i++)
  {
    targetCoeffSum+=(weights[i]*filterCoeff[i]*factor);
  }
  targetCoeffSumInt=ROUND(targetCoeffSum);
  roundFiltCoeff(filterCoeffQuant, filterCoeff, sqrFiltLength, factor);
  quantCoeffSum=0;
  for (i=0; i<sqrFiltLength; i++)
  {
    quantCoeffSum+=weights[i]*filterCoeffQuant[i];
  }
  
  int count=0;
  while(quantCoeffSum!=targetCoeffSumInt && count < 10)
  {
    if (quantCoeffSum>targetCoeffSumInt)
    {
      diff=quantCoeffSum-targetCoeffSumInt;
      errMin=0; minInd=-1;
      for (k=0; k<sqrFiltLength; k++)
      {
        if (weights[k]<=diff)
        {
          for (i=0; i<sqrFiltLength; i++)
          {
            m_filterCoeffQuantMod[i]=filterCoeffQuant[i];
          }
          m_filterCoeffQuantMod[k]--;
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeff[i]=(double)m_filterCoeffQuantMod[i]/(double)factor;
          }
          error=calculateErrorCoeffProvided(E, y, filterCoeff, sqrFiltLength);
          if (error<errMin || minInd==-1)
          {
            errMin=error;
            minInd=k;
          }
        } // if (weights(k)<=diff)
      } // for (k=0; k<sqrFiltLength; k++)
      filterCoeffQuant[minInd]--;
    }
    else
    {
      diff=targetCoeffSumInt-quantCoeffSum;
      errMin=0; minInd=-1;
      for (k=0; k<sqrFiltLength; k++)
      {
        if (weights[k]<=diff)
        {
          for (i=0; i<sqrFiltLength; i++)
          {
            m_filterCoeffQuantMod[i]=filterCoeffQuant[i];
          }
          m_filterCoeffQuantMod[k]++;
          for (i=0; i<sqrFiltLength; i++)
          {
            filterCoeff[i]=(double)m_filterCoeffQuantMod[i]/(double)factor;
          }
          error=calculateErrorCoeffProvided(E, y, filterCoeff, sqrFiltLength);
          if (error<errMin || minInd==-1)
          {
            errMin=error;
            minInd=k;
          }
        } // if (weights(k)<=diff)
      } // for (k=0; k<sqrFiltLength; k++)
      filterCoeffQuant[minInd]++;
    }
    
    quantCoeffSum=0;
    for (i=0; i<sqrFiltLength; i++)
    {
      quantCoeffSum+=weights[i]*filterCoeffQuant[i];
    }
  }
  if( count == 10 )
  {
    for (i=0; i<sqrFiltLength; i++)
    {
      filterCoeffQuant[i] = 0;
    }
  }
  
  for (i=0; i<sqrFiltLength; i++)
  {
    filterCoeff[i]=(double)filterCoeffQuant[i]/(double)factor;
  }
  
  error=calculateErrorCoeffProvided(E, y, filterCoeff, sqrFiltLength);
  return(error);
}

Double TEncAdaptiveLoopFilter::findFilterCoeff(double ***EGlobalSeq, double **yGlobalSeq, double *pixAccGlobalSeq, int **filterCoeffSeq, int **filterCoeffQuantSeq, int intervalBest[NO_VAR_BINS][2], int varIndTab[NO_VAR_BINS], int sqrFiltLength, int filters_per_fr, int *weights, int bit_depth, double errorTabForce0Coeff[NO_VAR_BINS][2])
{
  static double pixAcc_temp;
  double error;
  int k, filtNo;
  
  error = 0;
  for(filtNo = 0; filtNo < filters_per_fr; filtNo++)
  {
    add_A(m_E_temp, EGlobalSeq, intervalBest[filtNo][0], intervalBest[filtNo][1], sqrFiltLength);
    add_b(m_y_temp, yGlobalSeq, intervalBest[filtNo][0], intervalBest[filtNo][1], sqrFiltLength);
    
    pixAcc_temp = 0;    
    for(k = intervalBest[filtNo][0]; k <= intervalBest[filtNo][1]; k++)
      pixAcc_temp += pixAccGlobalSeq[k];
    
    // Find coeffcients
    errorTabForce0Coeff[filtNo][1] = pixAcc_temp + QuantizeIntegerFilterPP(m_filterCoeff, m_filterCoeffQuant, m_E_temp, m_y_temp, sqrFiltLength, weights, bit_depth);
    errorTabForce0Coeff[filtNo][0] = pixAcc_temp;
    error += errorTabForce0Coeff[filtNo][1];
    
    for(k = 0; k < sqrFiltLength; k++)
    {
      filterCoeffSeq[filtNo][k] = m_filterCoeffQuant[k];
      filterCoeffQuantSeq[filtNo][k] = m_filterCoeffQuant[k];
    }
  }
  
  for(filtNo = 0; filtNo < filters_per_fr; filtNo++)
  {
    for(k = intervalBest[filtNo][0]; k <= intervalBest[filtNo][1]; k++)
      varIndTab[k] = filtNo;
  }
  
  return(error);
}

#if MQT_ALF_NPASS

#if MQT_BA_RA

/** Save redesigned filter set to buffer
 * \param filterCoeffPrevSelected filter set buffer
 */
#if STAR_CROSS_SHAPES_LUMA
Void TEncAdaptiveLoopFilter::saveFilterCoeffToBuffer(Int **filterCoeffPrevSelected,Int filtNo)
#else
Void TEncAdaptiveLoopFilter::saveFilterCoeffToBuffer(Int **filterCoeffPrevSelected)
#endif
{
  Int iBufferIndex = m_iCurrentPOC%m_iGOPSize;

  static Bool bFirst = true;
  static Bool* pbFirstAccess;
  if(bFirst)
  {
    pbFirstAccess = new Bool[NUM_ALF_CLASS_METHOD];
    for(Int i=0; i< NUM_ALF_CLASS_METHOD; i++)
    {
      pbFirstAccess[i]= true;
    }
    bFirst = false;
  }

  if(iBufferIndex == 0)
  {
    if(pbFirstAccess[m_uiVarGenMethod])
    {
      for(Int varInd=0; varInd<NO_VAR_BINS; ++varInd)
      {
        ::memcpy(m_aiFilterCoeffSaved[m_iGOPSize][varInd],filterCoeffPrevSelected[varInd], sizeof(Int)*MAX_SQR_FILT_LENGTH );
      }
#if STAR_CROSS_SHAPES_LUMA
      m_iPreviousFilterShape[m_iGOPSize]= filtNo; 
#endif
      pbFirstAccess[m_uiVarGenMethod] = false;
    }

    for(Int varInd=0; varInd<NO_VAR_BINS; ++varInd)
    {
      ::memcpy(m_aiFilterCoeffSaved[0][varInd],m_aiFilterCoeffSaved[m_iGOPSize][varInd], sizeof(Int)*MAX_SQR_FILT_LENGTH );
      ::memcpy(m_aiFilterCoeffSaved[m_iGOPSize][varInd],filterCoeffPrevSelected[varInd], sizeof(Int)*MAX_SQR_FILT_LENGTH );

    }
#if STAR_CROSS_SHAPES_LUMA
    m_iPreviousFilterShape[0]= m_iPreviousFilterShape[m_iGOPSize]; 
    m_iPreviousFilterShape[m_iGOPSize]= filtNo; 
#endif
  }
  else
  {
    for(Int varInd=0; varInd<NO_VAR_BINS; ++varInd)
    {
      ::memcpy(m_aiFilterCoeffSaved[iBufferIndex][varInd],filterCoeffPrevSelected[varInd], sizeof(Int)*MAX_SQR_FILT_LENGTH );
    }
#if STAR_CROSS_SHAPES_LUMA
    m_iPreviousFilterShape[iBufferIndex]= filtNo; 
#endif

  }
}

/** set initial m_maskImg with previous (time-delayed) filters
 * \param pcPicOrg original picture
 * \param pcPicDec reconstructed picture after deblocking
 */
Void TEncAdaptiveLoopFilter::setMaskWithTimeDelayedResults(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec)
{

  static imgpel** bestImgMask;
  static Bool bFirst = true;
  if(bFirst)
  {
    get_mem2Dpel(&bestImgMask, m_im_height, m_im_width);
    bFirst = false;
  }

  imgpel* pDec       = (imgpel*)pcPicDec->getLumaAddr();
  imgpel* pOrg       = (imgpel*)pcPicOrg->getLumaAddr();
  imgpel* pRest      = (imgpel*)m_pcPicYuvTmp->getLumaAddr();
  Int     LumaStride = pcPicOrg->getStride();



  UInt64    uiRate, uiDist;
  Double    dCost, dMinCost = MAX_DOUBLE;
  ALFParam  cAlfParam;
  allocALFParam(&cAlfParam);
  cAlfParam.alf_flag        = 0;
  cAlfParam.cu_control_flag = 0;
  cAlfParam.chroma_idc      = 0;

  //filter frame with the previous time-delayed filters
#if STAR_CROSS_SHAPES_LUMA
  Int filtNo;
  m_pcTempAlfParam->alf_flag = 1;
#if MQT_BA_RA
  m_pcTempAlfParam->alf_pcr_region_flag = m_uiVarGenMethod;
#endif
  m_pcTempAlfParam->cu_control_flag = 0;
#else
  Int iTap = 9;
  Int filtNo = 0;
  m_pcTempAlfParam->alf_flag = 1;
#if MQT_BA_RA
  m_pcTempAlfParam->alf_pcr_region_flag = m_uiVarGenMethod;
#endif
  m_pcTempAlfParam->cu_control_flag = 0;
  m_pcTempAlfParam->tap = iTap;
#if TI_ALF_MAX_VSIZE_7
  m_pcTempAlfParam->tapV      = TComAdaptiveLoopFilter::ALFTapHToTapV(iTap);
  m_pcTempAlfParam->num_coeff = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(iTap);
#else
  m_pcTempAlfParam->num_coeff = iTap*iTap/4 + 2;
#endif
#endif

  for (Int index=0; index<2; index++)
  {
    setFilterIdx(index);

#if STAR_CROSS_SHAPES_LUMA
    filtNo = m_pcTempAlfParam->realfiltNo = m_iPreviousFilterShape[m_iFilterIdx];
    if (filtNo != 0 && filtNo != 1) 
    {
      continue;
    }
    m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filtNo] ; 
#endif

    xcalcPredFilterCoeffPrev(filtNo);
#if MTK_NONCROSS_INLOOP_FILTER
    if(!m_bUseNonCrossALF)
      xfilterFrame_en(0, 0, m_im_height, m_im_width, pDec, pRest, filtNo, LumaStride);
    else
      xfilterSlices_en(pDec, pRest, filtNo, LumaStride);
#else
    xfilterFrame_en(pDec, pRest, filtNo, LumaStride);
#endif
    xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, NULL, uiRate, uiDist, dCost);
    if (dCost < dMinCost)
    {
      dMinCost  = dCost;
      m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
      copyALFParam(&cAlfParam, m_pcTempAlfParam);
    }
  }

  //decided the best CU control depth
  m_pcPicYuvBest->copyToPicLuma(m_pcPicYuvTmp);
  m_pcEntropyCoder->setAlfCtrl(true);

  Int maxDepth = g_uiMaxCUDepth;
  if (pcPicOrg->getWidth() < 1000) maxDepth = 2;
  for (UInt uiDepth = 0; uiDepth < maxDepth; uiDepth++)
  {
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
    copyALFParam(m_pcTempAlfParam, &cAlfParam);
    m_pcTempAlfParam->cu_control_flag = 1;

    xSetCUAlfCtrlFlags_qc(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, m_pcTempAlfParam); //set up varImg here
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    xEncodeCUAlfCtrlFlags();
    uiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
    dCost  = (Double)(uiRate) * m_dLambdaLuma + (Double)(uiDist);

    if (dCost < dMinCost)
    {
      dMinCost    = dCost;
      copyALFParam(&cAlfParam, m_pcTempAlfParam);
      ::memcpy(bestImgMask[0], m_maskImg[0], sizeof(imgpel)*m_im_height* m_im_width);
    }
  }
  //set initial maskImg for first filtering & calculate the SSD of non-filtered region for estimating filtering distortion
  if (cAlfParam.cu_control_flag)
  {
    ::memcpy(m_maskImg[0], bestImgMask[0], sizeof(imgpel)*m_im_height* m_im_width);

  }
  else
  {
    for(Int y=0; y< m_im_height; y++)
    {
      for(Int x=0; x< m_im_width; x++)
      {
        m_maskImg[y][x] = 1;
      }
    }
  }
  m_pcEntropyCoder->setAlfCtrl(false);
  m_pcEntropyCoder->setMaxAlfCtrlDepth(0);

  // generate filters for future reference
  m_iDesignCurrentFilter = 0; 

  int  filters_per_fr;
  int  lambda_val = (Int)m_dLambdaLuma;

  lambda_val = lambda_val * (1<<(2*g_uiBitIncrement));

#if STAR_CROSS_SHAPES_LUMA
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
  {
    xstoreInBlockMatrix(0, 0, m_im_height, m_im_width, true, true, pOrg, pDec, cAlfParam.realfiltNo, LumaStride);
  }
  else
  { 
    xstoreInBlockMatrixforSlices(pOrg, pDec, cAlfParam.realfiltNo, LumaStride);
  }
#else
  xstoreInBlockMatrix(pOrg, pDec, cAlfParam.realfiltNo, LumaStride);
#endif
#else
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
    xstoreInBlockMatrix(0, 0, m_im_height, m_im_width, true, true, pOrg, pDec, cAlfParam.tap, LumaStride);
  else
    xstoreInBlockMatrixforSlices(pOrg, pDec, cAlfParam.tap, LumaStride);
#else
  xstoreInBlockMatrix(pOrg, pDec, cAlfParam.tap, LumaStride);
#endif

  assert(filtNo == 0);
#endif
  xfindBestFilterVarPred(m_yGlobalSym[filtNo], m_EGlobalSym[filtNo], m_pixAcc, 
    m_filterCoeffSym, m_filterCoeffSymQuant, 
    filtNo, &filters_per_fr, 
    m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambda_val);

  xcalcPredFilterCoeff(filtNo);

#if STAR_CROSS_SHAPES_LUMA
  saveFilterCoeffToBuffer(m_filterCoeffPrevSelected,filtNo);
#else
  saveFilterCoeffToBuffer(m_filterCoeffPrevSelected);
#endif
  m_iDesignCurrentFilter = 1;

  freeALFParam(&cAlfParam);

}




#endif


/** set ALF encoding parameters
 * \param pcPic picture pointer
 */
Void TEncAdaptiveLoopFilter::setALFEncodingParam(TComPic *pcPic)
{
  if(m_iALFEncodePassReduction)
  {
    m_iALFNumOfRedesign = 0;
    m_iCurrentPOC = m_pcPic->getPOC();

    if((m_eSliceType == I_SLICE) || (m_iGOPSize==8 && (m_iCurrentPOC % 4 == 0)))
    {
      m_iUsePreviousFilter = 0;
    }
    else
    {
      m_iUsePreviousFilter = 1;
    }
  }
  else
  {
    m_iALFNumOfRedesign = ALF_NUM_OF_REDESIGN;
  }
  m_iDesignCurrentFilter = 1;

}

/** Calculate/Restore filter coefficients from previous filters
 * \param filtNo
 */
Void TEncAdaptiveLoopFilter::xcalcPredFilterCoeffPrev(Int filtNo)
{
  int varInd, i;

  for(varInd=0; varInd<NO_VAR_BINS; ++varInd)
  {
    for(i = 0; i < MAX_SQR_FILT_LENGTH; i++)
    {
      m_filterCoeffPrevSelected[varInd][i]=m_aiFilterCoeffSaved[m_iFilterIdx][varInd][i];
    }
  }
}

/** set filter buffer index
 * \param index the processing order of time-delayed filtering
 */
Void TEncAdaptiveLoopFilter::setFilterIdx(Int index)
{
  if (m_iGOPSize == 8)
  {
    if ((m_iCurrentPOC % m_iGOPSize) == 0)
    {
      Int FiltTable[2] = {0, m_iGOPSize};
      m_iFilterIdx = FiltTable[index]; 
    }
    if ((m_iCurrentPOC % m_iGOPSize) == 4)
    {
      Int FiltTable[2] = {0, m_iGOPSize};
      m_iFilterIdx = FiltTable[index]; 
    }
    if ((m_iCurrentPOC % m_iGOPSize) == 2)
    {
      Int FiltTable[2] = {0, 4};
      m_iFilterIdx = FiltTable[index]; 
    }
    if ((m_iCurrentPOC % m_iGOPSize) == 6)
    {
      Int FiltTable[2] = {4, m_iGOPSize};
      m_iFilterIdx = FiltTable[index]; 
    }
    if ((m_iCurrentPOC % m_iGOPSize) == 1)
    {
      Int FiltTable[2] = {0, 2};
      m_iFilterIdx = FiltTable[index]; 
    }
    if ((m_iCurrentPOC % m_iGOPSize) == 3)
    {
      Int FiltTable[2] = {2, 4};
      m_iFilterIdx = FiltTable[index]; 
    }
    if ((m_iCurrentPOC % m_iGOPSize) == 5)
    {
      Int FiltTable[2] = {4, 6};
      m_iFilterIdx = FiltTable[index]; 
    }
    if ((m_iCurrentPOC % m_iGOPSize) == 7)
    {
      Int FiltTable[2] = {6, m_iGOPSize};
      m_iFilterIdx = FiltTable[index]; 
    }
  }
  else
  {
    Int FiltTable[2] = {0, m_iGOPSize};
    m_iFilterIdx = FiltTable[index]; 
  }
}

/** set initial m_maskImg
 * \param pcPicOrg original picture pointer
 * \param pcPicDec reconstructed picture pointer
 */
Void TEncAdaptiveLoopFilter::setInitialMask(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec)
{
  Int Height = pcPicOrg->getHeight();
  Int Width = pcPicOrg->getWidth();
  Int LumaStride = pcPicOrg->getStride();
  imgpel* pDec = (imgpel*)pcPicDec->getLumaAddr();

#if STAR_CROSS_SHAPES_LUMA
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
  {
    calcVar(0, 0, m_varImg, pDec, FILTER_LENGTH/2, VAR_SIZE, Height, Width, LumaStride);
  }
  else
  {
    calcVarforSlices(m_varImg, pDec, FILTER_LENGTH/2, VAR_SIZE, LumaStride);
  }
#else
  calcVar(m_varImg, pDec, FILTER_LENGTH/2, VAR_SIZE, Height, Width, LumaStride);
#endif
#else
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
    calcVar(0, 0, m_varImg, pDec, 9/2, VAR_SIZE, Height, Width, LumaStride);
  else
    calcVarforSlices(m_varImg, pDec, 9/2, VAR_SIZE, LumaStride);
#else
  calcVar(m_varImg, pDec, 9/2, VAR_SIZE, Height, Width, LumaStride);
#endif
#endif

  if(!m_iALFEncodePassReduction || !m_iUsePreviousFilter)
  {
    for(Int y=0; y<Height; y++)
    {
      for(Int x=0; x<Width; x++)
      {
        m_maskImg[y][x] = 1;
      }
    }
  }
  else
  {
#if MQT_BA_RA
    setMaskWithTimeDelayedResults(pcPicOrg, pcPicDec);
#else
    Int uiBestDepth=0;
    UInt64 uiRate, uiDist;
    Double dCost, dMinCost = MAX_DOUBLE;
    //imgpel* pOrg = (imgpel*)pcPicOrg->getLumaAddr();
    imgpel* pRest = (imgpel*)m_pcPicYuvTmp->getLumaAddr();

#if STAR_CROSS_SHAPES_LUMA
    m_pcTempAlfParam->cu_control_flag = 0;
#else
    Int iTap = 9;
    Int filtNo = 0;
    m_pcTempAlfParam->cu_control_flag = 0;
    m_pcTempAlfParam->tap = iTap;
#if TI_ALF_MAX_VSIZE_7
    m_pcTempAlfParam->tapV      = TComAdaptiveLoopFilter::ALFTapHToTapV(iTap);
    m_pcTempAlfParam->num_coeff = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(iTap);
#else
    m_pcTempAlfParam->num_coeff = iTap*iTap/4 + 2;
#endif
#endif

    for (Int index=0; index<2; index++)
    {
      setFilterIdx(index);
#if STAR_CROSS_SHAPES_LUMA
      Int filtNo = m_pcTempAlfParam->realfiltNo = m_iPreviousFilterShape[m_iFilterIdx];
      if (filtNo != 0 && filtNo != 1) 
      {
        continue; 
      }
      m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filtNo] ; 
#endif
      xcalcPredFilterCoeffPrev(filtNo);
#if MTK_NONCROSS_INLOOP_FILTER
      if(!m_bUseNonCrossALF)
        xfilterFrame_en(0, 0, Height, Width, pDec, pRest, filtNo, LumaStride);
      else
        xfilterSlices_en(pDec, pRest, filtNo, LumaStride);
#else
      xfilterFrame_en(pDec, pRest, filtNo, LumaStride);
#endif
      xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, NULL, uiRate, uiDist, dCost);
      if (dCost < dMinCost)
      {
        dMinCost  = dCost;
        m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
      }
    }
    m_pcPicYuvBest->copyToPicLuma(m_pcPicYuvTmp);

    m_pcEntropyCoder->setAlfCtrl(true);
    Int maxDepth = g_uiMaxCUDepth;
    if (pcPicOrg->getWidth() < 1000) maxDepth = 2;
    for (UInt uiDepth = 0; uiDepth < maxDepth; uiDepth++)
    {
      m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
      copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
      m_pcTempAlfParam->cu_control_flag = 1;

      xSetCUAlfCtrlFlags_qc(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, m_pcTempAlfParam); //set up varImg here
      m_pcEntropyCoder->resetEntropy();
      m_pcEntropyCoder->resetBits();
      xEncodeCUAlfCtrlFlags();
      uiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
      dCost  = (Double)(uiRate) * m_dLambdaLuma + (Double)(uiDist);

      if (dCost < dMinCost)
      {
        uiBestDepth = uiDepth;
        dMinCost    = dCost;
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
        //save maskImg
        xCopyTmpAlfCtrlFlagsFrom();
      }
    }

    copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
    m_iDesignCurrentFilter = 0; // design filter for subsequent slices
    xReDesignFilterCoeff_qc(pcPicOrg, pcPicDec, m_pcPicYuvTmp, true);
    m_iDesignCurrentFilter = 1;

    if (m_pcBestAlfParam->cu_control_flag)
    {
      m_pcEntropyCoder->setAlfCtrl(true);
      m_pcEntropyCoder->setMaxAlfCtrlDepth(uiBestDepth);
      xCopyTmpAlfCtrlFlagsTo();
    }
    else
    {
      m_pcEntropyCoder->setAlfCtrl(false);
      m_pcEntropyCoder->setMaxAlfCtrlDepth(0);
      for(Int y=0; y<Height; y++)
      {
        for(Int x=0; x<Width; x++)
        {
          m_maskImg[y][x] = 1;
        }
      }
    }
#endif
  }
}



#if MQT_BA_RA

/** Estimate RD cost of all filter size & store the best one
 * \param ImgOrg original picture
 * \param ImgDec reconstructed picture after deblocking
 * \param Sride  line buffer size of picture buffer
 * \param pcAlfSaved the best Alf parameters 
 * \returns ruiDist             estimated distortion
 * \returns ruiRate             required bits
 * \returns rdCost              estimated R-D cost
 */
Void  TEncAdaptiveLoopFilter::xFirstEstimateFilteringFrameLumaAllTap(imgpel* ImgOrg, imgpel* ImgDec, Int Stride, 
                                                                     ALFParam* pcAlfSaved,
                                                                     UInt64& ruiRate,
                                                                     UInt64& ruiDist,
                                                                     Double& rdCost
                                                                     )
{

  static Bool bFirst = true;
  static Int  aiVarIndTabBest[NO_VAR_BINS];
  static Double **ySym, ***ESym;
  static Int**  ppiBestCoeffSet;

  if(bFirst)
  {
    initMatrix_int(&ppiBestCoeffSet, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);
    bFirst = false;
  }
  Int    lambda_val = ((Int) m_dLambdaLuma) * (1<<(2*g_uiBitIncrement));
  Int    filtNo, ibestfiltNo=0, filters_per_fr, ibestfilters_per_fr=0;
  Int64  iEstimatedDist;
  UInt64 uiRate;
  Double dEstimatedCost, dEstimatedMinCost = MAX_DOUBLE;;
#if !STAR_CROSS_SHAPES_LUMA
  Bool   bMatrixBaseReady  = false;
#endif
  
  UInt   uiBitShift = (g_uiBitIncrement<<1);

#if !STAR_CROSS_SHAPES_LUMA
  m_iMatrixBaseFiltNo = 0;
#endif
  m_pcTempAlfParam->alf_flag = 1;
  m_pcTempAlfParam->cu_control_flag = 0;
  m_pcTempAlfParam->chroma_idc = 0;
  m_pcTempAlfParam->alf_pcr_region_flag = m_uiVarGenMethod;

#if STAR_CROSS_SHAPES_LUMA
  //prepare the 11x5 MatrixBase - correlation -  
  ESym     = m_EGlobalSym     [2];//Square11x5
  ySym     = m_yGlobalSym     [2];//Square11x5
  m_pcTempAlfParam->num_coeff = MAX_SQR_FILT_LENGTH;
#if MTK_NONCROSS_INLOOP_FILTER
  {
    if(!m_bUseNonCrossALF)
    {
      xstoreInBlockMatrix(0, 0, m_im_height, m_im_width, true, true, ImgOrg, ImgDec, 2, Stride);
    }
    else
    {
      xstoreInBlockMatrixforSlices(ImgOrg, ImgDec, 2, Stride);
    }
  }
#else
  xstoreInBlockMatrix(ImgOrg, ImgDec, 2, Stride);
#endif
  for (int filter_shape = 0; filter_shape < 2 ;filter_shape ++)
  {
    m_pcTempAlfParam->realfiltNo = filtNo = filter_shape;
    m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filtNo] ;  

    ESym     = m_EGlobalSym     [filtNo];
    ySym     = m_yGlobalSym     [filtNo];

    xretriveBlockMatrix(m_pcTempAlfParam->num_coeff, m_iFilterTabIn11x5Sym[filtNo], 
                        m_EGlobalSym[2], ESym, 
                        m_yGlobalSym[2], ySym);
#else
  for(Int iTap = ALF_MAX_NUM_TAP; iTap>=ALF_MIN_NUM_TAP; iTap -= 2)
  {
    m_pcTempAlfParam->tap = iTap;
#if TI_ALF_MAX_VSIZE_7
    m_pcTempAlfParam->tapV      = TComAdaptiveLoopFilter::ALFTapHToTapV(iTap);
    m_pcTempAlfParam->num_coeff = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(iTap);
#else
    m_pcTempAlfParam->num_coeff = iTap*iTap/4 + 2;
#endif

    if (iTap==9)
    {
      filtNo = 0;
    }
    else if (iTap==7)
    {
      filtNo = 1;
    }
    else
    {
      filtNo = 2;
    }

    ESym     = m_EGlobalSym     [filtNo];
    ySym     = m_yGlobalSym     [filtNo];

    if( bMatrixBaseReady )
    {
      xretriveBlockMatrix(m_pcTempAlfParam->num_coeff, m_iTapPosTabIn9x9Sym[filtNo], 
        m_EGlobalSym[m_iMatrixBaseFiltNo], ESym, 
        m_yGlobalSym[m_iMatrixBaseFiltNo], ySym);

    }
    else
#if MTK_NONCROSS_INLOOP_FILTER
    {
      if(!m_bUseNonCrossALF)
        xstoreInBlockMatrix(0, 0, m_im_height, m_im_width, true, true, ImgOrg, ImgDec, iTap, Stride);
      else
        xstoreInBlockMatrixforSlices(ImgOrg, ImgDec, iTap, Stride);
    }
#else
      xstoreInBlockMatrix(ImgOrg, ImgDec, iTap, Stride);
#endif

    if(filtNo == m_iMatrixBaseFiltNo)
    {
      bMatrixBaseReady = true;
    }
#endif

    xfindBestFilterVarPred(ySym, ESym, m_pixAcc, m_filterCoeffSym, m_filterCoeffSymQuant, filtNo, &filters_per_fr, 
      m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambda_val);

    uiRate         = xcodeFiltCoeff(m_filterCoeffSymQuant, filtNo, m_varIndTab, filters_per_fr,0, m_pcTempAlfParam);
    iEstimatedDist = xEstimateFiltDist(filters_per_fr, m_varIndTab, ESym, ySym, m_filterCoeffSym, m_pcTempAlfParam->num_coeff);
    dEstimatedCost = (Double)(uiRate) * m_dLambdaLuma + (Double)(iEstimatedDist);

    if(dEstimatedCost < dEstimatedMinCost)
    {
      dEstimatedMinCost   = dEstimatedCost;
      ibestfiltNo         = filtNo;
      ibestfilters_per_fr = filters_per_fr;

      copyALFParam(pcAlfSaved, m_pcTempAlfParam); 

      for(Int i=0; i< ibestfilters_per_fr; i++ )
      {
        iEstimatedDist += (((Int64)m_pixAcc_merged[i]) >> uiBitShift);
      }

      ruiDist = (iEstimatedDist > 0)?((UInt64)iEstimatedDist):(0);
      rdCost  = dEstimatedMinCost + (Double)(ruiDist);
      ruiRate = uiRate;

      ::memcpy(aiVarIndTabBest, m_varIndTab, sizeof(Int)*NO_VAR_BINS);
      for(Int i=0; i< ibestfilters_per_fr; i++ )
      {
        ::memcpy( ppiBestCoeffSet[i], m_filterCoeffSym[i], sizeof(Int) * m_pcTempAlfParam->num_coeff);
      }


    }
  }

  filtNo         = ibestfiltNo;
  filters_per_fr = ibestfilters_per_fr;
  ::memcpy(m_varIndTab, aiVarIndTabBest, sizeof(Int)*NO_VAR_BINS);
  for(Int i=0; i< filters_per_fr; i++ )
  {
    ::memcpy(m_filterCoeffSym[i], ppiBestCoeffSet[i], sizeof(Int) * pcAlfSaved->num_coeff);
  }
  xcalcPredFilterCoeff(filtNo);

  if (!m_iUsePreviousFilter)
  {
#if STAR_CROSS_SHAPES_LUMA
    saveFilterCoeffToBuffer(m_filterCoeffPrevSelected,filtNo);
#else
    saveFilterCoeffToBuffer(m_filterCoeffPrevSelected);
#endif
  }

  if( m_iUsePreviousFilter )
  {
    UInt64 uiOffRegionDistortion = 0;
    Int    iPelDiff;
    Pel*   pOrgTemp = (Pel*)ImgOrg;
    Pel*   pDecTemp = (Pel*)ImgDec;
    for(Int y=0; y< m_im_height; y++)
    {
      for(Int x=0; x< m_im_width; x++)
      {
        if(m_maskImg[y][x] == 0)
        {
          iPelDiff = pOrgTemp[x] - pDecTemp[x];
          uiOffRegionDistortion += (UInt64)(  (iPelDiff*iPelDiff) >> uiBitShift );
        }
      }
      pOrgTemp += Stride;
      pDecTemp += Stride;

      ruiDist += uiOffRegionDistortion;
      rdCost  += (Double)uiOffRegionDistortion;

    }
  }


}

#else
Void   TEncAdaptiveLoopFilter::xFirstFilteringFrameLumaAllTap(imgpel* ImgOrg, imgpel* ImgDec, imgpel* ImgRest, Int Stride)
{
  static Bool bFirst = true;
  static Int  aiVarIndTabBest[NO_VAR_BINS];
  static Double **ySym, ***ESym;
  static Int**  ppiBestCoeffSet;

  if(bFirst)
  {
    initMatrix_int(&ppiBestCoeffSet, NO_VAR_BINS, MAX_SQR_FILT_LENGTH);
    bFirst = false;
  }

  Int    lambda_val = ((Int) m_dLambdaLuma) * (1<<(2*g_uiBitIncrement));
  Int    filtNo, ibestfiltNo=0, filters_per_fr, ibestfilters_per_fr=0;
  Int64  iEstimatedDist;
  UInt64 uiRate;
  Double dEstimatedCost, dEstimatedMinCost = MAX_DOUBLE;;
#if STAR_CROSS_SHAPES_LUMA
  //prepare the 11x5 MatrixBase - correlation -  
  ESym     = m_EGlobalSym     [2];//Square11x5
  ySym     = m_yGlobalSym     [2];//Square11x5
  m_pcTempAlfParam->num_coeff = MAX_SQR_FILT_LENGTH;
#if MTK_NONCROSS_INLOOP_FILTER
  {
    if(!m_bUseNonCrossALF)
    {
      xstoreInBlockMatrix(0, 0, m_im_height, m_im_width, true, true, ImgOrg, ImgDec, 2, Stride);
    }
    else
    {
      xstoreInBlockMatrixforSlices(ImgOrg, ImgDec, 2, Stride);
    }
  }
#else
  xstoreInBlockMatrix(ImgOrg, ImgDec, 2, Stride);
#endif
  for (int filter_shape = 0; filter_shape < 2 ;filter_shape ++)
  {
    m_pcTempAlfParam->realfiltNo = filtNo = filter_shape;
    m_pcTempAlfParam->num_coeff = = m_sqrFiltLengthTab[filtNo];  

    ESym     = m_EGlobalSym     [filtNo];
    ySym     = m_yGlobalSym     [filtNo];

    xretriveBlockMatrix(m_pcTempAlfParam->num_coeff, m_iFilterTabIn11x5Sym[filtNo], 
                        m_EGlobalSym[2], ESym, 
                        m_yGlobalSym[2], ySym);
#else
  Bool   bMatrixBaseReady  = false;
  m_iMatrixBaseFiltNo = 0; 
  for(Int iTap = ALF_MAX_NUM_TAP; iTap>=ALF_MIN_NUM_TAP; iTap -= 2)
  {
    m_pcTempAlfParam->tap = iTap;
#if TI_ALF_MAX_VSIZE_7
    m_pcTempAlfParam->tapV      = TComAdaptiveLoopFilter::ALFTapHToTapV(iTap);
    m_pcTempAlfParam->num_coeff = TComAdaptiveLoopFilter::ALFTapHToNumCoeff(iTap);
#else
    m_pcTempAlfParam->num_coeff = iTap*iTap/4 + 2;
#endif
    if (iTap==9)
    {
      filtNo = 0;
    }
    else if (iTap==7)
    {
      filtNo = 1;
    }
    else
    {
      filtNo = 2;
    }

    ESym     = m_EGlobalSym     [filtNo];
    ySym     = m_yGlobalSym     [filtNo];

    if( bMatrixBaseReady )
    {
      xretriveBlockMatrix(m_pcTempAlfParam->num_coeff, m_iTapPosTabIn9x9Sym[filtNo], 
        m_EGlobalSym[m_iMatrixBaseFiltNo], ESym, 
        m_yGlobalSym[m_iMatrixBaseFiltNo], ySym);

    }
    else
#if MTK_NONCROSS_INLOOP_FILTER
    {
      if(!m_bUseNonCrossALF)
        xstoreInBlockMatrix(0, 0, m_im_height, m_im_width, true, true, ImgOrg, ImgDec, iTap, Stride);
      else
        xstoreInBlockMatrixforSlices(ImgOrg, ImgDec, iTap, Stride);
    }
#else
      xstoreInBlockMatrix(ImgOrg, ImgDec, iTap, Stride);
#endif
    if(filtNo == m_iMatrixBaseFiltNo)
    {
      bMatrixBaseReady = true;
    }
#endif

    xfindBestFilterVarPred(ySym, ESym, m_pixAcc, m_filterCoeffSym, m_filterCoeffSymQuant, filtNo, &filters_per_fr, 
      m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambda_val);

    uiRate         = xcodeFiltCoeff(m_filterCoeffSymQuant, filtNo, m_varIndTab, filters_per_fr,0, m_pcTempAlfParam);
    iEstimatedDist = xEstimateFiltDist(filters_per_fr, m_varIndTab, ESym, ySym, m_filterCoeffSym, m_pcTempAlfParam->num_coeff);
    dEstimatedCost = (Double)(uiRate) * m_dLambdaLuma + (Double)(iEstimatedDist);

    if(dEstimatedCost < dEstimatedMinCost)
    {
      dEstimatedMinCost   = dEstimatedCost;
      ibestfiltNo         = filtNo;
      ibestfilters_per_fr = filters_per_fr;
      copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam); 
      ::memcpy(aiVarIndTabBest, m_varIndTab, sizeof(Int)*NO_VAR_BINS);
      for(Int i=0; i< ibestfilters_per_fr; i++ )
      {
        ::memcpy( ppiBestCoeffSet[i], m_filterCoeffSym[i], sizeof(Int) * m_pcTempAlfParam->num_coeff);
      }
    }
  }

  filtNo         = ibestfiltNo;
  filters_per_fr = ibestfilters_per_fr;
  ::memcpy(m_varIndTab, aiVarIndTabBest, sizeof(Int)*NO_VAR_BINS);
  for(Int i=0; i< filters_per_fr; i++ )
  {
    ::memcpy(m_filterCoeffSym[i], ppiBestCoeffSet[i], sizeof(Int) * m_pcTempAlfParam->num_coeff);
  }

  xcalcPredFilterCoeff(filtNo);


  filtNo         = ibestfiltNo;
  filters_per_fr = ibestfilters_per_fr;
  copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
  ::memcpy(m_varIndTab, aiVarIndTabBest, sizeof(Int)*NO_VAR_BINS);
  for(Int i=0; i< filters_per_fr; i++ )
  {
    ::memcpy(m_filterCoeffSym[i], ppiBestCoeffSet[i], sizeof(Int) * m_pcTempAlfParam->num_coeff);
  }

  xcalcPredFilterCoeff(filtNo);
#if MTK_NONCROSS_INLOOP_FILTER
  if(!m_bUseNonCrossALF)
    xfilterFrame_en(0, 0, m_im_height, m_im_width, ImgDec, ImgRest, filtNo, Stride);
  else
    xfilterSlices_en(ImgDec, ImgRest, filtNo, Stride);
#else
  xfilterFrame_en(ImgDec, ImgRest, filtNo, Stride);
#endif

}


#endif


/** Retrieve correlations from other correlation matrix
 * \param iNumTaps number of filter taps
 * \param piTapPosInMaxFilter relative tap position in 9x9 footprint
 * \param pppdEBase base auto-correlation matrix
 * \param ppdyBase base cross-correlation array
 * \returns pppdETarget target auto-correlation matrix
 * \returns ppdyTarget target cross-correlation array
 */
Void TEncAdaptiveLoopFilter::xretriveBlockMatrix(Int iNumTaps, 
                                                 Int* piTapPosInMaxFilter, 
                                                 Double*** pppdEBase, Double*** pppdETarget, 
                                                 Double**  ppdyBase,  Double** ppdyTarget )
{
  Int varInd;
  Int i, j, r, c;

  Double** ppdSrcE;
  Double** ppdDstE;
  Double*  pdSrcy;
  Double*  pdDsty;

  for (varInd=0; varInd< NO_VAR_BINS; varInd++)
  {
    ppdSrcE = pppdEBase  [varInd];
    ppdDstE = pppdETarget[varInd];
    pdSrcy  = ppdyBase   [varInd];
    pdDsty  = ppdyTarget [varInd];

    for(j=0; j< iNumTaps; j++)
    {
      r = piTapPosInMaxFilter[j];

      for(i=j; i< iNumTaps; i++)
      {
        c = piTapPosInMaxFilter[i];

        //auto-correlation retrieval
        ppdDstE[j][i] = ppdSrcE[r][c];

      }

      //cross-correlation retrieval
      pdDsty[j] = pdSrcy[r];

    }

    //symmetric copy
    for(j=1; j< iNumTaps; j++)
      for(i=0; i< j; i++)
        ppdDstE[j][i] = ppdDstE[i][j];

  }

}

/** Estimate filtering distortion by correlation values and filter coefficients
 * \param ppdE auto-correlation matrix
 * \param pdy cross-correlation array
 * \param piCoeff  filter coefficients
 * \param iFiltLength numbr of filter taps
 * \returns estimated distortion
 */
Int64 TEncAdaptiveLoopFilter::xFastFiltDistEstimation(Double** ppdE, Double* pdy, Int* piCoeff, Int iFiltLength)
{
  //static memory
  static Bool     bFirst = true;
  static Double*  pdcoeff;
  if(bFirst)
  {
    pdcoeff= new Double[MAX_SQR_FILT_LENGTH];        
    bFirst= false;
  }

  //variable
  Int    i,j;
  Int64  iDist;
  Double dDist, dsum;


  for(i=0; i< iFiltLength; i++)
    pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<< (NUM_BITS - 1) );


  dDist =0;
  for(i=0; i< iFiltLength; i++)
  {
    dsum= ((Double)ppdE[i][i]) * pdcoeff[i];
    for(j=i+1; j< iFiltLength; j++)
      dsum += (Double)(2*ppdE[i][j])* pdcoeff[j];

    dDist += ((dsum - 2.0 * pdy[i])* pdcoeff[i] );
  }


  UInt uiShift = g_uiBitIncrement<<1;
  if(dDist < 0)
  {
    iDist = -(((Int64)(-dDist + 0.5)) >> uiShift);
  }
  else //dDist >=0
  {
    iDist= ((Int64)(dDist+0.5)) >> uiShift;
  }

  return iDist;

}


/** Estimate total filtering cost of all groups
 * \param filters_per_fr number of filters for the slice
 * \param VarIndTab merge index of all groups
 * \param pppdE  auto-correlation matrix pointer for all groups
 * \param ppdy cross-correlation array pointer for all groups
 * \returns estimated distortion
 */
Int64 TEncAdaptiveLoopFilter::xEstimateFiltDist(Int filters_per_fr, Int* VarIndTab, 
                                                Double*** pppdE, Double** ppdy, 
                                                Int** ppiCoeffSet, Int iFiltLength)

{
  Int64     iDist;
  Double**  ppdDstE;
  Double**  ppdSrcE;
  Double*   pdDsty;  
  Double*   pdSrcy;
  Int       f, j, i, varInd;
  Int*      piCoeff;

  //clean m_E_merged & m_y_merged
  for(f=0; f< filters_per_fr; f++)
  {
    for(j =0; j < iFiltLength; j++)
    {
      //clean m_E_merged one line
      for(i=0; i < iFiltLength; i++)
        m_E_merged[f][j][i] = 0;

      //clean m_y_merged
      m_y_merged[f][j] = 0;
    }
#if MQT_BA_RA
    m_pixAcc_merged[f] = 0;
#endif
  }


  //merge correlation values
  for (varInd=0; varInd< NO_VAR_BINS; varInd++)
  {
    ppdSrcE = pppdE[varInd];
    ppdDstE = m_E_merged[ VarIndTab[varInd] ];

    pdSrcy  = ppdy[varInd];
    pdDsty  = m_y_merged[ VarIndTab[varInd] ];

    for(j=0; j< iFiltLength; j++)
    {
      for(i=0; i< iFiltLength; i++)
        ppdDstE[j][i] += ppdSrcE[j][i];

      pdDsty[j] += pdSrcy[j];
    }
#if MQT_BA_RA
    m_pixAcc_merged[ VarIndTab[varInd]  ] += m_pixAcc[varInd];
#endif

  }

  //estimate distortion reduction by using FFDE (JCTVC-C143)
  iDist = 0;
  for(f=0; f< filters_per_fr; f++)
  {
    piCoeff = ppiCoeffSet[f];
    ppdDstE = m_E_merged [f];
    pdDsty  = m_y_merged [f];

    iDist += xFastFiltDistEstimation(ppdDstE, pdDsty, piCoeff, iFiltLength);
  }


  return iDist;

}
#endif

#if MTK_NONCROSS_INLOOP_FILTER

/** Calculate ALF grouping indices for ALF slices
 * \param varmap grouping indices buffer
 * \param imgY_Dec picture buffer
 * \param pad_size (max. filter tap)/2
 * \param fl  VAR_SIZE
 * \param img_stride picture buffer stride
 */
Void TEncAdaptiveLoopFilter::calcVarforSlices(imgpel **varmap, imgpel *imgY_Dec, Int pad_size, Int fl, Int img_stride)
{
#if MQT_BA_RA
  if(m_uiVarGenMethod == ALF_RA)
  {
    return;
  }
#endif

  Pel* pPicSrc   = (Pel *)imgY_Dec;
  Pel* pPicSlice = m_pcSliceYuvTmp->getLumaAddr();

  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    CAlfSlice* pSlice = &(m_pSlice[s]);

    pSlice->copySliceLuma(pPicSlice, pPicSrc, img_stride);
    pSlice->extendSliceBorderLuma(pPicSlice, img_stride, (UInt)EXTEND_NUM_PEL);
    calcVarforOneSlice(pSlice, varmap, (imgpel*)pPicSlice, pad_size, fl, img_stride);
  }
}


/** Calculate ALF grouping indices for ALF slices
 * \param varmap grouping indices buffer
 * \param imgY_Dec picture buffer
 * \param pad_size (max. filter tap)/2
 * \param fl  VAR_SIZE
 * \param img_stride picture buffer stride
 */
Void TEncAdaptiveLoopFilter::xfilterSlices_en(imgpel* ImgDec, imgpel* ImgRest,int filtNo, int iStride)
{
  Pel* pPicSrc   = (Pel *)ImgDec;
  Pel* pPicSlice = m_pcSliceYuvTmp->getLumaAddr();

  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    CAlfSlice* pSlice = &(m_pSlice[s]);

    pSlice->copySliceLuma(pPicSlice, pPicSrc, iStride);
    pSlice->extendSliceBorderLuma(pPicSlice, iStride, EXTEND_NUM_PEL);

    xfilterOneSlice_en(pSlice, (imgpel*)pPicSlice, ImgRest, filtNo, iStride);
  }
}

/** Filter one slice
 * \param pSlice slice parameters
 * \param ImgDec picture before filtering
 * \param ImgRest picture after filtering
 * \param filtNo  related to filter tap size
 * \param iStride picture buffer stride
 */
Void TEncAdaptiveLoopFilter::xfilterOneSlice_en(CAlfSlice* pSlice, imgpel* ImgDec, imgpel* ImgRest,int filtNo, int iStride)
{
  UInt uiNumLCUs = pSlice->getNumLCUs();

  Int iHeight, iWidth;
  Int ypos, xpos;

  for(UInt i=0; i< uiNumLCUs; i++)
  {
    CAlfLCU& cAlfLCU = (*pSlice)[i]; 
    for(UInt j=0; j< cAlfLCU.getNumSGU(); j++)
    {
      ypos    = (Int)(cAlfLCU[j].posY  );
      xpos    = (Int)(cAlfLCU[j].posX  );
      iHeight = (Int)(cAlfLCU[j].height);
      iWidth  = (Int)(cAlfLCU[j].width );
    
      xfilterFrame_en(ypos, xpos, iHeight, iWidth, ImgDec, ImgRest, filtNo, iStride);
    }
  }
}


/** Calculate block autocorrelations and crosscorrelations for ALF slices
 * \param ImgOrg original picture
 * \param ImgDec picture before filtering
 * \param tap  filter tap size
 * \param iStride picture buffer stride
 */
Void   TEncAdaptiveLoopFilter::xstoreInBlockMatrixforSlices(imgpel* ImgOrg, imgpel* ImgDec, Int tap, Int iStride)
{
  Pel* pPicSrc   = (Pel *)ImgDec;
  Pel* pPicSlice = m_pcSliceYuvTmp->getLumaAddr();

  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    CAlfSlice* pSlice = &(m_pSlice[s]);
    pSlice->copySliceLuma(pPicSlice, pPicSrc, iStride);
    pSlice->extendSliceBorderLuma(pPicSlice, iStride, (UInt)EXTEND_NUM_PEL);
    xstoreInBlockMatrixforOneSlice(pSlice, ImgOrg, (imgpel*)pPicSlice, tap, iStride, (s==0), (s== m_uiNumSlicesInPic-1));
  }
}

/** Calculate block autocorrelations and crosscorrelations for one ALF slices
 * \param pSlice slice parameters
 * \param ImgOrg original picture
 * \param ImgDec picture before filtering
 * \param tap  filter tap size
 * \param iStride picture buffer stride
 * \param bFirstSlice  true for the first processing slice of the picture
 * \param bLastSlice true for the last processing slice of the picture
 */
Void   TEncAdaptiveLoopFilter::xstoreInBlockMatrixforOneSlice(CAlfSlice* pSlice, 
                                                              imgpel* ImgOrg, imgpel* ImgDec, 
                                                              Int tap, Int iStride, 
                                                              Bool bFirstSlice, 
                                                              Bool bLastSlice
                                                              )
{


  UInt uiNumLCUs = pSlice->getNumLCUs();

  Int iHeight, iWidth;
  Int ypos, xpos;
  Bool bFirstLCU, bLastLCU;
  Bool bFirstSGU, bLastSGU;
  UInt uiNumSGUs;

  for(UInt i=0; i< uiNumLCUs; i++)
  {
    bFirstLCU = (i==0);
    bLastLCU  = (i== uiNumLCUs -1);

    CAlfLCU& cAlfLCU = (*pSlice)[i]; 
    uiNumSGUs = cAlfLCU.getNumSGU();

    for(UInt j=0; j< uiNumSGUs; j++)
    {
      bFirstSGU= (j ==0);
      bLastSGU = (j == uiNumSGUs -1);

      ypos    = (Int)(cAlfLCU[j].posY  );
      xpos    = (Int)(cAlfLCU[j].posX  );
      iHeight = (Int)(cAlfLCU[j].height);
      iWidth  = (Int)(cAlfLCU[j].width );

      xstoreInBlockMatrix(ypos, xpos, iHeight, iWidth, 
        (bFirstSlice && bFirstLCU && bFirstSGU),(bLastSlice && bLastLCU && bLastSGU),
        ImgOrg, ImgDec,tap, iStride);
    }
  }
}


/** Calculate autocorrelations and crosscorrelations for chroma slices
 * \param ComponentID Cb or Cr
 * \param pOrg original picture
 * \param pCmp picture before filtering
 * \param iTap  filter tap size
 * \param iOrgStride picture buffer stride for pOrg
 * \param iCmpStride picture buffer stride for pCmp
 */
Void TEncAdaptiveLoopFilter::xCalcCorrelationFuncforChromaSlices(Int ComponentID, Pel* pOrg, Pel* pCmp, Int iTap, Int iOrgStride, Int iCmpStride)
{

  assert(iOrgStride == iCmpStride);

  Pel* pPicSrc   = pCmp;
  Pel* pPicSlice = (ComponentID == ALF_Cb)?(m_pcSliceYuvTmp->getCbAddr()):(m_pcSliceYuvTmp->getCrAddr());

  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    CAlfSlice* pSlice = &(m_pSlice[s]);

    pSlice->copySliceChroma(pPicSlice, pPicSrc, iCmpStride);
    pSlice->extendSliceBorderChroma(pPicSlice, iCmpStride, (UInt)EXTEND_NUM_PEL_C);

    xCalcCorrelationFuncforChromaOneSlice(pSlice, pOrg, pPicSlice, iTap, iCmpStride,(s==m_uiNumSlicesInPic-1));
  }
}

/** Calculate autocorrelations and crosscorrelations for one chroma slice
 * \param pSlice slice parameters
 * \param pOrg original picture
 * \param pCmp picture before filtering
 * \param iTap  filter tap size
 * \param iStride picture buffer stride
 * \param bLastSlice the last processing slice of picture
 */
Void TEncAdaptiveLoopFilter::xCalcCorrelationFuncforChromaOneSlice(CAlfSlice* pSlice, Pel* pOrg, Pel* pCmp, Int iTap, Int iStride, Bool bLastSlice)
{
  UInt uiNumLCUs = pSlice->getNumLCUs();

  Int iHeight, iWidth;
  Int ypos, xpos;
  Bool bLastLCU;
  Bool bLastSGU;
  UInt uiNumSGUs;

  for(UInt i=0; i< uiNumLCUs; i++)
  {
    bLastLCU  = (i== uiNumLCUs -1);

    CAlfLCU& cAlfLCU = (*pSlice)[i]; 
    uiNumSGUs = cAlfLCU.getNumSGU();

    for(UInt j=0; j< uiNumSGUs; j++)
    {
      bLastSGU = (j == uiNumSGUs -1);
      ypos    = (Int)(cAlfLCU[j].posY   >>1);
      xpos    = (Int)(cAlfLCU[j].posX   >>1);
      iHeight = (Int)(cAlfLCU[j].height >>1);
      iWidth  = (Int)(cAlfLCU[j].width  >>1);
      xCalcCorrelationFunc(ypos, xpos, pOrg, pCmp, iTap, iWidth, iHeight, iStride, iStride, (bLastSlice && bLastLCU && bLastSGU) );
    }
  }
}

/** Calculate block autocorrelations and crosscorrelations for one chroma slice
 * \param ComponentID slice parameters
 * \param pcPicDecYuv original picture
 * \param pcPicRestYuv picture before filtering
 * \param qh  filter tap size
 * \param iTap picture buffer stride
 */
Void TEncAdaptiveLoopFilter::xFrameChromaforSlices(Int ComponentID, TComPicYuv* pcPicDecYuv, TComPicYuv* pcPicRestYuv, Int *qh, Int iTap )
{
  Pel* pPicDec   = (ComponentID == ALF_Cb)?(    pcPicDecYuv->getCbAddr()):(    pcPicDecYuv->getCrAddr());
  //  Pel* pPicRest  = (ComponentID == ALF_Cb)?(   pcPicRestYuv->getCbAddr()):(   pcPicRestYuv->getCrAddr());
  Pel* pPicSlice = (ComponentID == ALF_Cb)?(m_pcSliceYuvTmp->getCbAddr()):(m_pcSliceYuvTmp->getCrAddr());

  Int iStride = pcPicDecYuv->getCStride();

  assert(iStride == pcPicRestYuv->getCStride());

  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    CAlfSlice* pSlice = &(m_pSlice[s]);

    pSlice->copySliceChroma(pPicSlice, pPicDec, iStride);
    pSlice->extendSliceBorderChroma(pPicSlice, iStride, (UInt)EXTEND_NUM_PEL_C);

    xFrameChromaforOneSlice(pSlice, ComponentID, m_pcSliceYuvTmp, pcPicRestYuv, qh, iTap);
  }
}


/** Copy CU control flags from TComCU
 * \param bCUCtrlEnabled true for ALF CU control enabled
 * \param iCUCtrlDepth ALF CU Control depth
 */
Void TEncAdaptiveLoopFilter::getCtrlFlagsForSlices(Bool bCUCtrlEnabled, Int iCUCtrlDepth)
{
  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    CAlfSlice& cSlice = m_pSlice[s];

    cSlice.setCUCtrlEnabled(bCUCtrlEnabled);
    if(bCUCtrlEnabled)
    {
      cSlice.setCUCtrlDepth(iCUCtrlDepth);
      cSlice.getCtrlFlagsForOneSlice();
    }
  }
}

/** Copy CU control flags to ALF parameters
 * \return ruiNumFlags reference to the flag of number of ALF CU control flags
 * \return puiFlags pointer to the buffer of ALF CU control flags
 */
Void TEncAdaptiveLoopFilter::transferCtrlFlagsToAlfParam(UInt& ruiNumFlags, UInt* puiFlags)
{
  UInt* puiSliceFlags = puiFlags;

  ruiNumFlags = 0;
  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    CAlfSlice& cSlice = m_pSlice[s];

    for(UInt idx=0; idx < cSlice.getNumLCUs(); idx++)
    {
      CAlfLCU& cAlfLCU = cSlice[idx];
      UInt uiNumFlags= cAlfLCU.getNumCtrlFlags();

      for(UInt i=0; i< uiNumFlags; i++)
      {
        puiSliceFlags[i] = cAlfLCU.getCUCtrlFlag(i);
      }

      puiSliceFlags += uiNumFlags;
      ruiNumFlags += uiNumFlags;
    }

  }


}




#endif

#if ALF_CHROMA_NEW_SHAPES
// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncAdaptiveLoopFilter::xFilterTapDecisionChroma( UInt64 uiLumaRate, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, UInt64& ruiBits )
{
  Int   iShape, num_coeff;
  Int64 iOrgDistCb, iOrgDistCr, iFiltDistCb, iFiltDistCr, iDist;
  Bool  bChanged = false;
  Int*  qh = m_pcTempAlfParam->coeff_chroma;

  UInt64 uiMinRate = uiLumaRate;
  UInt64 uiMinDist = MAX_INT;
  Double dMinCost  = MAX_DOUBLE;
  Double dLocalMinCost = MAX_DOUBLE;

  copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
  xCalcRDCostChroma(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiMinRate, uiMinDist, dMinCost);

  for(iShape = 0; iShape < 2; iShape++)
  {
    // set global variables
    num_coeff = m_sqrFiltLengthTab[iShape];
    m_pcTempAlfParam->chroma_idc = 3;
    m_pcTempAlfParam->realfiltNo_chroma = iShape;
    m_pcTempAlfParam->num_coeff_chroma = num_coeff;

    // keep original corr pointer
    Double **ppdTmpCorr = m_ppdAlfCorr;

    // calc Cb matrix
    m_pcTempAlfParam->chroma_idc = 2;
    m_ppdAlfCorr = m_ppdAlfCorrCb;
    for(Int i=0; i<ALF_MAX_NUM_COEF; i++)
    {
      ::memset(m_ppdAlfCorr[i], 0, sizeof(Double) * ALF_MAX_NUM_COEF + 1);
    }
    Pel *pOrg = pcPicOrg->getCbAddr();
    Pel *pCmp = pcPicDec->getCbAddr();
#if MTK_NONCROSS_INLOOP_FILTER
    if(!m_bUseNonCrossALF)
      xCalcCorrelationFunc(0, 0, pOrg, pCmp, iShape, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride(), true);
    else
      xCalcCorrelationFuncforChromaSlices(ALF_Cb, pOrg, pCmp, iShape, pcPicOrg->getCStride(), pcPicDec->getCStride());
#else        
    xCalcCorrelationFunc(pOrg, pCmp, iShape, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride());
#endif

    // calc Cr matrix
    m_pcTempAlfParam->chroma_idc = 1;
    m_ppdAlfCorr = m_ppdAlfCorrCr;
    for(Int i=0; i<ALF_MAX_NUM_COEF; i++)
    {
      ::memset(m_ppdAlfCorr[i], 0, sizeof(Double) * ALF_MAX_NUM_COEF + 1);
    }
    pOrg = pcPicOrg->getCrAddr();
    pCmp = pcPicDec->getCrAddr();
#if MTK_NONCROSS_INLOOP_FILTER
    if(!m_bUseNonCrossALF)
      xCalcCorrelationFunc(0, 0, pOrg, pCmp, iShape, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride(), true);
    else
      xCalcCorrelationFuncforChromaSlices(ALF_Cb, pOrg, pCmp, iShape, pcPicOrg->getCStride(), pcPicDec->getCStride());
#else        
    xCalcCorrelationFunc(pOrg, pCmp, iShape, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride());
#endif

    // restore original corr pointer
    m_ppdAlfCorr = ppdTmpCorr;

    // calc original dist
    memset(qh, 0, sizeof(Int)*num_coeff);
    qh[num_coeff-2] = 1<<(NUM_BITS - 1);
    iOrgDistCb = xFastFiltDistEstimationChroma(m_ppdAlfCorrCb, qh, num_coeff);
    iOrgDistCr = xFastFiltDistEstimationChroma(m_ppdAlfCorrCr, qh, num_coeff);

    for(Int iCmp=1; iCmp<=3; iCmp++)
    {
      m_pcTempAlfParam->chroma_idc = iCmp;
      xCalcALFCoeffChroma(iCmp, iShape, qh);
      iFiltDistCb = ((iCmp>>1)&0x1) ? xFastFiltDistEstimationChroma(m_ppdAlfCorrCb, qh, num_coeff) : iOrgDistCb;
      iFiltDistCr = ((iCmp)   &0x1) ? xFastFiltDistEstimationChroma(m_ppdAlfCorrCr, qh, num_coeff) : iOrgDistCr;
      iDist = iFiltDistCb + iFiltDistCr;
      UInt64 uiRate = xCalcRateChroma(m_pcTempAlfParam);
      Double dCost  = (Double)iDist + m_dLambdaChroma * (Double)uiRate;
      if(dCost < dLocalMinCost)
      {
        dLocalMinCost = dCost;
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
        bChanged = true;
      }
    }
  }
  copyALFParam(m_pcTempAlfParam, m_pcBestAlfParam);
  if(!bChanged)
  {
    m_pcBestAlfParam->chroma_idc = 0;
    return;
  }

  // Adaptive in-loop wiener filtering for chroma
  xFilteringFrameChroma(pcPicOrg, pcPicDec, pcPicRest);
  
  // filter on/off decision for chroma
  Int iCWidth = (pcPicOrg->getWidth()>>1);
  Int iCHeight = (pcPicOrg->getHeight()>>1);
  Int iCStride = pcPicOrg->getCStride();
  UInt64 uiFiltDistCb = xCalcSSD(pcPicOrg->getCbAddr(), pcPicRest->getCbAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiFiltDistCr = xCalcSSD(pcPicOrg->getCrAddr(), pcPicRest->getCrAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiOrgDistCb  = xCalcSSD(pcPicOrg->getCbAddr(), pcPicDec->getCbAddr(), iCWidth, iCHeight, iCStride);
  UInt64 uiOrgDistCr  = xCalcSSD(pcPicOrg->getCrAddr(), pcPicDec->getCrAddr(), iCWidth, iCHeight, iCStride);

  if(((m_pcTempAlfParam->chroma_idc)>>1 & 0x1) && (uiOrgDistCb<=uiFiltDistCb))
  {
    m_pcTempAlfParam->chroma_idc -= 2;
    pcPicDec->copyToPicCb(pcPicRest);
  }
  if(((m_pcTempAlfParam->chroma_idc)    & 0x1) && (uiOrgDistCr<=uiFiltDistCr))
  {
    m_pcTempAlfParam->chroma_idc -= 1;
    pcPicDec->copyToPicCr(pcPicRest);
  }
  
  if(m_pcTempAlfParam->chroma_idc)
  {
    UInt64 uiRate, uiDist;
    Double dCost;
    xCalcRDCostChroma(pcPicOrg, pcPicRest, m_pcTempAlfParam, uiRate, uiDist, dCost);
    
    if( dCost < dMinCost )
    {
      copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
      predictALFCoeffChroma(m_pcBestAlfParam);
      
      ruiBits += uiRate;
      ruiDist += uiDist;
    }
    else
    {
      m_pcBestAlfParam->chroma_idc = 0;
      
      if((m_pcTempAlfParam->chroma_idc>>1)&0x01)
        pcPicDec->copyToPicCb(pcPicRest);
      if(m_pcTempAlfParam->chroma_idc&0x01)
        pcPicDec->copyToPicCr(pcPicRest);
      
      ruiBits += uiMinRate;
      ruiDist += uiMinDist;
    }
  }
  else
  {
    m_pcBestAlfParam->chroma_idc = 0;
    
    ruiBits += uiMinRate;
    ruiDist += uiMinDist;
    
    pcPicDec->copyToPicCb(pcPicRest);
    pcPicDec->copyToPicCr(pcPicRest);
  }
}

Int64 TEncAdaptiveLoopFilter::xFastFiltDistEstimationChroma(Double** ppdCorr, Int* piCoeff, Int iSqrFiltLength)
{
  //static memory
  static Bool     bFirst = true;
  static Double*  pdcoeff;
  if(bFirst)
  {
    pdcoeff= new Double[MAX_SQR_FILT_LENGTH];
    bFirst= false;
  }

  //variable
  Int    i,j;
  Int64  iDist;
  Double dDist, dsum;

  memset(pdcoeff, 0, sizeof(Double)*MAX_SQR_FILT_LENGTH);

  for(i=0; i< iSqrFiltLength-1; i++)
  {
    pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<< (NUM_BITS - 1) );
  }
  pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<< (NUM_BITS - 1 - g_uiBitIncrement) );

  dDist =0;
  for(i=0; i< iSqrFiltLength; i++)
  {
    dsum= ((Double)ppdCorr[i][i]) * pdcoeff[i];
    for(j=i+1; j< iSqrFiltLength; j++)
      dsum += (Double)(2*ppdCorr[i][j])* pdcoeff[j];

    dDist += ((dsum - 2.0 * ppdCorr[i][iSqrFiltLength])* pdcoeff[i] );
  }

  UInt uiShift = g_uiBitIncrement<<1;
  if(dDist < 0)
  {
    iDist = -(((Int64)(-dDist + 0.5)) >> uiShift);
  }
  else //dDist >=0
  {
    iDist= ((Int64)(dDist+0.5)) >> uiShift;
  }

  return iDist;
}

Void TEncAdaptiveLoopFilter::xCalcALFCoeffChroma(Int iChromaIdc, Int iShape, Int* piCoeff)
{
  Int iSqrFiltLength = m_sqrFiltLengthTab[iShape];

  for(Int i=0; i<iSqrFiltLength; i++)
  {
    memset(m_ppdAlfCorr[i], 0, sizeof(Double)*(iSqrFiltLength + 1));
  }

  // retrive
  if((iChromaIdc>>1) & 0x1)
  {
    for(Int i=0; i<iSqrFiltLength; i++)
    {
      for(Int j=i; j<iSqrFiltLength+1; j++)
      {
        m_ppdAlfCorr[i][j] += m_ppdAlfCorrCb[i][j];
      }
    }
  }
  if(iChromaIdc & 0x1)
  {
    for(Int i=0; i<iSqrFiltLength; i++)
    {
      for(Int j=i; j<iSqrFiltLength+1; j++)
      {
        m_ppdAlfCorr[i][j] += m_ppdAlfCorrCr[i][j];
      }
    }
  }

  // copy
  for(Int i=1; i<iSqrFiltLength; i++)
  {
    for(Int j=0; j<i; j++)
    {
      m_ppdAlfCorr[i][j] = m_ppdAlfCorr[j][i];
    }
  }

  Double *corr = new Double[iSqrFiltLength];
  for(Int i=0; i<iSqrFiltLength; i++)
  {
    corr[i] = m_ppdAlfCorr[i][iSqrFiltLength];
  }

  // calc coeff
  gnsSolveByChol(m_ppdAlfCorr, corr, m_pdDoubleAlfCoeff, iSqrFiltLength);
  xQuantFilterCoef(m_pdDoubleAlfCoeff, piCoeff, iShape, g_uiBitDepth + g_uiBitIncrement);
  delete [] corr;
}

UInt64 TEncAdaptiveLoopFilter::xCalcRateChroma(ALFParam* pAlfParam)
{
  UInt64 uiRate;
#if E045_SLICE_COMMON_INFO_SHARING
  uiRate = 0;
#endif
  Int* piTmpCoef;
  piTmpCoef = new Int[ALF_MAX_NUM_COEF_C];

  memcpy(piTmpCoef, pAlfParam->coeff_chroma, sizeof(Int)*pAlfParam->num_coeff_chroma);

  predictALFCoeffChroma(pAlfParam);

  m_pcEntropyCoder->resetEntropy();
  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->encodeAlfParam(pAlfParam);

#if E045_SLICE_COMMON_INFO_SHARING
  if(m_uiNumSlicesInPic ==1)
  {
    if(m_bSharedPPSAlfParamEnabled)
    {
      uiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
      m_pcEntropyCoder->resetEntropy();
      m_pcEntropyCoder->resetBits();
    }

    m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
    uiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
  }
  else
  {
    uiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
    for(UInt s=0; s< m_uiNumSlicesInPic; s++)
    {
      m_pcEntropyCoder->resetEntropy();
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam, m_uiNumSlicesInPic, &(m_pSlice[s]));
      uiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
    }
  }
#else
  if(pAlfParam->cu_control_flag)
  {
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
    m_pcEntropyCoder->setSliceGranularity(m_iSGDepth);
#endif

    m_pcEntropyCoder->encodeAlfCtrlParam(pAlfParam);
  }
  uiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
#endif
  memcpy(pAlfParam->coeff_chroma, piTmpCoef, sizeof(int)*pAlfParam->num_coeff_chroma);
  delete[] piTmpCoef;
  piTmpCoef = NULL;

  return uiRate;
}
#endif

#if MTK_SAO
inline Double xRoundIbdi2(Double x)
{
#if FULL_NBIT
  Int bitDepthMinus8 = g_uiBitDepth - 8;
  return ((x)>0) ? (Int)(((Int)(x)+(1<<(bitDepthMinus8-1)))/(1<<bitDepthMinus8)) : ((Int)(((Int)(x)-(1<<(bitDepthMinus8-1)))/(1<<bitDepthMinus8)));
#else
  return ((x)>0) ? (Int)(((Int)(x)+(1<<(g_uiBitIncrement-1)))/(1<<g_uiBitIncrement)) : ((Int)(((Int)(x)-(1<<(g_uiBitIncrement-1)))/(1<<g_uiBitIncrement)));
#endif
}

inline Double xRoundIbdi(Double x)
{
#if FULL_NBIT
  return (g_uiBitDepth > 8 ? xRoundIbdi2((x)) : ((x)>=0 ? ((Int)((x)+0.5)) : ((Int)((x)-0.5)))) ;
#else
  return (g_uiBitIncrement >0 ? xRoundIbdi2((x)) : ((x)>=0 ? ((Int)((x)+0.5)) : ((Int)((x)-0.5)))) ;
#endif
}

/** run QAO One Part.
 * \param  pQAOOnePart, iPartIdx
 */
Void TEncSampleAdaptiveOffset::xQAOOnePart(SAOQTPart *psQTPart, Int iPartIdx, Double dLambda)
{
  Int iTypeIdx;
  Int iNumTotalType = MAX_NUM_SAO_TYPE;
  SAOQTPart*  pOnePart = &(psQTPart[iPartIdx]);

  Int64 iEstDist;
  Int64 iOffsetOrg;
  Int64 iOffset;
  Int64 iCount;
  Int iClassIdx;
  Int uiShift = g_uiBitIncrement << 1;
  //   Double dAreaWeight =  (pOnePart->part_xe - pOnePart->part_xs + 1) * (pOnePart->part_ye - pOnePart->part_ys + 1);
  Double dAreaWeight =  0;
  Double dComplexityCost = 0;
  Int    iQaoPara1 = SAO_RDCO;

  UInt uiDepth = pOnePart->PartLevel;

  //   m_iDistOrg [iPartIdx] = 0;

  m_iDistOrg [iPartIdx] =  (Int64)((Double)(iQaoPara1)/10000 * dLambda * dAreaWeight);

  for (iTypeIdx=-1; iTypeIdx<iNumTotalType; iTypeIdx++)
  {
    if( m_bUseSBACRD )
    {
      m_pcRDGoOnSbacCoder->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]); // pcCU->getDepth(0) ==> m_puhDepth[uiIdx]
      m_pcRDGoOnSbacCoder->resetBits();
    }
    else
    {
      m_pcEntropyCoder->resetEntropy();
      m_pcEntropyCoder->resetBits();
    }

    iEstDist = 0;

    m_pcEntropyCoder->m_pcEntropyCoderIf->codeAoUvlc(iTypeIdx+1);

    if (iTypeIdx>=0)
    {

      for(iClassIdx=1; iClassIdx < m_iNumClass[iTypeIdx]+1; iClassIdx++)
      {
        if(m_iCount [iPartIdx][iTypeIdx][iClassIdx])
        {
#if SAO_ACCURATE_OFFSET
#if FULL_NBIT
          m_iOffset[iPartIdx][iTypeIdx][iClassIdx]    = (Int64) xRoundIbdi((Double)(m_iOffsetOrg[iPartIdx][iTypeIdx][iClassIdx]<<g_uiBitDepth-8) / (Double)(m_iCount [iPartIdx][iTypeIdx][iClassIdx]<<m_uiAoBitDepth));
#else
          m_iOffset[iPartIdx][iTypeIdx][iClassIdx]    = (Int64) xRoundIbdi((Double)(m_iOffsetOrg[iPartIdx][iTypeIdx][iClassIdx]<<g_uiBitIncrement) / (Double)(m_iCount [iPartIdx][iTypeIdx][iClassIdx]<<m_uiAoBitDepth));
#endif
#else
          m_iOffset[iPartIdx][iTypeIdx][iClassIdx]    = (Int64) xRoundIbdi((Double)(m_iOffsetOrg[iPartIdx][iTypeIdx][iClassIdx]<<m_uiAoBitDepth) / (Double)m_iCount [iPartIdx][iTypeIdx][iClassIdx]);
#endif
#if SAO_CLIP_OFFSET
          m_iOffset[iPartIdx][iTypeIdx][iClassIdx] = Clip3(-m_iOffsetTh, m_iOffsetTh-1, (Int)m_iOffset[iPartIdx][iTypeIdx][iClassIdx]);
#endif
        }
        else
        {
          m_iOffsetOrg[iPartIdx][iTypeIdx][iClassIdx] = 0;
          m_iOffset[iPartIdx][iTypeIdx][iClassIdx] = 0;
        }

        iCount     =  m_iCount [iPartIdx][iTypeIdx][iClassIdx];
#if SAO_ACCURATE_OFFSET
        iOffset    =  m_iOffset[iPartIdx][iTypeIdx][iClassIdx] << m_uiAoBitDepth;
#else
#if FULL_NBIT
        iOffset    =  m_iOffset[iPartIdx][iTypeIdx][iClassIdx] << (g_uiBitDepth-8-m_uiAoBitDepth);
#else
        iOffset    =  m_iOffset[iPartIdx][iTypeIdx][iClassIdx] << (g_uiBitIncrement-m_uiAoBitDepth);
#endif
#endif
        iOffsetOrg =  m_iOffsetOrg[iPartIdx][iTypeIdx][iClassIdx];
        iEstDist   += (( iCount*iOffset*iOffset-iOffsetOrg*iOffset*2 ) >> uiShift);
        m_pcEntropyCoder->m_pcEntropyCoderIf->codeAoSvlc((Int)m_iOffset[iPartIdx][iTypeIdx][iClassIdx]);
      }
      m_iDist[iPartIdx][iTypeIdx] = iEstDist;
      m_iRate[iPartIdx][iTypeIdx] = m_pcEntropyCoder->getNumberOfWrittenBits();

      m_dCost[iPartIdx][iTypeIdx] = (Double)((Double)m_iDist[iPartIdx][iTypeIdx] + dLambda * (Double) m_iRate[iPartIdx][iTypeIdx]);
      dComplexityCost = (Double)(iQaoPara1)/10000 * dLambda * (Double)m_iWeightAO[iTypeIdx] * dAreaWeight;
      m_dCost[iPartIdx][iTypeIdx] = (Double)((Double)m_iDist[iPartIdx][iTypeIdx] + dLambda * (Double) m_iRate[iPartIdx][iTypeIdx]) + dComplexityCost;

      //       printf("\n%3d:%10.f, %10.0f, %10.0f",iPartIdx,(Double)m_iDist[iPartIdx][iTypeIdx], dComplexityCost);

      //        printf("\n%d, %d, %6d, %6d, %f", iPartIdx, iTypeIdx, (Int)m_iDist[iPartIdx][iTypeIdx], (Int)m_iRate[iPartIdx][iTypeIdx], m_dCost[iPartIdx][iTypeIdx]);
      if(m_dCost[iPartIdx][iTypeIdx] < m_dCostPartBest[iPartIdx])
      {
        m_iDistOrg [iPartIdx] = (Int64)dComplexityCost;
        m_dCostPartBest[iPartIdx] = m_dCost[iPartIdx][iTypeIdx];
        m_iTypePartBest[iPartIdx] = iTypeIdx;
        if( m_bUseSBACRD )
          m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[pOnePart->PartLevel][CI_TEMP_BEST] );
      }
    }
    else
    {

      if(m_iDistOrg[iPartIdx] < m_dCostPartBest[iPartIdx] )
      {
        m_dCostPartBest[iPartIdx] = (Double) m_iDistOrg[iPartIdx] + m_pcEntropyCoder->getNumberOfWrittenBits()*dLambda ; 
        m_iTypePartBest[iPartIdx] = -1;
        if( m_bUseSBACRD )
          m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[pOnePart->PartLevel][CI_TEMP_BEST] );
      }
    }
  }

  pOnePart->bProcessed = true;
  pOnePart->bSplit     = false;
  pOnePart->iMinDist   =        m_iTypePartBest[iPartIdx] >= 0 ? m_iDist[iPartIdx][m_iTypePartBest[iPartIdx]] : m_iDistOrg[iPartIdx];
  pOnePart->iMinRate   = (Int) (m_iTypePartBest[iPartIdx] >= 0 ? m_iRate[iPartIdx][m_iTypePartBest[iPartIdx]] : 0);
  pOnePart->dMinCost   = pOnePart->iMinDist + dLambda * pOnePart->iMinRate;
  pOnePart->iBestType  = m_iTypePartBest[iPartIdx];
  if (pOnePart->iBestType != -1)
  {
    pOnePart->bEnableFlag =  1;
    pOnePart->iLength = m_iNumClass[pOnePart->iBestType];
    for (Int i=0; i<pOnePart->iLength ; i++)
      pOnePart->iOffset[i] = (Int) m_iOffset[iPartIdx][pOnePart->iBestType][i+1];
  }
  else
  {
    pOnePart->bEnableFlag =  0;
    pOnePart->iLength     =  0;
  }

}

/** run Part Tree Disable.
 * \param  pQAOOnePart, iPartIdx
 */
Void TEncSampleAdaptiveOffset::xPartTreeDisable(SAOQTPart *psQTPart, Int iPartIdx)
{
  SAOQTPart*  pOnePart= &(psQTPart[iPartIdx]);

  pOnePart->bEnableFlag = false;
  pOnePart->bSplit      = false;
  pOnePart->iLength     =  0;
  pOnePart->iBestType   = -1;

  if (pOnePart->PartLevel < m_uiMaxSplitLevel)
  {
    for (Int i=0; i<NUM_DOWN_PART; i++)
    {
      xPartTreeDisable(psQTPart, pOnePart->DownPartsIdx[i]);
    }
  }

}


/** run QuadTree Decision Function.
 * \param  iPartIdx, pcPicOrg, pcPicDec, pcPicRest, &dCostFinal
 */
Void TEncSampleAdaptiveOffset::xQuadTreeDecisionFunc(SAOQTPart *psQTPart, Int iPartIdx, Double &dCostFinal, Int iMaxLevel, Double dLambda)
{
  SAOQTPart*  pOnePart = &(psQTPart[iPartIdx]);

  UInt uiDepth = pOnePart->PartLevel;
  UInt uhNextDepth = uiDepth+1;

  if (iPartIdx == 0)
  {
    dCostFinal = 0;
  }

  //QAO for this part
  if(!pOnePart->bProcessed)
  {
    xQAOOnePart (psQTPart, iPartIdx, dLambda);
  }

  //QAO for sub 4 parts
  if (pOnePart->PartLevel < iMaxLevel)
  {
    Double      dCostNotSplit = dLambda + pOnePart->dMinCost;
    Double      dCostSplit    = dLambda;

    for (Int i=0; i< NUM_DOWN_PART ;i++)
    {
      if( m_bUseSBACRD )  
      {
        if ( 0 == i) //initialize RD with previous depth buffer
        {
          m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uiDepth][CI_CURR_BEST]);
        }
        else
        {
          m_pppcRDSbacCoder[uhNextDepth][CI_CURR_BEST]->load(m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]);
        }
      }  
      xQuadTreeDecisionFunc(psQTPart, pOnePart->DownPartsIdx[i], dCostFinal, iMaxLevel, dLambda);
      dCostSplit += dCostFinal;
      if( m_bUseSBACRD )
      {
        m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]->load(m_pppcRDSbacCoder[uhNextDepth][CI_TEMP_BEST]);
      }
    }


    if(dCostSplit < dCostNotSplit)
    {
      dCostFinal = dCostSplit;
      pOnePart->bSplit      = true;
      pOnePart->bEnableFlag = false;
      pOnePart->iLength     =  0;
      pOnePart->iBestType   = -1;
      if( m_bUseSBACRD )
      {
        m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]->load(m_pppcRDSbacCoder[uhNextDepth][CI_NEXT_BEST]);
      }
    }
    else
    {
      dCostFinal = dCostNotSplit;
      pOnePart->bSplit = false;
      for (Int i=0; i<NUM_DOWN_PART; i++)
      {
        xPartTreeDisable(psQTPart, pOnePart->DownPartsIdx[i]);
      }
      if( m_bUseSBACRD )
      {
        m_pppcRDSbacCoder[uiDepth][CI_NEXT_BEST]->load(m_pppcRDSbacCoder[uiDepth][CI_TEMP_BEST]);
      }
    }
  }
  else
  {
    dCostFinal = pOnePart->dMinCost;
  }
}
/** delete allocated memory of TEncSampleAdaptiveOffset class.
 */
Void TEncSampleAdaptiveOffset::destroyEncBuffer()
{

    for (Int i=0;i<m_iNumTotalParts;i++)
    {
      for (Int j=0;j<MAX_NUM_SAO_TYPE;j++)
      {
        if (m_iCount [i][j])
        {
          delete [] m_iCount [i][j]; 
        }
        if (m_iOffset[i][j])
        {
          delete [] m_iOffset[i][j]; 
        }
        if (m_iOffsetOrg[i][j])
        {
          delete [] m_iOffsetOrg[i][j]; 
        }
      }
      if (m_iRate[i])
      {
        delete [] m_iRate[i];
      }
      if (m_iDist[i])
      {
        delete [] m_iDist[i]; 
      }
      if (m_dCost[i])
      {
        delete [] m_dCost[i]; 
      }
      if (m_iCount [i])
      {
        delete [] m_iCount [i]; 
      }
      if (m_iOffset[i])
      {
        delete [] m_iOffset[i]; 
      }
      if (m_iOffsetOrg[i])
      {
        delete [] m_iOffsetOrg[i]; 
      }

    }
    if (m_iDistOrg)
    {
      delete [] m_iDistOrg ; m_iDistOrg = NULL;
    }
    if (m_dCostPartBest)
    {
      delete [] m_dCostPartBest ; m_dCostPartBest = NULL;
    }
    if (m_iTypePartBest)
    {
      delete [] m_iTypePartBest ; m_iTypePartBest = NULL;
    }
    if (m_iRate)
    {
      delete [] m_iRate ; m_iRate = NULL;
    }
    if (m_iDist)
    {
      delete [] m_iDist ; m_iDist = NULL;
    }
    if (m_dCost)
    {
      delete [] m_dCost ; m_dCost = NULL;
    }
    if (m_iCount)
    {
      delete [] m_iCount  ; m_iCount = NULL;
    }
    if (m_iOffset)
    {
      delete [] m_iOffset ; m_iOffset = NULL;
    }
    if (m_iOffsetOrg)
    {
      delete [] m_iOffsetOrg ; m_iOffsetOrg = NULL;
    }

    {
      Int iMaxDepth = 4;
      Int iDepth;
      for ( iDepth = 0; iDepth < iMaxDepth+1; iDepth++ )
      {
        for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
        {
          delete m_pppcRDSbacCoder[iDepth][iCIIdx];
          delete m_pppcBinCoderCABAC[iDepth][iCIIdx];
        }
      }

      for ( iDepth = 0; iDepth < iMaxDepth+1; iDepth++ )
      {
        delete [] m_pppcRDSbacCoder[iDepth];
        delete [] m_pppcBinCoderCABAC[iDepth];
      }

      delete [] m_pppcRDSbacCoder;
      delete [] m_pppcBinCoderCABAC;
    }

}
Void TEncSampleAdaptiveOffset::createEncBuffer()
{
    m_iDistOrg = new Int64 [m_iNumTotalParts]; 
    m_dCostPartBest = new Double [m_iNumTotalParts]; 
    m_iTypePartBest = new Int [m_iNumTotalParts]; 

    m_iRate = new Int64* [m_iNumTotalParts];
    m_iDist = new Int64* [m_iNumTotalParts];
    m_dCost = new Double*[m_iNumTotalParts];

    m_iCount  = new Int64 **[m_iNumTotalParts];
    m_iOffset = new Int64 **[m_iNumTotalParts];
    m_iOffsetOrg = new Int64 **[m_iNumTotalParts];

    for (Int i=0;i<m_iNumTotalParts;i++)
    {
      m_iRate[i] = new Int64  [MAX_NUM_SAO_TYPE];
      m_iDist[i] = new Int64  [MAX_NUM_SAO_TYPE]; 
      m_dCost[i] = new Double [MAX_NUM_SAO_TYPE]; 

      m_iCount [i] = new Int64 *[MAX_NUM_SAO_TYPE]; 
      m_iOffset[i] = new Int64 *[MAX_NUM_SAO_TYPE]; 
      m_iOffsetOrg[i] = new Int64 *[MAX_NUM_SAO_TYPE]; 

      for (Int j=0;j<MAX_NUM_SAO_TYPE;j++)
      {
        m_iCount [i][j] = new Int64 [MAX_NUM_QAO_CLASS]; 
        m_iOffset[i][j] = new Int64 [MAX_NUM_QAO_CLASS]; 
        m_iOffsetOrg[i][j]=  new Int64 [MAX_NUM_QAO_CLASS]; 
      }
    }
    {
      Int iMaxDepth = 4;
      m_pppcRDSbacCoder = new TEncSbac** [iMaxDepth+1];
      m_pppcBinCoderCABAC = new TEncBinCABAC** [iMaxDepth+1];

      for ( Int iDepth = 0; iDepth < iMaxDepth+1; iDepth++ )
      {
        m_pppcRDSbacCoder[iDepth] = new TEncSbac* [CI_NUM];
        m_pppcBinCoderCABAC[iDepth] = new TEncBinCABAC* [CI_NUM];

        for (Int iCIIdx = 0; iCIIdx < CI_NUM; iCIIdx ++ )
        {
          m_pppcRDSbacCoder[iDepth][iCIIdx] = new TEncSbac;
          m_pppcBinCoderCABAC [iDepth][iCIIdx] = new TEncBinCABAC;
          m_pppcRDSbacCoder   [iDepth][iCIIdx]->init( m_pppcBinCoderCABAC [iDepth][iCIIdx] );
        }
      }
    }
}

/** start Sao Encoder.
 * \param pcPic, pcEntropyCoder, pppcRDSbacCoder, pcRDGoOnSbacCoder 
 */
Void TEncSampleAdaptiveOffset::startSaoEnc( TComPic* pcPic, TEncEntropy* pcEntropyCoder, TEncSbac*** pppcRDSbacCoder, TEncSbac* pcRDGoOnSbacCoder)
{
  if( pcRDGoOnSbacCoder )
    m_bUseSBACRD = true;
  else
    m_bUseSBACRD = false;

  m_pcPic = pcPic;
  m_pcEntropyCoder = pcEntropyCoder;

  m_pcRDGoOnSbacCoder = pcRDGoOnSbacCoder;
  m_pcEntropyCoder->resetEntropy();
  m_pcEntropyCoder->resetBits();

  if( m_bUseSBACRD )
  {
    m_pcRDGoOnSbacCoder->store( m_pppcRDSbacCoder[0][CI_NEXT_BEST]);
    m_pppcRDSbacCoder[0][CI_CURR_BEST]->load( m_pppcRDSbacCoder[0][CI_NEXT_BEST]);
  }

  m_bSaoFlag = 0;

#if MTK_SAO_CHROMA
  m_bSaoFlagCb = 0;
  m_bSaoFlagCr = 0;
#endif

  for (Int i=0;i<m_iNumTotalParts;i++)
  {
    m_dCostPartBest[i] = MAX_DOUBLE;
    m_iTypePartBest[i] = -1;
    m_iDistOrg[i] = 0;
    for (Int j=0;j<MAX_NUM_SAO_TYPE;j++)
    {
      m_iDist[i][j] = 0;
      m_iRate[i][j] = 0;
      m_dCost[i][j] = 0;
      for (Int k=0;k<MAX_NUM_QAO_CLASS;k++)
      {
        m_iCount [i][j][k] = 0;
        m_iOffset[i][j][k] = 0;
        m_iOffsetOrg[i][j][k] = 0;
      }  
    }
  }

  for(Int i=0; i< m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; i++)
  {
    m_psQAOPart[i].bEnableFlag   =  0;
    m_psQAOPart[i].iBestType     = -1;
    m_psQAOPart[i].iLength       =  0;
    m_psQAOPart[i].bSplit        =  false; 
    m_psQAOPart[i].bProcessed    = false;
    m_psQAOPart[i].dMinCost      = MAX_DOUBLE;
    m_psQAOPart[i].iMinDist      = MAX_INT;
    m_psQAOPart[i].iMinRate      = MAX_INT;

    for (Int j=0;j<MAX_NUM_QAO_CLASS;j++)
    {
      m_psQAOPart[i].iOffset[j] = 0;
    }
  }

  for(Int i=0; i< m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; i++)
  {
    m_psQAOPart[i].bEnableFlag   =  0;
    m_psQAOPart[i].iBestType     = -1;
    m_psQAOPart[i].iLength       =  0;
    for (Int j=0;j<MAX_NUM_QAO_CLASS;j++)
    {
      m_psQAOPart[i].iOffset[j] = 0;
    }
  }
#if MTK_SAO_CHROMA
  for(Int i=0; i< m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; i++)
  {
    m_psQAOPartCb[i].bEnableFlag   =  0;
    m_psQAOPartCb[i].iBestType     = -1;
    m_psQAOPartCb[i].iLength       =  0;
    m_psQAOPartCb[i].bSplit        =  false; 
    m_psQAOPartCb[i].bProcessed    = false;
    m_psQAOPartCb[i].dMinCost      = MAX_DOUBLE;
    m_psQAOPartCb[i].iMinDist      = MAX_INT;
    m_psQAOPartCb[i].iMinRate      = MAX_INT;

    for (Int j=0;j<MAX_NUM_QAO_CLASS;j++)
    {
      m_psQAOPartCb[i].iOffset[j] = 0;
    }
  }

  for(Int i=0; i< m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; i++)
  {
    m_psQAOPartCb[i].bEnableFlag   =  0;
    m_psQAOPartCb[i].iBestType     = -1;
    m_psQAOPartCb[i].iLength       =  0;
    for (Int j=0;j<MAX_NUM_QAO_CLASS;j++)
    {
      m_psQAOPartCb[i].iOffset[j] = 0;
    }
  }

  for(Int i=0; i< m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; i++)
  {
    m_psQAOPartCr[i].bEnableFlag   =  0;
    m_psQAOPartCr[i].iBestType     = -1;
    m_psQAOPartCr[i].iLength       =  0;
    m_psQAOPartCr[i].bSplit        =  false; 
    m_psQAOPartCr[i].bProcessed    = false;
    m_psQAOPartCr[i].dMinCost      = MAX_DOUBLE;
    m_psQAOPartCr[i].iMinDist      = MAX_INT;
    m_psQAOPartCr[i].iMinRate      = MAX_INT;

    for (Int j=0;j<MAX_NUM_QAO_CLASS;j++)
    {
      m_psQAOPartCr[i].iOffset[j] = 0;
    }
  }

  for(Int i=0; i< m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; i++)
  {
    m_psQAOPartCr[i].bEnableFlag   =  0;
    m_psQAOPartCr[i].iBestType     = -1;
    m_psQAOPartCr[i].iLength       =  0;
    for (Int j=0;j<MAX_NUM_QAO_CLASS;j++)
    {
      m_psQAOPartCr[i].iOffset[j] = 0;
    }
  }
#endif

}

/** end Sao Encoder.
 */
Void TEncSampleAdaptiveOffset::endSaoEnc()
{
  m_pcPic = NULL;
  m_pcEntropyCoder = NULL;
}

inline int xSign(int x)
{
  return ((x >> 31) | ((int)( (((unsigned int) -x)) >> 31)));
}

#if SAO_FGS_MNIF
/** calculate SAO statistics for current LCU with slice granularity
 * \param iAddr, iPartIdx
 */
Void TEncSampleAdaptiveOffset::calcAoStatsCuMap(Int iAddr, Int iPartIdx, Int iYCbCr)
#if MTK_SAO_REMOVE_SKIP
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  TComSPS *pTmpSPS =  m_pcPic->getSlice(0)->getSPS();

  Pel* pOrg      ;
  Pel* pRec      ;
  Pel* pMap      ;    

  Int iStride    ;
  Int iLcuWidth  = pTmpSPS->getMaxCUHeight();
  Int iLcuHeight = pTmpSPS->getMaxCUWidth();
  Int iPicWidth  = pTmpSPS->getWidth();
  Int iPicHeight = pTmpSPS->getHeight();
  UInt uiLPelX   = pTmpCu->getCUPelX();
  UInt uiTPelY   = pTmpCu->getCUPelY();
  UInt uiRPelX;//   = uiLPelX + iLcuWidth;
  UInt uiBPelY;//   = uiTPelY + iLcuHeight;
  Int64* iStats ;
  Int64* iCount ;
  Int iClassIdx;
  Int iPicWidthTmp;
  Int iPicHeightTmp;
  Int iStartX;
  Int iStartY;
  Int iEndX;
  Int iEndY;

  if (iYCbCr == 1 )
  {
    iPicWidthTmp  = iPicWidth    >> 1;
    iPicHeightTmp = iPicHeight   >> 1;
    iLcuWidth     = iLcuWidth    >> 1;
    iLcuHeight    = iLcuHeight   >> 1;
    uiLPelX       = uiLPelX      >> 1;
    uiTPelY       = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
    pMap       =  m_pcPicYuvMap->getCbAddr(iAddr);    
    iStride    = m_pcPic->getCStride();
  }
  else if (iYCbCr == 2)
  {
    iPicWidthTmp  = iPicWidth  >> 1;
    iPicHeightTmp = iPicHeight >> 1;
    iLcuWidth    = iLcuWidth    >> 1;
    iLcuHeight   = iLcuHeight   >> 1;
    uiLPelX      = uiLPelX      >> 1;
    uiTPelY      = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
    pMap       =  m_pcPicYuvMap->getCrAddr(iAddr);    
    iStride    = m_pcPic->getCStride();
  }
  else
  {
    iPicWidthTmp  = iPicWidth  ;
    iPicHeightTmp = iPicHeight ;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidth ? iPicWidth : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeight? iPicHeight: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
    pMap       = m_pcPicYuvMap->getLumaAddr(iAddr);    
    iStride    = m_pcPic->getStride();
  }


  //   if(m_iAoType == BO_0 || m_iAoType == BO_1)
  {
    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_0];
    iCount = m_iCount    [iPartIdx][SAO_BO_0];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo0[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }

    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_1];
    iCount = m_iCount    [iPartIdx][SAO_BO_1];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo1[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }
  }

  Int iSignLeft;
  Int iSignRight;
  Int iSignDown;
  Int iSignDown1;
  Int iSignDown2;

  UInt uiEdgeType;

  //   if (m_iAoType == EO_0  || m_iAoType == EO_1 || m_iAoType == EO_2 || m_iAoType == EO_3)
  {
    //     if (m_iAoType == EO_0  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_0];
      iCount = m_iCount    [iPartIdx][SAO_EO_0];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft  = xSign(pRec[iStartX] - pRec[iStartX-1]);
        for (x=iStartX; x< iEndX; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;
          if (pMap[x-1] == pMap[x+1])
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        pOrg += iStride;
        pRec += iStride;
        pMap += iStride;
      }
    }

    //     if (m_iAoType == EO_1  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_1];
      iCount = m_iCount    [iPartIdx][SAO_EO_1];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (iStartY == 1)
      {
        pOrg += iStride;
        pRec += iStride;
        pMap += iStride;
      }

      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          iSignDown  =  xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType =  iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;
          if (pMap[x-iStride] == pMap[x+iStride])
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        pOrg += iStride;
        pRec += iStride;
        pMap += iStride;
      }
    }
    //     if (m_iAoType == EO_2  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_2];
      iCount = m_iCount    [iPartIdx][SAO_EO_2];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (iStartY == 1)
      {
        pOrg += iStride;
        pRec += iStride;
        pMap += iStride;
      }
      for (x=iStartX; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride-1]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        iSignDown2 = xSign(pRec[iStride+iStartX] - pRec[iStartX-1]);
        for (x=iStartX; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          if (pMap[x-iStride-1] == pMap[x+iStride+1] )
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        m_iUpBufft[iStartX] = iSignDown2;
        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
        pOrg += iStride;
        pMap += iStride;
      }
    } 
    //     if (m_iAoType == EO_3  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_3];
      iCount = m_iCount    [iPartIdx][SAO_EO_3];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (iStartY == 1)
      {
        pOrg += iStride;
        pRec += iStride;
        pMap += iStride;
      }
      for (x=iStartX-1; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride+1]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        for (x=iStartX; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1]   = -iSignDown1; 
          if ((pMap[x-iStride+1] == pMap[x+iStride-1]) && (pMap[x+iStride-1]==pMap[x]))
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        m_iUpBuff1[iEndX-1] = xSign(pRec[iEndX-1 + iStride] - pRec[iEndX]);

        pRec += iStride;
        pOrg += iStride;
        pMap += iStride;
      } 
    } 
  }
}
#else
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  TComSPS *pTmpSPS =  m_pcPic->getSlice(0)->getSPS();

  Pel* pOrg      ;
  Pel* pRec      ;
  Pel* pMap      ;    

  Int iStride    ;
  Int iLcuWidth  = pTmpSPS->getMaxCUHeight();
  Int iLcuHeight = pTmpSPS->getMaxCUWidth();
  Int iPicWidth  = pTmpSPS->getWidth();
  Int iPicHeight = pTmpSPS->getHeight();
  UInt uiLPelX   = pTmpCu->getCUPelX();
  UInt uiTPelY   = pTmpCu->getCUPelY();
  UInt uiRPelX;//   = uiLPelX + iLcuWidth;
  UInt uiBPelY;//   = uiTPelY + iLcuHeight;
  Int64* iStats ;
  Int64* iCount ;
  Int iClassIdx;
  Int iPicWidthTmp;
  Int iPicHeightTmp;

  if (iYCbCr == 1 )
  {
    iPicWidthTmp  = iPicWidth  >> 1;
    iPicHeightTmp = iPicHeight >> 1;
    iLcuWidth     = iLcuWidth    >> 1;
    iLcuHeight    = iLcuHeight   >> 1;
    uiLPelX       = uiLPelX      >> 1;
    uiTPelY       = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
    pMap       =  m_pcPicYuvMap->getCbAddr(iAddr);    
    iStride    = m_pcPic->getCStride();
  }
  else if (iYCbCr == 2)
  {
    iPicWidthTmp  = iPicWidth  >> 1;
    iPicHeightTmp = iPicHeight >> 1;
    iLcuWidth    = iLcuWidth    >> 1;
    iLcuHeight   = iLcuHeight   >> 1;
    uiLPelX      = uiLPelX      >> 1;
    uiTPelY      = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
    pMap       =  m_pcPicYuvMap->getCrAddr(iAddr);    
    iStride    = m_pcPic->getCStride();
  }
  else
  {

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidth ? iPicWidth : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeight? iPicHeight: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
    pMap       =  m_pcPicYuvMap->getCbAddr(iAddr);    
    iStride    = m_pcPic->getStride();
  }


  //   if(m_iAoType == BO_0 || m_iAoType == BO_1)
  {
    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_0];
    iCount = m_iCount    [iPartIdx][SAO_BO_0];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo0[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }

    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_1];
    iCount = m_iCount    [iPartIdx][SAO_BO_1];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo1[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }
  }

  Int iSignLeft;
  Int iSignRight;
  Int iSignDown;
  Int iSignDown1;
  Int iSignDown2;

  UInt uiEdgeType;

  //   if (m_iAoType == EO_0  || m_iAoType == EO_1 || m_iAoType == EO_2 || m_iAoType == EO_3)
  {
    //     if (m_iAoType == EO_0  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_0];
      iCount = m_iCount    [iPartIdx][SAO_EO_0];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft  = xSign(pRec[1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;
          if (pMap[x-1] == pMap[x+1])
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        pOrg += iStride;
        pRec += iStride;
        pMap += iStride;
      }
    }

    //     if (m_iAoType == EO_1  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_1];
      iCount = m_iCount    [iPartIdx][SAO_EO_1];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      pOrg += iStride;
      pRec += iStride;
      pMap += iStride;

      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride]);
      }

      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {

          iSignDown  =  xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType =  iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;
          if (pMap[x-iStride] == pMap[x+iStride])
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        pOrg += iStride;
        pRec += iStride;
        pMap += iStride;
      }
    }
    //     if (m_iAoType == EO_2  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_2];
      iCount = m_iCount    [iPartIdx][SAO_EO_2];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      pOrg += iStride;
      pRec += iStride;
      pMap += iStride;
      for (x=1; x<iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride-1]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        iSignDown2 = xSign(pRec[iStride+1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          if (pMap[x-iStride-1] == pMap[x+iStride+1] )
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        m_iUpBufft[1] = iSignDown2;
        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
        pOrg += iStride;
        pMap += iStride;
      }
    } 
    //     if (m_iAoType == EO_3  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_3];
      iCount = m_iCount    [iPartIdx][SAO_EO_3];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pMap = getPicYuvAddr(m_pcPicYuvMap, iYCbCr, iAddr);

      pOrg += iStride;
      pRec += iStride;
      pMap += iStride;
      for (x=0; x<iLcuWidth-1; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride+1]);
      }

      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1]   = -iSignDown1; 
          if (pMap[x-iStride+1] == pMap[x+iStride-1])
          {
            iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
            iCount[m_auiEoTable[uiEdgeType]] ++;
          }
        }
        m_iUpBuff1[iLcuWidth-2] = xSign(pRec[iLcuWidth-2 + iStride] - pRec[iLcuWidth-1]);

        pRec += iStride;
        pOrg += iStride;
        pMap += iStride;
      } 
    } 
  }
}
#endif
/** calculate SAO statistics for current LCU
 * \param iAddr, iPartIdx
 */
Void TEncSampleAdaptiveOffset::calcAoStatsCu(Int iAddr, Int iPartIdx, Int iYCbCr)
{
  if (getUseNonCrossAlf())
  {
    calcAoStatsCuMap( iAddr, iPartIdx, iYCbCr);
  }
  else
  {
    calcAoStatsCuOrg( iAddr, iPartIdx, iYCbCr);
  }
}
#endif
/** calculate SAO statistics for current LCU without slice granularity
 * \param iAddr, iPartIdx
 */
#if SAO_FGS_MNIF
Void TEncSampleAdaptiveOffset::calcAoStatsCuOrg(Int iAddr, Int iPartIdx, Int iYCbCr)
#else
Void TEncSampleAdaptiveOffset::calcAoStatsCu(Int iAddr, Int iPartIdx, Int iYCbCr)
#endif
#if MTK_SAO_REMOVE_SKIP
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  TComSPS *pTmpSPS =  m_pcPic->getSlice(0)->getSPS();

  Pel* pOrg      ;
  Pel* pRec      ;
  Int iStride    ;
  Int iLcuWidth  = pTmpSPS->getMaxCUHeight();
  Int iLcuHeight = pTmpSPS->getMaxCUWidth();
  Int iPicWidth  = pTmpSPS->getWidth();
  Int iPicHeight = pTmpSPS->getHeight();
  UInt uiLPelX   = pTmpCu->getCUPelX();
  UInt uiTPelY   = pTmpCu->getCUPelY();
  UInt uiRPelX;//   = uiLPelX + iLcuWidth;
  UInt uiBPelY;//   = uiTPelY + iLcuHeight;
  Int64* iStats ;
  Int64* iCount ;
  Int iClassIdx;
  Int iPicWidthTmp;
  Int iPicHeightTmp;
  Int iStartX;
  Int iStartY;
  Int iEndX;
  Int iEndY;

  if (iYCbCr == 1 )
  {
    iPicWidthTmp  = iPicWidth  >> 1;
    iPicHeightTmp = iPicHeight >> 1;
    iLcuWidth     = iLcuWidth    >> 1;
    iLcuHeight    = iLcuHeight   >> 1;
    uiLPelX       = uiLPelX      >> 1;
    uiTPelY       = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }
  else if (iYCbCr == 2)
  {
    iPicWidthTmp  = iPicWidth  >> 1;
    iPicHeightTmp = iPicHeight >> 1;
    iLcuWidth    = iLcuWidth    >> 1;
    iLcuHeight   = iLcuHeight   >> 1;
    uiLPelX      = uiLPelX      >> 1;
    uiTPelY      = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }
  else
  {
    iPicWidthTmp  = iPicWidth  ;
    iPicHeightTmp = iPicHeight ;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidth ? iPicWidth : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeight? iPicHeight: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
    iStride    = m_pcPic->getStride();
  }


  //   if(m_iAoType == BO_0 || m_iAoType == BO_1)
  {
    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_0];
    iCount = m_iCount    [iPartIdx][SAO_BO_0];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo0[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }

    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_1];
    iCount = m_iCount    [iPartIdx][SAO_BO_1];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo1[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }
  }

  Int iSignLeft;
  Int iSignRight;
  Int iSignDown;
  Int iSignDown1;
  Int iSignDown2;

  UInt uiEdgeType;

  //   if (m_iAoType == EO_0  || m_iAoType == EO_1 || m_iAoType == EO_2 || m_iAoType == EO_3)
  {
    //     if (m_iAoType == EO_0  )
    {

      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_0];
      iCount = m_iCount    [iPartIdx][SAO_EO_0];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft  = xSign(pRec[iStartX] - pRec[iStartX-1]);
        for (x=iStartX; x< iEndX; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;

          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;
        }
        pOrg += iStride;
        pRec += iStride;
      }
    }

    //     if (m_iAoType == EO_1  )
    {

      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_1];
      iCount = m_iCount    [iPartIdx][SAO_EO_1];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;
      if (iStartY == 1)
      {
        pOrg += iStride;
        pRec += iStride;
      }

      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {
          iSignDown  =  xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType =  iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;

          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;
        }
        pOrg += iStride;
        pRec += iStride;
      }
    }
    //     if (m_iAoType == EO_2  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_2];
      iCount = m_iCount    [iPartIdx][SAO_EO_2];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (iStartY == 1)
      {
        pOrg += iStride;
        pRec += iStride;
      }

      for (x=iStartX; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride-1]);
      }
      for (y=iStartY; y<iEndY; y++)
      {
        iSignDown2 = xSign(pRec[iStride+iStartX] - pRec[iStartX-1]);
        for (x=iStartX; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;
        }
        m_iUpBufft[iStartX] = iSignDown2;
        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
        pOrg += iStride;
      }
    } 
    //     if (m_iAoType == EO_3  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_3];
      iCount = m_iCount    [iPartIdx][SAO_EO_3];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

      iStartX = (uiLPelX == 0)            ? 1 : 0;
      iEndX   = (uiRPelX == iPicWidthTmp) ? iLcuWidth-1 : iLcuWidth;

      iStartY = (uiTPelY == 0) ?             1 : 0;
      iEndY   = (uiBPelY == iPicHeightTmp) ? iLcuHeight-1 : iLcuHeight;

      if (iStartY == 1)
      {
        pOrg += iStride;
        pRec += iStride;
      }

      for (x=iStartX-1; x<iEndX; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride+1]);
      }

      for (y=iStartY; y<iEndY; y++)
      {
        for (x=iStartX; x<iEndX; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1]   = -iSignDown1; 
          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;
        }
        m_iUpBuff1[iEndX-1] = xSign(pRec[iEndX-1 + iStride] - pRec[iEndX]);

        pRec += iStride;
        pOrg += iStride;
      } 
    } 
  }
}
#else
{
  Int x,y;
  TComDataCU *pTmpCu = m_pcPic->getCU(iAddr);
  TComSPS *pTmpSPS =  m_pcPic->getSlice(0)->getSPS();

  Pel* pOrg      ;
  Pel* pRec      ;
  Int iStride    ;
  Int iLcuWidth  = pTmpSPS->getMaxCUHeight();
  Int iLcuHeight = pTmpSPS->getMaxCUWidth();
  Int iPicWidth  = pTmpSPS->getWidth();
  Int iPicHeight = pTmpSPS->getHeight();
  UInt uiLPelX   = pTmpCu->getCUPelX();
  UInt uiTPelY   = pTmpCu->getCUPelY();
  UInt uiRPelX;//   = uiLPelX + iLcuWidth;
  UInt uiBPelY;//   = uiTPelY + iLcuHeight;
  Int64* iStats ;
  Int64* iCount ;
  Int iClassIdx;
  Int iPicWidthTmp;
  Int iPicHeightTmp;

  if (iYCbCr == 1 )
  {
    iPicWidthTmp  = iPicWidth  >> 1;
    iPicHeightTmp = iPicHeight >> 1;
    iLcuWidth     = iLcuWidth    >> 1;
    iLcuHeight    = iLcuHeight   >> 1;
    uiLPelX       = uiLPelX      >> 1;
    uiTPelY       = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCbAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }
  else if (iYCbCr == 2)
  {
    iPicWidthTmp  = iPicWidth  >> 1;
    iPicHeightTmp = iPicHeight >> 1;
    iLcuWidth    = iLcuWidth    >> 1;
    iLcuHeight   = iLcuHeight   >> 1;
    uiLPelX      = uiLPelX      >> 1;
    uiTPelY      = uiTPelY      >> 1;

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidthTmp ? iPicWidthTmp : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeightTmp? iPicHeightTmp: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getCrAddr(iAddr);
    iStride    = m_pcPic->getCStride();
  }
  else
  {

    uiRPelX    = uiLPelX + iLcuWidth  ;
    uiBPelY    = uiTPelY + iLcuHeight ;

    uiRPelX    = uiRPelX > iPicWidth ? iPicWidth : uiRPelX;
    uiBPelY    = uiBPelY > iPicHeight? iPicHeight: uiBPelY;

    iLcuWidth  = uiRPelX - uiLPelX;
    iLcuHeight = uiBPelY - uiTPelY;

    pRec       = m_pcPic->getPicYuvRec()->getLumaAddr(iAddr);
    iStride    = m_pcPic->getStride();
  }


  //   if(m_iAoType == BO_0 || m_iAoType == BO_1)
  {
    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_0];
    iCount = m_iCount    [iPartIdx][SAO_BO_0];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo0[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }

    iStats = m_iOffsetOrg[iPartIdx][SAO_BO_1];
    iCount = m_iCount    [iPartIdx][SAO_BO_1];

    pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
    pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);

    for (y=0; y<iLcuHeight; y++)
    {
      for (x=0; x<iLcuWidth; x++)
      {
        iClassIdx =  m_ppLumaTableBo1[pRec[x]];
        if (iClassIdx)
        {
          iStats[iClassIdx] += (pOrg[x] - pRec[x]); 
          iCount[iClassIdx] ++;
        }
      }
      pOrg += iStride;
      pRec += iStride;
    }
  }

  Int iSignLeft;
  Int iSignRight;
  Int iSignDown;
  Int iSignDown1;
  Int iSignDown2;

  UInt uiEdgeType;

  //   if (m_iAoType == EO_0  || m_iAoType == EO_1 || m_iAoType == EO_2 || m_iAoType == EO_3)
  {
    //     if (m_iAoType == EO_0  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_0];
      iCount = m_iCount    [iPartIdx][SAO_EO_0];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      for (y=0; y<iLcuHeight; y++)
      {
        iSignLeft  = xSign(pRec[1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignRight =  xSign(pRec[x] - pRec[x+1]); 
          uiEdgeType =  iSignRight + iSignLeft + 2;
          iSignLeft  = -iSignRight;

          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;
        }
        pOrg += iStride;
        pRec += iStride;
      }
    }

    //     if (m_iAoType == EO_1  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_1];
      iCount = m_iCount    [iPartIdx][SAO_EO_1];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pOrg += iStride;
      pRec += iStride;

      for (x=0; x< iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride]);
      }

      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=0; x<iLcuWidth; x++)
        {

          iSignDown  =  xSign(pRec[x] - pRec[x+iStride]); 
          uiEdgeType =  iSignDown + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x]= -iSignDown;

          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;

        }
        pOrg += iStride;
        pRec += iStride;
      }
    }
    //     if (m_iAoType == EO_2  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_2];
      iCount = m_iCount    [iPartIdx][SAO_EO_2];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pOrg += iStride;
      pRec += iStride;
      for (x=1; x<iLcuWidth; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride-1]);
      }
      for (y=1; y<iLcuHeight-1; y++)
      {
        iSignDown2 = xSign(pRec[iStride+1] - pRec[0]);
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride+1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBufft[x+1] = -iSignDown1; 
          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;
        }
        m_iUpBufft[1] = iSignDown2;
        ipSwap     = m_iUpBuff1;
        m_iUpBuff1 = m_iUpBufft;
        m_iUpBufft = ipSwap;

        pRec += iStride;
        pOrg += iStride;
      }
    } 
    //     if (m_iAoType == EO_3  )
    {
      iStats = m_iOffsetOrg[iPartIdx][SAO_EO_3];
      iCount = m_iCount    [iPartIdx][SAO_EO_3];

      pOrg = getPicYuvAddr(m_pcPic->getPicYuvOrg(), iYCbCr, iAddr);
      pRec = getPicYuvAddr(m_pcPic->getPicYuvRec(), iYCbCr, iAddr);
      pOrg += iStride;
      pRec += iStride;
      for (x=0; x<iLcuWidth-1; x++)
      {
        m_iUpBuff1[x] = xSign(pRec[x] - pRec[x-iStride+1]);
      }

      for (y=1; y<iLcuHeight-1; y++)
      {
        for (x=1; x<iLcuWidth-1; x++)
        {
          iSignDown1      =  xSign(pRec[x] - pRec[x+iStride-1]) ;
          uiEdgeType      =  iSignDown1 + m_iUpBuff1[x] + 2;
          m_iUpBuff1[x-1]   = -iSignDown1; 
          iStats[m_auiEoTable[uiEdgeType]] += (pOrg[x] - pRec[x]);
          iCount[m_auiEoTable[uiEdgeType]] ++;
        }
        m_iUpBuff1[iLcuWidth-2] = xSign(pRec[iLcuWidth-2 + iStride] - pRec[iLcuWidth-1]);

        pRec += iStride;
        pOrg += iStride;
      } 
    } 
  }
}
#endif
/** run get QAO Stats
 * \param pcPicYuvOrg, pcPicYuvRec, pcPicYuvExt
 */
Void TEncSampleAdaptiveOffset::xGetQAOStats(SAOQTPart *psQTPart, Int iYCbCr)
{
  Int iLevelIdx, iPartIdx, iTypeIdx, iClassIdx;
  Int i;
  Int iNumTotalType = MAX_NUM_SAO_TYPE;
  Int LcuIdxX;
  Int LcuIdxY;
  Int iAddr;
  Int iFrameWidthInCU = m_pcPic->getFrameWidthInCU();
  Int iDownPartIdx;
  Int iPartStart;
  Int iPartEnd;
  SAOQTPart*  pOnePart; 

  if (m_uiMaxSplitLevel == 0)
  {
    iPartIdx = 0;
    pOnePart = &(psQTPart[iPartIdx]);
    for (LcuIdxY = pOnePart->StartCUY; LcuIdxY<= pOnePart->EndCUY; LcuIdxY++)
    {
      for (LcuIdxX = pOnePart->StartCUX; LcuIdxX<= pOnePart->EndCUX; LcuIdxX++)
      {
        iAddr = LcuIdxY*iFrameWidthInCU + LcuIdxX;
        calcAoStatsCu(iAddr, iPartIdx, iYCbCr);
      }
    }

  }
  else
  {
    for(iPartIdx=m_aiNumCulPartsLevel[m_uiMaxSplitLevel-1]; iPartIdx<m_aiNumCulPartsLevel[m_uiMaxSplitLevel]; iPartIdx++)
    {
      pOnePart = &(psQTPart[iPartIdx]);
      for (LcuIdxY = pOnePart->StartCUY; LcuIdxY<= pOnePart->EndCUY; LcuIdxY++)
      {
        for (LcuIdxX = pOnePart->StartCUX; LcuIdxX<= pOnePart->EndCUX; LcuIdxX++)
        {
          iAddr = LcuIdxY*iFrameWidthInCU + LcuIdxX;
          calcAoStatsCu(iAddr, iPartIdx, iYCbCr);
        }
      }
    }
    for (iLevelIdx = m_uiMaxSplitLevel-1; iLevelIdx>=0; iLevelIdx-- )
    {
      iPartStart = (iLevelIdx > 0) ? m_aiNumCulPartsLevel[iLevelIdx-1] : 0;
      iPartEnd   = m_aiNumCulPartsLevel[iLevelIdx];

      for(iPartIdx = iPartStart; iPartIdx < iPartEnd; iPartIdx++)
      {
        pOnePart = &(psQTPart[iPartIdx]);
        for (i=0; i< NUM_DOWN_PART; i++)
        {
          iDownPartIdx = pOnePart->DownPartsIdx[i];
          for (iTypeIdx=0; iTypeIdx<iNumTotalType; iTypeIdx++)
          {
            for (iClassIdx=0; iClassIdx<m_iNumClass[iTypeIdx]+1; iClassIdx++)
            {
              m_iOffsetOrg[iPartIdx][iTypeIdx][iClassIdx] += m_iOffsetOrg[iDownPartIdx][iTypeIdx][iClassIdx];
              m_iCount [iPartIdx][iTypeIdx][iClassIdx]    += m_iCount [iDownPartIdx][iTypeIdx][iClassIdx];
            }
          }
        }
      }
    }
  }
}


#if SAO_CHROMA_LAMBDA 
/** Sample adaptive offset Process
 * \param dLambdaLuma
 * \param dLambdaChroma
 */
Void TEncSampleAdaptiveOffset::SAOProcess( Double dLambdaLuma, Double dLambdaChroma)
#else
/** Sample adaptive offset Process
 * \param dLambda
 */
Void TEncSampleAdaptiveOffset::SAOProcess( Double dLambda)
#endif
{

#if MTK_SAO_REMOVE_SKIP
  m_pcPic->getPicYuvRec()->copyToPic( m_pcPicYuvTmp );
#endif

  m_eSliceType           =  m_pcPic->getSlice(0)->getSliceType();
  m_iPicNalReferenceIdc  = (m_pcPic->getSlice(0)->isReferenced() ? 1 :0);

#if SAO_CHROMA_LAMBDA 
  m_dLambdaLuma    = dLambdaLuma;
  m_dLambdaChroma  = dLambdaChroma;
#else
  m_dLambdaLuma    = dLambda;
  m_dLambdaChroma  = dLambda;
#endif

#if SAO_ACCURATE_OFFSET
#if FULL_NBIT
  m_uiAoBitDepth = g_uiBitDepth + (g_uiBitDepth-8) - min((Int)(g_uiBitDepth + (g_uiBitDepth-8)), 10);
#else
  m_uiAoBitDepth = g_uiBitDepth + g_uiBitIncrement - min((Int)(g_uiBitDepth + g_uiBitIncrement), 10);
#endif

#else
#if FULL_NBIT
  if (g_uiBitDepth>9)
#else
  if (g_uiBitIncrement>1)
#endif
  {
    m_uiAoBitDepth = 1;
  }
  else
  {
    m_uiAoBitDepth = 0;
  }
#endif
#if SAO_CLIP_OFFSET
  const Int iOffsetBitRange8Bit = 4;
#if SAO_ACCURATE_OFFSET
  Int iOffsetBitDepth = g_uiBitDepth + g_uiBitIncrement - m_uiAoBitDepth;
#else
  Int iOffsetBitDepth = g_uiBitDepth + m_uiAoBitDepth;
#endif
  Int iOffsetBitRange = iOffsetBitRange8Bit + (iOffsetBitDepth - 8);
  m_iOffsetTh = 1 << (iOffsetBitRange - 1);
#endif

  Double dCostFinal = 0;

#if MTK_SAO_CHROMA
  Double dCostFinalCb = 0;
  Double dCostFinalCr = 0;
  xGetQAOStats(m_psQAOPart, 0);
  xQuadTreeDecisionFunc(m_psQAOPart, 0, dCostFinal, m_uiMaxSplitLevel, m_dLambdaLuma);
#else
  xGetQAOStats(m_psQAOPart, 0);
  xQuadTreeDecisionFunc(m_psQAOPart, 0, dCostFinal, m_uiMaxSplitLevel, m_dLambdaLuma);
#endif
  m_bSaoFlag = dCostFinal < m_iDistOrg[0] ? 1:0;

  if(m_bSaoFlag)
  {
#if MTK_SAO_CHROMA
    xProcessQuadTreeAo(m_psQAOPart, 0, 0);

    for (Int i=0;i<m_iNumTotalParts;i++)
    {
      m_dCostPartBest[i] = MAX_DOUBLE;
      m_iTypePartBest[i] = -1;
      m_iDistOrg[i] = 0;
      for (Int j=0;j<MAX_NUM_SAO_TYPE;j++)
      {
        m_iDist[i][j] = 0;
        m_iRate[i][j] = 0;
        m_dCost[i][j] = 0;
        for (Int k=0;k<MAX_NUM_QAO_CLASS;k++)
        {
          m_iCount [i][j][k] = 0;
          m_iOffset[i][j][k] = 0;
          m_iOffsetOrg[i][j][k] = 0;
        }  
      }
    }
    xGetQAOStats(m_psQAOPartCb, 1);
    xQuadTreeDecisionFunc(m_psQAOPartCb, 0, dCostFinalCb, m_uiMaxSplitLevel, m_dLambdaChroma);

    if (dCostFinalCb<0)
    {
      m_bSaoFlagCb = 1;
      xProcessQuadTreeAo(m_psQAOPartCb, 0, 1);
    }
    else
    {
      m_bSaoFlagCb = 0;
    }

    for (Int i=0;i<m_iNumTotalParts;i++)
    {
      m_dCostPartBest[i] = MAX_DOUBLE;
      m_iTypePartBest[i] = -1;
      m_iDistOrg[i] = 0;
      for (Int j=0;j<MAX_NUM_SAO_TYPE;j++)
      {
        m_iDist[i][j] = 0;
        m_iRate[i][j] = 0;
        m_dCost[i][j] = 0;
        for (Int k=0;k<MAX_NUM_QAO_CLASS;k++)
        {
          m_iCount [i][j][k] = 0;
          m_iOffset[i][j][k] = 0;
          m_iOffsetOrg[i][j][k] = 0;
        }  
      }
    }

    xGetQAOStats(m_psQAOPartCr, 2);
    xQuadTreeDecisionFunc(m_psQAOPartCr, 0, dCostFinalCr, m_uiMaxSplitLevel, m_dLambdaChroma);
    if (dCostFinalCr<0)
    {
      m_bSaoFlagCr = 1;
      xProcessQuadTreeAo(m_psQAOPartCr, 0, 2);
    }
    else
    {
      m_bSaoFlagCr = 0;
    }

#else
    xProcessQuadTreeAo(m_psQAOPart, 0, 0);
#endif
  }

}
#endif
//! \}
