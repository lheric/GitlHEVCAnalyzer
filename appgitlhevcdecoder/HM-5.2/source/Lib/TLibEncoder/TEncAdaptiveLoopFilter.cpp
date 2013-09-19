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

// ====================================================================================================================
// Constructor / destructor
// ====================================================================================================================

TEncAdaptiveLoopFilter::TEncAdaptiveLoopFilter()
{
  m_ppdAlfCorr = NULL;
  m_ppdAlfCorrCb = NULL;
  m_ppdAlfCorrCr = NULL;
  m_pdDoubleAlfCoeff = NULL;
  m_pcEntropyCoder = NULL;
  m_pcBestAlfParam = NULL;
  m_pcTempAlfParam = NULL;
  m_pcPicYuvBest = NULL;
  m_pcPicYuvTmp = NULL;
  pcAlfParamShape0 = NULL;
  pcAlfParamShape1 = NULL;
  pcPicYuvRecShape0 = NULL;
  pcPicYuvRecShape1 = NULL;
  m_pcSliceYuvTmp = NULL;

  m_iALFMaxNumberFilters = NO_FILTERS;

  m_bAlfCUCtrlEnabled = false;
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

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
      m_mergeTableSavedMethods[i] = new Int*[iNumOfBuffer];
      m_aiFilterCoeffSavedMethods[i] = new Int**[iNumOfBuffer];
      for(Int j=0; j< iNumOfBuffer; j++)
      {
        m_mergeTableSavedMethods[i][j] = new Int[NO_VAR_BINS];
        m_aiFilterCoeffSavedMethods[i][j] = new Int*[NO_VAR_BINS];
        for(Int k=0; k< NO_VAR_BINS; k++)
        {
          m_aiFilterCoeffSavedMethods[i][j][k] = new Int[ALF_MAX_NUM_COEF];
        }
      }
      m_iPreviousFilterShapeMethods[i] = new Int[iNumOfBuffer];
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
        delete[] m_mergeTableSavedMethods[i][j];
      }
      delete[] m_aiFilterCoeffSavedMethods[i];
      delete[] m_iPreviousFilterShapeMethods[i];
      delete[] m_mergeTableSavedMethods[i];

    }

  }

}

/**
 \param pcPic           picture (TComPic) pointer
 \param pcEntropyCoder  entropy coder class
 */
Void TEncAdaptiveLoopFilter::startALFEnc( TComPic* pcPic, TEncEntropy* pcEntropyCoder )
{
  m_pcEntropyCoder = pcEntropyCoder;
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
  pcPicYuvRecShape0 = new TComPicYuv();
  pcPicYuvRecShape0->createLuma(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
 
  pcPicYuvRecShape1 = new TComPicYuv();
  pcPicYuvRecShape1->createLuma(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  
  pcAlfParamShape0 = new ALFParam;
  pcAlfParamShape1 = new ALFParam;

  allocALFParam(pcAlfParamShape0);  
  allocALFParam(pcAlfParamShape1);

  // init qc_filter
  initMatrix4D_double(&m_EGlobalSym, NUM_ALF_FILTER_SHAPE+1,  NO_VAR_BINS, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);
  initMatrix3D_double(&m_yGlobalSym, NUM_ALF_FILTER_SHAPE+1, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); 
  initMatrix_int(&m_filterCoeffSymQuant, NO_VAR_BINS, ALF_MAX_NUM_COEF); 
  m_pixAcc = (double *) calloc(NO_VAR_BINS, sizeof(double));
  initMatrix_Pel(&m_maskImg, m_img_height, m_img_width);
  initMatrix_double(&m_E_temp, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);//
  m_y_temp = (double *) calloc(MAX_SQR_FILT_LENGTH, sizeof(double));//
  initMatrix3D_double(&m_E_merged, NO_VAR_BINS, MAX_SQR_FILT_LENGTH, MAX_SQR_FILT_LENGTH);//
  initMatrix_double(&m_y_merged, NO_VAR_BINS, MAX_SQR_FILT_LENGTH); //
  m_pixAcc_merged = (double *) calloc(NO_VAR_BINS, sizeof(double));//
  m_filterCoeffQuantMod = (int *) calloc(ALF_MAX_NUM_COEF, sizeof(int));//
  m_filterCoeff = (double *) calloc(ALF_MAX_NUM_COEF, sizeof(double));//
  m_filterCoeffQuant = (int *) calloc(ALF_MAX_NUM_COEF, sizeof(int));//
  initMatrix_int(&m_diffFilterCoeffQuant, NO_VAR_BINS, ALF_MAX_NUM_COEF);//
  initMatrix_int(&m_FilterCoeffQuantTemp, NO_VAR_BINS, ALF_MAX_NUM_COEF);//
  m_tempALFp = new ALFParam;
  allocALFParam(m_tempALFp);
  if( m_bUseNonCrossALF )
  {
    m_pcSliceYuvTmp = new TComPicYuv();
    m_pcSliceYuvTmp->create(iWidth, iHeight, g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth);
  }

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
  // delete qc filters
  destroyMatrix4D_double(m_EGlobalSym, NUM_ALF_FILTER_SHAPE+1,  NO_VAR_BINS);
  destroyMatrix3D_double(m_yGlobalSym, NUM_ALF_FILTER_SHAPE+1);
  destroyMatrix_int(m_filterCoeffSymQuant);
  
  free(m_pixAcc);
  destroyMatrix_Pel(m_maskImg);
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

  if(m_bUseNonCrossALF)
  {
    m_pcSliceYuvTmp->destroy();
    delete m_pcSliceYuvTmp;
    m_pcSliceYuvTmp = NULL;
  }
}

#if ALF_CHROMA_LAMBDA  
/**
 \param pcAlfParam           ALF parameter
 \param [out] pvAlfCtrlParam ALF CU control parameters container for slices
 \param dLambdaLuma          luma lambda value for RD cost computation
 \param dLambdaChroma        chroma lambda value for RD cost computation
 \retval ruiDist             distortion
 \retval ruiBits             required bits
 \retval ruiMaxAlfCtrlDepth  optimal partition depth
 */
Void TEncAdaptiveLoopFilter::ALFProcess( ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>* pvAlfCtrlParam, Double dLambdaLuma, Double dLambdaChroma, UInt64& ruiDist, UInt64& ruiBits)

#else
/**
 \param pcAlfParam           ALF parameter
 \param dLambda              lambda value for RD cost computation
 \retval ruiDist             distortion
 \retval ruiBits             required bits
 \retval ruiMaxAlfCtrlDepth  optimal partition depth
 */
Void TEncAdaptiveLoopFilter::ALFProcess( ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>* pvAlfCtrlParam, Double dLambda, UInt64& ruiDist, UInt64& ruiBits)

#endif
{
  
  // set lambda
#if ALF_CHROMA_LAMBDA  
  m_dLambdaLuma   = dLambdaLuma;
  m_dLambdaChroma = dLambdaChroma;
#else
  m_dLambdaLuma   = dLambda;
  m_dLambdaChroma = dLambda;
#endif

  m_lcuHeight = m_pcPic->getSlice(0)->getSPS()->getMaxCUHeight();
  m_lineIdxPadBot = m_lcuHeight - 4 - 4; // DFRegion, Vertical Taps
  m_lineIdxPadTop = m_lcuHeight - 4; // DFRegion

  m_lcuHeightChroma = m_lcuHeight>>1;
  m_lineIdxPadBotChroma = m_lcuHeightChroma - 2 - 4; // DFRegion, Vertical Taps
  m_lineIdxPadTopChroma = m_lcuHeightChroma - 2 ; // DFRegion

  TComPicYuv* pcPicOrg = m_pcPic->getPicYuvOrg();
  
  // extend image for filtering
  TComPicYuv* pcPicYuvRec    = m_pcPic->getPicYuvRec();
  TComPicYuv* pcPicYuvExtRec = m_pcTempPicYuv;
  
  pcPicYuvRec->copyToPic(pcPicYuvExtRec);
  if(!m_bUseNonCrossALF)
  {
  pcPicYuvExtRec->setBorderExtension( false );
  pcPicYuvExtRec->extendPicBorder   ();
  }
 
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
  // initialize temp_alfps
  m_pcTempAlfParam->alf_flag        = 1;
  m_pcTempAlfParam->chroma_idc      = 0;

  m_bAlfCUCtrlEnabled = (pvAlfCtrlParam != NULL)?true:false;
  if(m_bAlfCUCtrlEnabled)
  {
    m_vBestAlfCUCtrlParam.resize(m_uiNumSlicesInPic);
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      m_vBestAlfCUCtrlParam[s].cu_control_flag = 0;
    }
  }
  else
  {
    m_vBestAlfCUCtrlParam.clear();
  }

  setALFEncodingParam(m_pcPic);

  // adaptive in-loop wiener filtering
  xEncALFLuma( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );

  // cu-based filter on/off control
  xCUAdaptiveControl_qc( pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, uiMinRate, uiMinDist, dMinCost );
  
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

    uiMinRate = uiOrigRate;
    uiMinDist = uiOrigDist;
    
    m_pcEntropyCoder->setAlfCtrl(false);
    if(m_bAlfCUCtrlEnabled)
    {
      for(Int s=0; s< m_uiNumSlicesInPic; s++)
      {
        m_vBestAlfCUCtrlParam[s].cu_control_flag = 0;
      }
    }
    pcPicYuvExtRec->copyToPicLuma(pcPicYuvRec);
    
    ruiBits = uiOrigRate;
    ruiDist = uiOrigDist;
  }
  // if ALF works
  if( m_pcBestAlfParam->alf_flag )
  {
    // do additional ALF process for chroma
    xFilterTapDecisionChroma( uiMinRate, pcPicOrg, pcPicYuvExtRec, pcPicYuvRec, ruiDist, ruiBits );
  }
  
  // copy to best storage
  copyALFParam(pcAlfParam, m_pcBestAlfParam);
  
  if(m_bAlfCUCtrlEnabled)
  {
    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      (*pvAlfCtrlParam)[s]= m_vBestAlfCUCtrlParam[s];
    }
  }
}

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

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

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
Void TEncAdaptiveLoopFilter::xEncodeCUAlfCtrlFlags(std::vector<AlfCUCtrlInfo> &vAlfCUCtrlParam)
{
  for(Int s=0; s< m_uiNumSlicesInPic; s++)
  {
    if(!m_pcPic->getValidSlice(s))
    {
      continue;
    }

    AlfCUCtrlInfo& rCUCtrlInfo = vAlfCUCtrlParam[s];
    if(rCUCtrlInfo.cu_control_flag == 1)
    {
      for(Int i=0; i< (Int)rCUCtrlInfo.alf_cu_flag.size(); i++)
      {
        m_pcEntropyCoder->encodeAlfCtrlFlag(rCUCtrlInfo.alf_cu_flag[i]);
      }
    }
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

Void TEncAdaptiveLoopFilter::xCalcCorrelationFunc(Int ypos, Int xpos, Pel* pImgOrg, Pel* pImgPad, Int filtNo, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride, Bool bSymmCopyBlockMatrix)
{
  Int     yposEnd = ypos + iHeight -1;
  Int     xposEnd = xpos + iWidth  -1;
  Int     N       = m_sqrFiltLengthTab[filtNo];

  Int imgHeightChroma = m_img_height>>1;
  Int yLineInLCU;
  Int paddingline ;

  Int ELocal[ALF_MAX_NUM_COEF];
  Pel *pImgPad1, *pImgPad2, *pImgPad3, *pImgPad4;
  Int i, j, k, l;
  Int yLocal;

  pImgPad += (ypos*iCmpStride);
  pImgOrg += (ypos*iOrgStride);

  switch(filtNo)
  {
  case ALF_STAR5x5:
    {
      for (i= ypos; i<= yposEnd; i++)
      {
        yLineInLCU = i % m_lcuHeightChroma;

        if (yLineInLCU < m_lineIdxPadBotChroma || i-yLineInLCU+m_lcuHeightChroma >= imgHeightChroma )
        {
          pImgPad1 = pImgPad +   iCmpStride;
          pImgPad2 = pImgPad -   iCmpStride;
          pImgPad3 = pImgPad + 2*iCmpStride;
          pImgPad4 = pImgPad - 2*iCmpStride;
        }
        else if (yLineInLCU < m_lineIdxPadTopChroma)
        {
          paddingline = - yLineInLCU + m_lineIdxPadTopChroma - 1;
          pImgPad1 = pImgPad + min(paddingline, 1)*iCmpStride;
          pImgPad2 = pImgPad -   iCmpStride;
          pImgPad3 = pImgPad + min(paddingline, 2)*iCmpStride;
          pImgPad4 = pImgPad - 2*iCmpStride;
        }
        else
        {
          paddingline = yLineInLCU - m_lineIdxPadTopChroma ;
          pImgPad1 = pImgPad +   iCmpStride;
          pImgPad2 = pImgPad - min(paddingline, 1)*iCmpStride;
          pImgPad3 = pImgPad + 2*iCmpStride;
          pImgPad4 = pImgPad - min(paddingline, 2)*iCmpStride;
        }

        if ( (yLineInLCU == m_lineIdxPadTopChroma || yLineInLCU == m_lineIdxPadTopChroma-1) && i-yLineInLCU+m_lcuHeightChroma < imgHeightChroma ) 
        {
          pImgPad+= iCmpStride;
          pImgOrg+= iOrgStride;
          continue;
        }
        else
        {
        for (j= xpos; j<= xposEnd; j++)
        {
          memset(ELocal, 0, N*sizeof(Int));

          ELocal[0] = (pImgPad3[j+2] + pImgPad4[j-2]);
          ELocal[1] = (pImgPad3[j  ] + pImgPad4[j  ]);
          ELocal[2] = (pImgPad3[j-2] + pImgPad4[j+2]);

          ELocal[3] = (pImgPad1[j+1] + pImgPad2[j-1]);
          ELocal[4] = (pImgPad1[j  ] + pImgPad2[j  ]);
          ELocal[5] = (pImgPad1[j-1] + pImgPad2[j+1]);

          ELocal[6] = (pImgPad[j+2] + pImgPad[j-2]);
          ELocal[7] = (pImgPad[j+1] + pImgPad[j-1]);
          ELocal[8] = (pImgPad[j  ]);

          yLocal= (Int)pImgOrg[j];

          for(k=0; k<N; k++)
          {
            m_ppdAlfCorr[k][k] += ELocal[k]*ELocal[k];
            for(l=k+1; l<N; l++)
            {
              m_ppdAlfCorr[k][l] += ELocal[k]*ELocal[l];
            }

            m_ppdAlfCorr[k][N] += yLocal*ELocal[k];
          }
        }
        pImgPad+= iCmpStride;
        pImgOrg+= iOrgStride;
      }

      }
    }
    break;

  case ALF_CROSS9x9:
    {
      Pel *pImgPad5, *pImgPad6, *pImgPad7, *pImgPad8;

      for (i= ypos; i<= yposEnd; i++)
      {
        yLineInLCU = i % m_lcuHeightChroma;

        if (yLineInLCU<2 && i> 2)
        {
          paddingline = yLineInLCU + 2 ;
          pImgPad1 = pImgPad +   iCmpStride;
          pImgPad2 = pImgPad -   iCmpStride;
          pImgPad3 = pImgPad + 2*iCmpStride;
          pImgPad4 = pImgPad - 2*iCmpStride;
          pImgPad5 = (paddingline < 3) ? pImgPad : pImgPad + 3*iCmpStride;
          pImgPad6 = (paddingline < 3) ? pImgPad : pImgPad - min(paddingline, 3)*iCmpStride;;
          pImgPad7 = (paddingline < 4) ? pImgPad : pImgPad + 4*iCmpStride;
          pImgPad8 = (paddingline < 4) ? pImgPad : pImgPad - min(paddingline, 4)*iCmpStride;;      
        }
        else if (yLineInLCU < m_lineIdxPadBotChroma || i-yLineInLCU+m_lcuHeightChroma >= imgHeightChroma )
        {
          pImgPad1 = pImgPad +   iCmpStride;
          pImgPad2 = pImgPad -   iCmpStride;
          pImgPad3 = pImgPad + 2*iCmpStride;
          pImgPad4 = pImgPad - 2*iCmpStride;
          pImgPad5 = pImgPad + 3*iCmpStride;
          pImgPad6 = pImgPad - 3*iCmpStride;
          pImgPad7 = pImgPad + 4*iCmpStride;
          pImgPad8 = pImgPad - 4*iCmpStride;
        }
        else if (yLineInLCU < m_lineIdxPadTopChroma)
        {
          paddingline = - yLineInLCU + m_lineIdxPadTopChroma - 1;
          pImgPad1 = (paddingline < 1) ? pImgPad : pImgPad + min(paddingline, 1)*iCmpStride;
          pImgPad2 = (paddingline < 1) ? pImgPad : pImgPad -   iCmpStride;
          pImgPad3 = (paddingline < 2) ? pImgPad : pImgPad + min(paddingline, 2)*iCmpStride;
          pImgPad4 = (paddingline < 2) ? pImgPad : pImgPad - 2*iCmpStride;
          pImgPad5 = (paddingline < 3) ? pImgPad : pImgPad + min(paddingline, 3)*iCmpStride;
          pImgPad6 = (paddingline < 3) ? pImgPad : pImgPad - 3*iCmpStride;
          pImgPad7 = (paddingline < 4) ? pImgPad : pImgPad + min(paddingline, 4)*iCmpStride;
          pImgPad8 = (paddingline < 4) ? pImgPad : pImgPad - 4*iCmpStride;
        }
        else
        {
          paddingline = yLineInLCU - m_lineIdxPadTopChroma ;
          pImgPad1 = (paddingline < 1) ? pImgPad : pImgPad +   iCmpStride;
          pImgPad2 = (paddingline < 1) ? pImgPad : pImgPad - min(paddingline, 1)*iCmpStride;
          pImgPad3 = (paddingline < 2) ? pImgPad : pImgPad + 2*iCmpStride;
          pImgPad4 = (paddingline < 2) ? pImgPad : pImgPad - min(paddingline, 2)*iCmpStride;
          pImgPad5 = (paddingline < 3) ? pImgPad : pImgPad + 3*iCmpStride;
          pImgPad6 = (paddingline < 3) ? pImgPad : pImgPad - min(paddingline, 3)*iCmpStride;
          pImgPad7 = (paddingline < 4) ? pImgPad : pImgPad + 4*iCmpStride;
          pImgPad8 = (paddingline < 4) ? pImgPad : pImgPad - min(paddingline, 4)*iCmpStride;
        }

        for (j= xpos; j<= xposEnd; j++)
        {
          memset(ELocal, 0, N*sizeof(Int));

          ELocal[0] = (pImgPad7[j] + pImgPad8[j]);

          ELocal[1] = (pImgPad5[j] + pImgPad6[j]);

          ELocal[2] = (pImgPad3[j] + pImgPad4[j]);

          ELocal[3] = (pImgPad1[j] + pImgPad2[j]);

          ELocal[4] = (pImgPad[j+4] + pImgPad[j-4]);
          ELocal[5] = (pImgPad[j+3] + pImgPad[j-3]);
          ELocal[6] = (pImgPad[j+2] + pImgPad[j-2]);
          ELocal[7] = (pImgPad[j+1] + pImgPad[j-1]);
          ELocal[8] = (pImgPad[j  ] );

          yLocal= (Int)pImgOrg[j];

          for(k=0; k<N; k++)
          {
            m_ppdAlfCorr[k][k] += ELocal[k]*ELocal[k];
            for(l=k+1; l<N; l++)
            {
              m_ppdAlfCorr[k][l] += ELocal[k]*ELocal[l];
            }

            m_ppdAlfCorr[k][N] += yLocal*ELocal[k];
          }
        }
        pImgPad+= iCmpStride;
        pImgOrg+= iOrgStride;
      }

    }
    break;
  default:
    {
      printf("Not a supported filter shape\n");
      assert(0);
      exit(1);
    }
  }

  if(bSymmCopyBlockMatrix)
  {
    for(j=0; j<N-1; j++)
    {
      for(i=j+1; i<N; i++)
      {
        m_ppdAlfCorr[i][j] = m_ppdAlfCorr[j][i];
      }
    }
  }
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
    {
      return 1;
    }
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
    {
      t -= a[i][j] * a[j][N];
    }
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
  if ( upper < j ) 
  {
    xFilterCoefQuickSort(coef_data, coef_num, upper, j);
  }
  if ( i < lower ) 
  {
    xFilterCoefQuickSort(coef_data, coef_num, i, lower);
  }
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

  N = m_sqrFiltLengthTab[tap];
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
  
  dh = new Double[N];
  nc = new Int[N];
  
  max_value =   (1<<(1+ALF_NUM_BIT_SHIFT))-1;
  min_value = 0-(1<<(1+ALF_NUM_BIT_SHIFT));
  
  dbl_total_gain=0.0;
  q_total_gain=0;
  for(i=0; i<N; i++)
  {
    if(h[i]>=0.0)
    {
      qh[i] =  (Int)( h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
    }
    else
    {
      qh[i] = -(Int)(-h[i]*(1<<ALF_NUM_BIT_SHIFT)+0.5);
    }
    
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
        {
          qh[N-1]--;
        }
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
        {
          qh[N-1]++;
        }
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

  checkFilterCoeffValue(qh, N, true);

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
  qh[N-1]  = 1<<ALF_NUM_BIT_SHIFT;
}

/** Calculate RD cost
 * \param [in] pAlfParam ALF parameters
 * \param [out] ruiRate coding bits
 * \param [in] uiDist distortion
 * \param [out] rdCost rate-distortion cost
 * \param [in] pvAlfCUCtrlParam ALF CU control parameters
 */
Void TEncAdaptiveLoopFilter::xCalcRDCost(ALFParam* pAlfParam, UInt64& ruiRate, UInt64 uiDist, Double& rdCost, std::vector<AlfCUCtrlInfo>* pvAlfCUCtrlParam)
{
  if(pAlfParam != NULL)
  {
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);

    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();

    if(pvAlfCUCtrlParam != NULL)
    {
      for(UInt s=0; s< m_uiNumSlicesInPic; s++)
      {
        if(!m_pcPic->getValidSlice(s))
        {
          continue;
        }
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeAlfCtrlParam( (*pvAlfCUCtrlParam)[s], m_uiNumCUsInFrame);
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
      }
    }
    else
    {
      ruiRate += m_uiNumSlicesInPic;
    }
  }
  else
  {
    ruiRate = 1;
  }
  
  rdCost      = (Double)(ruiRate) * m_dLambdaLuma + (Double)(uiDist);
}

/** Calculate RD cost
 * \param [in] pcPicOrg original picture buffer
 * \param [in] pcPicCmp compared picture buffer
 * \param [in] pAlfParam ALF parameters
 * \param [out] ruiRate coding bits
 * \param [out] ruiDist distortion
 * \param [out] rdCost rate-distortion cost
 * \param [in] pvAlfCUCtrlParam ALF CU control parameters
 */
Void TEncAdaptiveLoopFilter::xCalcRDCost(TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost, std::vector<AlfCUCtrlInfo>* pvAlfCUCtrlParam)
{
  if(pAlfParam != NULL)
  {
    m_pcEntropyCoder->resetEntropy();
    m_pcEntropyCoder->resetBits();
    m_pcEntropyCoder->encodeAlfParam(pAlfParam);
    
    ruiRate = m_pcEntropyCoder->getNumberOfWrittenBits();

    if(pvAlfCUCtrlParam != NULL)
    {
      for(UInt s=0; s< m_uiNumSlicesInPic; s++)
      {
        if(! m_pcPic->getValidSlice(s))
        {
          continue;
        }
        m_pcEntropyCoder->resetEntropy();
        m_pcEntropyCoder->resetBits();
        m_pcEntropyCoder->encodeAlfCtrlParam( (*pvAlfCUCtrlParam)[s], m_uiNumCUsInFrame);
        ruiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
      }

    }
    else
    {
      ruiRate += m_uiNumSlicesInPic;
    }
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
    ruiRate = xCalcRateChroma(pAlfParam);
  }
  ruiDist = 0;
  ruiDist += xCalcSSD(pcPicOrg->getCbAddr(), pcPicCmp->getCbAddr(), (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride());
  ruiDist += xCalcSSD(pcPicOrg->getCrAddr(), pcPicCmp->getCrAddr(), (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride());
  rdCost  = (Double)(ruiRate) * m_dLambdaChroma + (Double)(ruiDist);
}

Void TEncAdaptiveLoopFilter::xFilteringFrameChroma(ALFParam* pcAlfParam, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest)
{
  Int filtNo = pcAlfParam->filter_shape_chroma;
  Int *coeff = pcAlfParam->coeff_chroma;
  Int iChromaFormatShift = 1; //4:2:0

  if ((pcAlfParam->chroma_idc>>1)&0x01)
  {
    if(!m_bUseNonCrossALF)
    {
      Int iStride   = pcPicRest->getCStride();
      Pel* pDec  = pcPicDec->getCbAddr();
      Pel* pRest = pcPicRest->getCbAddr();

      filterChroma(pRest, pDec, iStride, 0, (Int)(m_img_height>>1) -1, 0, (Int)(m_img_width>>1)-1, filtNo,  coeff);
    }
    else
    {
      xFilterChromaSlices(ALF_Cb, pcPicDec, pcPicRest, coeff, filtNo, iChromaFormatShift);
    }
  }
  if ((pcAlfParam->chroma_idc)&0x01)
  {
    if(!m_bUseNonCrossALF)
    {
      Int iStride   = pcPicRest->getCStride();
      Pel* pDec  = pcPicDec->getCrAddr();
      Pel* pRest = pcPicRest->getCrAddr();

      filterChroma(pRest, pDec, iStride, 0, (Int)(m_img_height>>1) -1, 0, (Int)(m_img_width>>1)-1, filtNo,  coeff);
    }
    else
    {
      xFilterChromaSlices(ALF_Cr, pcPicDec, pcPicRest, coeff, filtNo, iChromaFormatShift);
    }
  }

  if(pcAlfParam->chroma_idc<3)
  {
    if(pcAlfParam->chroma_idc==1)
    {
      pcPicDec->copyToPicCb(pcPicRest);
    }
    if(pcAlfParam->chroma_idc==2)
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

  if(m_uiNumSlicesInPic > 1)
  {
    Pel* pPicDecLuma  = pcPicDec->getLumaAddr();
    Pel* pPicRestLuma = pcPicRest->getLumaAddr();
    Int  stride       = pcPicDec->getStride();
    UInt SUWidth      = m_pcPic->getMinCUWidth();
    UInt SUHeight     = m_pcPic->getMinCUHeight();

    UInt startSU, endSU, LCUX, LCUY, currSU, LPelX, TPelY;
    UInt posOffset;
    Pel *pDec, *pRest;

    for(Int s=0; s< m_uiNumSlicesInPic; s++)
    {
      if(!m_pcPic->getValidSlice(s))
      {
        continue;
      }
      std::vector< AlfLCUInfo* >&  vpSliceAlfLCU = m_pvpAlfLCU[s]; 
      for(Int i=0; i< vpSliceAlfLCU.size(); i++)
      {
        AlfLCUInfo& rAlfLCU    = *(vpSliceAlfLCU[i]);
        TComDataCU* pcCU       = rAlfLCU.pcCU;
        startSU                = rAlfLCU.startSU;
        endSU                  = rAlfLCU.endSU;
        LCUX                 = pcCU->getCUPelX();
        LCUY                 = pcCU->getCUPelY();

        for(currSU= startSU; currSU<= endSU; currSU++)
        {
          LPelX   = LCUX + g_auiRasterToPelX[ g_auiZscanToRaster[currSU] ];
          TPelY   = LCUY + g_auiRasterToPelY[ g_auiZscanToRaster[currSU] ];
          if( !( LPelX < m_img_width )  || !( TPelY < m_img_height )  )
          {
            continue;
          }
          if(!pcCU->getAlfCtrlFlag(currSU))
          {
            posOffset = TPelY*stride + LPelX;
            pDec = pPicDecLuma + posOffset;
            pRest= pPicRestLuma+ posOffset;
            for(Int y=0; y< SUHeight; y++)
            {
              ::memcpy(pRest, pDec, sizeof(Pel)*SUWidth);
              pDec += stride;
              pRest+= stride;
            }
          }
        }
      }
    }
    return;
  }

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

double TEncAdaptiveLoopFilter::xfindBestCoeffCodMethod(int **filterCoeffSymQuant, int filter_shape, int sqrFiltLength, int filters_per_fr, double errorForce0CoeffTab[NO_VAR_BINS][2], 
  double lambda)
{
  Int coeffBits, i;
  Double error=0, lagrangian;
  coeffBits = xsendAllFiltersPPPred(filterCoeffSymQuant, filter_shape, sqrFiltLength, filters_per_fr, 
    0, m_tempALFp);
  for(i=0;i<filters_per_fr;i++)
  {
    error += errorForce0CoeffTab[i][1];
  }
  lagrangian = error + lambda * coeffBits;
  return (lagrangian);
}

/** Predict ALF luma filter coefficients. Centre coefficient is always predicted. Determines if left neighbour should be predicted.
 */
Void TEncAdaptiveLoopFilter::predictALFCoeffLumaEnc(ALFParam* pcAlfParam, Int **pfilterCoeffSym, Int filter_shape)
{
  Int sum, coeffPred, ind;
  const Int* pFiltMag = NULL;
  pFiltMag = weightsTabShapes[filter_shape];
  for(ind = 0; ind < pcAlfParam->filters_per_group; ++ind)
  {
    sum = 0;
    for(Int i = 0; i < pcAlfParam->num_coeff-2; i++)
    {
      sum +=  pFiltMag[i]*pfilterCoeffSym[ind][i];
    }

    if((pcAlfParam->predMethod==0)|(ind==0))
    {
      coeffPred = ((1<<ALF_NUM_BIT_SHIFT)-sum) >> 2;
    }
    else
    {
      coeffPred = (0-sum) >> 2;
    }
    if(abs(pfilterCoeffSym[ind][pcAlfParam->num_coeff-2]-coeffPred) < abs(pfilterCoeffSym[ind][pcAlfParam->num_coeff-2]))
    {
      pcAlfParam->nbSPred[ind] = 0; 
    }
    else
    {
      pcAlfParam->nbSPred[ind] = 1; 
      coeffPred = 0;
    }
    sum += pFiltMag[pcAlfParam->num_coeff-2]*pfilterCoeffSym[ind][pcAlfParam->num_coeff-2];
    pfilterCoeffSym[ind][pcAlfParam->num_coeff-2] -= coeffPred; 
    if((pcAlfParam->predMethod==0)|(ind==0))
    {
      coeffPred = (1<<ALF_NUM_BIT_SHIFT)-sum;
    }
    else
    {
      coeffPred = -sum;
    }
    pfilterCoeffSym[ind][pcAlfParam->num_coeff-1] -= coeffPred;
  }
}

Int TEncAdaptiveLoopFilter::xsendAllFiltersPPPred(int **FilterCoeffQuant, int fl, int sqrFiltLength, 
                                                  int filters_per_group, int createBistream, ALFParam* ALFp)
{
  int ind, bit_ct = 0, bit_ct0 = 0, i;
  int predMethod = 0;
  int force0 = 0;
  Int64 Newbit_ct;
  
  for(ind = 0; ind < filters_per_group; ind++)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      m_FilterCoeffQuantTemp[ind][i]=FilterCoeffQuant[ind][i];
    }
  }
  ALFp->filters_per_group = filters_per_group;
  ALFp->predMethod = 0;
  ALFp->num_coeff = sqrFiltLength;
  predictALFCoeffLumaEnc(ALFp, m_FilterCoeffQuantTemp, fl);
  Int nbFlagIntra[16];
  for(ind = 0; ind < filters_per_group; ind++)
  {
    nbFlagIntra[ind] = ALFp->nbSPred[ind];
  }
  bit_ct0 = xcodeFilterCoeff(m_FilterCoeffQuantTemp, fl, sqrFiltLength, filters_per_group, 0);
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
  ALFp->predMethod = 1;
  predictALFCoeffLumaEnc(ALFp, m_diffFilterCoeffQuant, fl);
  
  if(xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group, 0) >= bit_ct0)
  {
    predMethod = 0;  
    if(filters_per_group > 1)
    {
      bit_ct += lengthPredFlags(force0, predMethod, NULL, 0, createBistream);
    }
    bit_ct += xcodeFilterCoeff(m_FilterCoeffQuantTemp, fl, sqrFiltLength, filters_per_group, createBistream);
  }
  else
  {
    predMethod = 1;
    if(filters_per_group > 1)
    {
      bit_ct += lengthPredFlags(force0, predMethod, NULL, 0, createBistream);
    }
    bit_ct += xcodeFilterCoeff(m_diffFilterCoeffQuant, fl, sqrFiltLength, filters_per_group, createBistream);
  }
  ALFp->filters_per_group = filters_per_group;
  ALFp->predMethod = predMethod;
  ALFp->num_coeff = sqrFiltLength;
  ALFp->filter_shape = fl;
  for(ind = 0; ind < filters_per_group; ++ind)
  {
    for(i = 0; i < sqrFiltLength; i++)
    {
      if (predMethod) ALFp->coeffmulti[ind][i] = m_diffFilterCoeffQuant[ind][i];
      else 
      {
        ALFp->coeffmulti[ind][i] = m_FilterCoeffQuantTemp[ind][i];
      }
    }
    if(predMethod==0)
    {
      ALFp->nbSPred[ind] = nbFlagIntra[ind];
    }
  }
  m_pcEntropyCoder->codeFiltCountBit(ALFp, &Newbit_ct);
  
  //  return(bit_ct);
  return ((Int)Newbit_ct);
}

Int TEncAdaptiveLoopFilter::xcodeAuxInfo(int filters_per_fr, int varIndTab[NO_VAR_BINS], int filter_shape, ALFParam* ALFp)
{
  int i, filterPattern[NO_VAR_BINS], startSecondFilter=0, bitCt=0;
  Int64 NewbitCt;

  //send realfiltNo (tap related)
  ALFp->filter_shape = filter_shape;

  // decide startSecondFilter and filterPattern
  memset(filterPattern, 0, NO_VAR_BINS * sizeof(int)); 
  if(filters_per_fr > 1)
  {
    for(i = 1; i < NO_VAR_BINS; ++i)
    {
      if(varIndTab[i] != varIndTab[i-1])
      {
        filterPattern[i] = 1;
        startSecondFilter = i;
      }
    }
  }
  memcpy (ALFp->filterPattern, filterPattern, NO_VAR_BINS * sizeof(int));
  ALFp->startSecondFilter = startSecondFilter;

  assert(filters_per_fr>0);
  m_pcEntropyCoder->codeAuxCountBit(ALFp, &NewbitCt);

  bitCt = (int) NewbitCt;
  return(bitCt);
}

Int   TEncAdaptiveLoopFilter::xcodeFilterCoeff(int **pDiffQFilterCoeffIntPP, int fl, int sqrFiltLength, 
                                               int filters_per_group, int createBitstream)
{
  int i, k, kMin, kStart, minBits, ind, scanPos, maxScanVal, coeffVal, len = 0,
    *pDepthInt=NULL, kMinTab[MAX_SCAN_VAL], bitsCoeffScan[MAX_SCAN_VAL][MAX_EXP_GOLOMB],
  minKStart, minBitsKStart, bitsKStart;
  
  int minScanVal = (fl==ALF_STAR5x5) ? 0 : MIN_SCAN_POS_CROSS;

  pDepthInt = pDepthIntTabShapes[fl];
  
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
    for(scanPos = minScanVal; scanPos < maxScanVal; scanPos++)
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
  for(scanPos = minScanVal; scanPos < maxScanVal; scanPos++)
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
  {
    return(q + 2 + k);
  }
  else
  {
    return(q + 1 + k);
  }
}

Int TEncAdaptiveLoopFilter::lengthPredFlags(int force0, int predMethod, int codedVarBins[NO_VAR_BINS], 
                                            int filters_per_group, int createBitstream)
{
  int bit_cnt = 0;
  
  if(force0)
  {
    bit_cnt = 2 + filters_per_group;
  }
  else
  {
    bit_cnt = 2;
  }
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

Void   TEncAdaptiveLoopFilter::xEncALFLuma ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost )
{
  //pcPicDec: extended decoded
  //pcPicRest: original decoded: filtered signal will be stored

  UInt64  uiRate;
  UInt64  uiDist;
  Double dCost;
  Int    LumaStride = pcPicOrg->getStride();
  Pel* pOrg  = pcPicOrg->getLumaAddr();
  Pel* pRest = pcPicRest->getLumaAddr();
  Pel* pDec  = pcPicDec->getLumaAddr();

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

    switch(i)
    {
    case ALF_RA:
      {
        adExtraCostReduction[i] = (double)(m_img_height * m_img_width) * m_dLambdaLuma * 2.0 / 4096.0;
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

    setInitialMask(pcPicOrg, pcPicDec);

    if(m_iALFEncodePassReduction == 0)
    {
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
          pcAlfParam->filter_shape = filter_shape;
          pcAlfParam->num_coeff = m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filter_shape];
          xFirstFilteringFrameLuma(pOrg, pDec, m_pcPicYuvTmp->getLumaAddr(), pcAlfParam, pcAlfParam->filter_shape, LumaStride);
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
        pcAlfParam->filter_shape = best_filter_shape;
        pcAlfParam->num_coeff = m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[best_filter_shape];
        xFirstFilteringFrameLuma(pOrg, pDec, m_pcPicYuvTmp->getLumaAddr(), pcAlfParam, best_filter_shape, LumaStride);
        xCalcRDCost(pcPicOrg, m_pcPicYuvTmp, pcAlfParam, uiRate, uiDist, dCost);
      }
    }
    else
    {
      decideFilterShapeLuma(pOrg, pDec, LumaStride, pcAlfParam, uiRate, uiDist, dCost);
    }

    dCost -= adExtraCostReduction[m_uiVarGenMethod];

    if(dCost < dMinMethodCost)
    {
      iBestClassMethod = m_uiVarGenMethod;
      dMinMethodCost = dCost;
      uiMinMethodRate= uiRate;
      uiMinMethodDist = uiDist;

      if(m_iALFEncodePassReduction == 0)
      {
        m_pcPicYuvTmp->copyToPicLuma(pcPicRest);
      }
    }  
  }

  m_uiVarGenMethod = iBestClassMethod;
  dMinMethodCost += adExtraCostReduction[m_uiVarGenMethod];
  m_varImg= m_varImgMethods[m_uiVarGenMethod];

  pcAlfParam = &(cFrmAlfParam[m_uiVarGenMethod]);

  ALFParam  cAlfParamWithBestMethod;
  allocALFParam(&cAlfParamWithBestMethod);  


  if(m_iALFEncodePassReduction ==0)
  {
    copyALFParam(&cAlfParamWithBestMethod, pcAlfParam); 
  }
  else
  {
    cAlfParamWithBestMethod.alf_flag = 1;
    cAlfParamWithBestMethod.chroma_idc = 0;
    cAlfParamWithBestMethod.alf_pcr_region_flag = m_uiVarGenMethod;
    cAlfParamWithBestMethod.filter_shape= pcAlfParam->filter_shape;
    cAlfParamWithBestMethod.num_coeff = m_sqrFiltLengthTab[cAlfParamWithBestMethod.filter_shape]; 
    decodeFilterSet(pcAlfParam, m_varIndTab, m_filterCoeffSym);
    if(!m_bUseNonCrossALF)
    {
      filterLuma(pRest, pDec, LumaStride, 0, m_img_height-1, 0, m_img_width-1,  pcAlfParam->filter_shape, m_filterCoeffSym, m_varIndTab, m_varImg);
    }
    else
    {
      xfilterSlicesEncoder(pDec, pRest, LumaStride, pcAlfParam->filter_shape, m_filterCoeffSym, m_varIndTab, m_varImg);
    }
    xcodeFiltCoeff(m_filterCoeffSym, pcAlfParam->filter_shape, m_varIndTab, pcAlfParam->filters_per_group,&cAlfParamWithBestMethod);

    xCalcRDCost(pcPicOrg, pcPicRest, &cAlfParamWithBestMethod, uiMinMethodRate, uiMinMethodDist, dMinMethodCost);

  }

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
}



Void   TEncAdaptiveLoopFilter::xFirstFilteringFrameLuma(Pel* imgOrg, Pel* imgDec, Pel* imgRest, ALFParam* ALFp, Int filtNo, Int stride)
{
  if(!m_bUseNonCrossALF)
  {
    xstoreInBlockMatrix(0, 0, m_img_height, m_img_width, true, true, imgOrg, imgDec, filtNo, stride);
  }
  else
  {
    xstoreInBlockMatrixforSlices(imgOrg, imgDec, filtNo, stride);
  }


  xFilteringFrameLuma(imgOrg, imgDec, imgRest, ALFp, filtNo, stride);
}

Void   TEncAdaptiveLoopFilter::xstoreInBlockMatrix(Int ypos, Int xpos, Int iheight, Int iwidth, Bool bResetBlockMatrix, Bool bSymmCopyBlockMatrix, Pel* pImgOrg, Pel* pImgPad, Int filtNo, Int stride)
{

  Pel  regionOfInterested = (m_iDesignCurrentFilter ==1)?(1):(0);
  Int     sqrFiltLength      = (filtNo == 2)?((Int)(MAX_SQR_FILT_LENGTH)):(m_sqrFiltLengthTab[filtNo]);
  Int     yposEnd            = ypos + iheight -1;
  Int     xposEnd            = xpos + iwidth  -1;
  Double ***EShape           = m_EGlobalSym[filtNo];
  Double **yShape            = m_yGlobalSym[filtNo];

  Int ELocal[MAX_SQR_FILT_LENGTH];
  Pel *pImgPad1, *pImgPad2, *pImgPad3, *pImgPad4;
  Int i,j,k,l,varInd, yLocal;
  double **E,*yy;

  static Int numValidPels;
  if(bResetBlockMatrix)
  {
    numValidPels = 0;
    memset( m_pixAcc, 0,sizeof(double)*NO_VAR_BINS);
    for (varInd=0; varInd<NO_VAR_BINS; varInd++)
    {
      memset(yShape[varInd],0,sizeof(double)*MAX_SQR_FILT_LENGTH);
      for (k=0; k<sqrFiltLength; k++)
      {
        memset(EShape[varInd][k],0,sizeof(double)*MAX_SQR_FILT_LENGTH);
      }
    }
    for (i = 0; i < m_img_height; i++)
    {
      for (j = 0; j < m_img_width; j++)
      {
        if (m_maskImg[i][j] == regionOfInterested)
        {
          numValidPels++;
        }
      }
    }
  }

  Int yLineInLCU;
  Int paddingLine ;

  pImgPad += (ypos* stride);
  pImgOrg += (ypos* stride);

  switch(filtNo)
  {
  case ALF_STAR5x5:
    {
      for (i= ypos; i<= yposEnd; i++)
      {
        yLineInLCU = i % m_lcuHeight;

        if (yLineInLCU < m_lineIdxPadBot || i-yLineInLCU+m_lcuHeight >= m_img_height )
        {
          pImgPad1 = pImgPad +   stride;
          pImgPad2 = pImgPad -   stride;
          pImgPad3 = pImgPad + 2*stride;
          pImgPad4 = pImgPad - 2*stride;
        }
        else if (yLineInLCU < m_lineIdxPadTop)
        {
          paddingLine = - yLineInLCU + m_lineIdxPadTop - 1;
          pImgPad1 = pImgPad + min(paddingLine, 1)*stride;
          pImgPad2 = pImgPad -   stride;
          pImgPad3 = pImgPad + min(paddingLine, 2)*stride;
          pImgPad4 = pImgPad - 2*stride;
        }
        else
        {
          paddingLine = yLineInLCU - m_lineIdxPadTop;
          pImgPad1 = pImgPad +   stride;
          pImgPad2 = pImgPad - min(paddingLine, 1)*stride;
          pImgPad3 = pImgPad + 2*stride;
          pImgPad4 = pImgPad - min(paddingLine, 2)*stride;
        }

        if ( (yLineInLCU == m_lineIdxPadTop || yLineInLCU == m_lineIdxPadTop-1) && i-yLineInLCU+m_lcuHeight < m_img_height ) 
        {
          pImgPad+= stride;
          pImgOrg+= stride;
          continue;
        }
        else
        {
        for (j= xpos; j<= xposEnd; j++)
        {
          if ( (m_maskImg[i][j] == regionOfInterested) || (numValidPels == 0) )
          {
            varInd = m_varImg[i/VAR_SIZE_H][j/VAR_SIZE_W];
            memset(ELocal, 0, 10*sizeof(Int));

            ELocal[0] = (pImgPad3[j+2] + pImgPad4[j-2]);
            ELocal[1] = (pImgPad3[j  ] + pImgPad4[j  ]);
            ELocal[2] = (pImgPad3[j-2] + pImgPad4[j+2]);

            ELocal[3] = (pImgPad1[j+1] + pImgPad2[j-1]);
            ELocal[4] = (pImgPad1[j  ] + pImgPad2[j  ]);
            ELocal[5] = (pImgPad1[j-1] + pImgPad2[j+1]);

            ELocal[6] = (pImgPad[j+2] + pImgPad[j-2]);
            ELocal[7] = (pImgPad[j+1] + pImgPad[j-1]);
            ELocal[8] = (pImgPad[j  ]);

            yLocal= pImgOrg[j];
            m_pixAcc[varInd]+=(yLocal*yLocal);
            E= EShape[varInd];  
            yy= yShape[varInd];

            for (k=0; k<10; k++)
            {
              for (l=k; l<10; l++)
              {
                E[k][l]+=(double)(ELocal[k]*ELocal[l]);
              }
              yy[k]+=(double)(ELocal[k]*yLocal);
            }

          }
        }
        pImgPad+= stride;
        pImgOrg+= stride;
        }
      }
    }
    break;

  case ALF_CROSS9x9: 
    {
      Pel *pImgPad5, *pImgPad6, *pImgPad7, *pImgPad8;

      for (i= ypos; i<= yposEnd; i++)
      {
        yLineInLCU = i % m_lcuHeight;

        if (yLineInLCU<m_lineIdxPadBot || i-yLineInLCU+m_lcuHeight >= m_img_height)
        {
          pImgPad1 = pImgPad +   stride;
          pImgPad2 = pImgPad -   stride;
          pImgPad3 = pImgPad + 2*stride;
          pImgPad4 = pImgPad - 2*stride;
          pImgPad5 = pImgPad + 3*stride;
          pImgPad6 = pImgPad - 3*stride;
          pImgPad7 = pImgPad + 4*stride;
          pImgPad8 = pImgPad - 4*stride;
        }
        else if (yLineInLCU<m_lineIdxPadTop)
        {
          paddingLine = - yLineInLCU + m_lineIdxPadTop - 1;
          pImgPad1 = (paddingLine < 1) ? pImgPad : pImgPad + min(paddingLine, 1)*stride;
          pImgPad2 = (paddingLine < 1) ? pImgPad : pImgPad -   stride;
          pImgPad3 = (paddingLine < 2) ? pImgPad : pImgPad + min(paddingLine, 2)*stride;
          pImgPad4 = (paddingLine < 2) ? pImgPad : pImgPad - 2*stride;
          pImgPad5 = (paddingLine < 3) ? pImgPad : pImgPad + min(paddingLine, 3)*stride;
          pImgPad6 = (paddingLine < 3) ? pImgPad : pImgPad - 3*stride;
          pImgPad7 = (paddingLine < 4) ? pImgPad : pImgPad + min(paddingLine, 4)*stride;
          pImgPad8 = (paddingLine < 4) ? pImgPad : pImgPad - 4*stride;
        }
        else
        {
          paddingLine = yLineInLCU - m_lineIdxPadTop;
          pImgPad1 = (paddingLine < 1) ? pImgPad : pImgPad +   stride;
          pImgPad2 = (paddingLine < 1) ? pImgPad : pImgPad - min(paddingLine, 1)*stride;
          pImgPad3 = (paddingLine < 2) ? pImgPad : pImgPad + 2*stride;
          pImgPad4 = (paddingLine < 2) ? pImgPad : pImgPad - min(paddingLine, 2)*stride;
          pImgPad5 = (paddingLine < 3) ? pImgPad : pImgPad + 3*stride;
          pImgPad6 = (paddingLine < 3) ? pImgPad : pImgPad - min(paddingLine, 3)*stride;
          pImgPad7 = (paddingLine < 4) ? pImgPad : pImgPad + 4*stride;
          pImgPad8 = (paddingLine < 4) ? pImgPad : pImgPad - min(paddingLine, 4)*stride;
        }         

        for (j= xpos; j<= xposEnd; j++)
        {
          if ( (m_maskImg[i][j] == regionOfInterested) || (numValidPels == 0) )
          {
            varInd = m_varImg[i/VAR_SIZE_H][j/VAR_SIZE_W];
            memset(ELocal, 0, 10*sizeof(Int));

            ELocal[0] = (pImgPad7[j] + pImgPad8[j]);

            ELocal[1] = (pImgPad5[j] + pImgPad6[j]);

            ELocal[2] = (pImgPad3[j] + pImgPad4[j]);

            ELocal[3] = (pImgPad1[j] + pImgPad2[j]);

            ELocal[4] = (pImgPad[j+4] + pImgPad[j-4]);
            ELocal[5] = (pImgPad[j+3] + pImgPad[j-3]);
            ELocal[6] = (pImgPad[j+2] + pImgPad[j-2]);
            ELocal[7] = (pImgPad[j+1] + pImgPad[j-1]);
            ELocal[8] = (pImgPad[j  ] );

            yLocal= pImgOrg[j];
            m_pixAcc[varInd]+=(yLocal*yLocal);
            E= EShape[varInd];
            yy= yShape[varInd];

            for (k=0; k<10; k++)
            {
              for (l=k; l<10; l++)
              {
                E[k][l]+=(double)(ELocal[k]*ELocal[l]);
              }
              yy[k]+=(double)(ELocal[k]*yLocal);
            }

          }
        }
        pImgPad+= stride;
        pImgOrg+= stride;
      }

    }
    break;
  default:
    {
      printf("Not a supported filter shape\n");
      assert(0);
      exit(1);
    }
  }

  if(bSymmCopyBlockMatrix)
  {
    for (varInd=0; varInd<NO_VAR_BINS; varInd++)
    {
      double **pE = EShape[varInd];
      for (k=1; k<sqrFiltLength; k++)
      {
        for (l=0; l<k; l++)
        {
          pE[k][l]=pE[l][k];
        }
      }
    }
  }
}


Void   TEncAdaptiveLoopFilter::xFilteringFrameLuma(Pel* imgOrg, Pel* imgPad, Pel* imgFilt, ALFParam* ALFp, Int filtNo, Int stride)
{
  static double **ySym, ***ESym;
  Int  filters_per_fr;
  Int lambdaVal = (Int) m_dLambdaLuma;
  lambdaVal = lambdaVal * (1<<(2*g_uiBitIncrement));

  ESym=m_EGlobalSym[filtNo];  
  ySym=m_yGlobalSym[filtNo];

  xfindBestFilterVarPred(ySym, ESym, m_pixAcc, m_filterCoeffSym, m_filterCoeffSymQuant, filtNo, &filters_per_fr,m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambdaVal);

  if(!m_bUseNonCrossALF)
  {
    filterLuma(imgFilt, imgPad, stride, 0, m_img_height-1, 0, m_img_width-1,  ALFp->filter_shape, m_filterCoeffSym, m_varIndTab, m_varImg);
  }
  else
  {
    xfilterSlicesEncoder(imgPad, imgFilt, stride, filtNo, m_filterCoeffSym, m_varIndTab, m_varImg);
  }

  xcodeFiltCoeff(m_filterCoeffSymQuant, filtNo, m_varIndTab, filters_per_fr,ALFp);
}


Void TEncAdaptiveLoopFilter::xfindBestFilterVarPred(double **ySym, double ***ESym, double *pixAcc, Int **filterCoeffSym, Int **filterCoeffSymQuant, Int filter_shape, Int *filters_per_fr_best, Int varIndTab[], Pel **imgY_rec, Pel **varImg, Pel **maskImg, Pel **imgY_pad, double lambda_val)
{
  Int filters_per_fr, firstFilt, interval[NO_VAR_BINS][2], intervalBest[NO_VAR_BINS][2];
  int i;
  double  lagrangian, lagrangianMin;
  int sqrFiltLength;
  int *weights;
  Int coeffBits;
  double errorForce0CoeffTab[NO_VAR_BINS][2];
  
  sqrFiltLength= m_sqrFiltLengthTab[filter_shape] ;
  weights = weightsTabShapes[filter_shape];

  // zero all variables 
  memset(varIndTab,0,sizeof(int)*NO_VAR_BINS);

  for(i = 0; i < NO_VAR_BINS; i++)
  {
    memset(filterCoeffSym[i],0,sizeof(int)*ALF_MAX_NUM_COEF);
    memset(filterCoeffSymQuant[i],0,sizeof(int)*ALF_MAX_NUM_COEF);
  }

  firstFilt=1;  lagrangianMin=0;
  filters_per_fr=NO_FILTERS;

  while(filters_per_fr>=1)
  {
    mergeFiltersGreedy(ySym, ESym, pixAcc, interval, sqrFiltLength, filters_per_fr);
    findFilterCoeff(ESym, ySym, pixAcc, filterCoeffSym, filterCoeffSymQuant, interval,
      varIndTab, sqrFiltLength, filters_per_fr, weights, errorForce0CoeffTab);

    lagrangian=xfindBestCoeffCodMethod(filterCoeffSymQuant, filter_shape, sqrFiltLength, filters_per_fr, errorForce0CoeffTab, lambda_val);

    if (lagrangian<lagrangianMin || firstFilt==1 || filters_per_fr == m_iALFMaxNumberFilters)
    {
      firstFilt=0;
      lagrangianMin=lagrangian;

      (*filters_per_fr_best)=filters_per_fr;
      memcpy(intervalBest, interval, NO_VAR_BINS*2*sizeof(int));
    }
    filters_per_fr--;
  }

  if ( (m_uiVarGenMethod == ALF_BA) && ((*filters_per_fr_best) > 1) )
  {
    Int iLastFilter = (*filters_per_fr_best)-1;
    if (intervalBest[iLastFilter][0] == NO_VAR_BINS-1)
    {
      intervalBest[iLastFilter-1][1] = NO_VAR_BINS-1;
      (*filters_per_fr_best) = iLastFilter;
    }
  }

  findFilterCoeff(ESym, ySym, pixAcc, filterCoeffSym, filterCoeffSymQuant, intervalBest,
    varIndTab, sqrFiltLength, (*filters_per_fr_best), weights, errorForce0CoeffTab);


  xfindBestCoeffCodMethod(filterCoeffSymQuant, filter_shape, sqrFiltLength, (*filters_per_fr_best), errorForce0CoeffTab, lambda_val);
  coeffBits = xcodeAuxInfo((*filters_per_fr_best), varIndTab, filter_shape, m_tempALFp);
  coeffBits += xsendAllFiltersPPPred(filterCoeffSymQuant, filter_shape, sqrFiltLength, (*filters_per_fr_best), 0, m_tempALFp);

  if( *filters_per_fr_best == 1)
  {
    ::memset(varIndTab, 0, sizeof(Int)*NO_VAR_BINS);
  }
}


/** code filter coefficients
 * \param filterCoeffSymQuant filter coefficients buffer
 * \param filtNo filter No.
 * \param varIndTab[] merge index information
 * \param filters_per_fr_best the number of filters used in this picture
 * \param frNo 
 * \param ALFp ALF parameters
 * \returns bitrate
 */
UInt TEncAdaptiveLoopFilter::xcodeFiltCoeff(Int **filterCoeffSymQuant, Int filter_shape, Int varIndTab[], Int filters_per_fr_best, ALFParam* ALFp)
{
  Int coeffBits;   
  Int sqrFiltLength = m_sqrFiltLengthTab[filter_shape] ; 

  ALFp->filters_per_group = filters_per_fr_best;

  coeffBits = xcodeAuxInfo(filters_per_fr_best, varIndTab, filter_shape, ALFp);


  ALFp->predMethod = 0;
  ALFp->num_coeff = sqrFiltLength;
  ALFp->filter_shape=filter_shape;

  if (filters_per_fr_best <= 1)
  {
    ALFp->predMethod = 0;
  }

  coeffBits += xsendAllFiltersPPPred(filterCoeffSymQuant, filter_shape, sqrFiltLength, 
    filters_per_fr_best, 1, ALFp);

  return (UInt)coeffBits;
}

Void TEncAdaptiveLoopFilter::getCtrlFlagsFromCU(AlfLCUInfo* pcAlfLCU, std::vector<UInt> *pvFlags, Int alfDepth, UInt maxNumSUInLCU)
{
  const UInt startSU               = pcAlfLCU->startSU;
  const UInt endSU                 = pcAlfLCU->endSU;
  const Bool bAllSUsInLCUInSameSlice = pcAlfLCU->bAllSUsInLCUInSameSlice;

  TComDataCU* pcCU = pcAlfLCU->pcCU;
  UInt  currSU, CUDepth, setDepth, ctrlNumSU;

  currSU = startSU;

  if(bAllSUsInLCUInSameSlice)
  {
    while(currSU < maxNumSUInLCU)
    {
      //depth of this CU
      CUDepth = pcCU->getDepth(currSU);

      //choose the min. depth for ALF
      setDepth   = (alfDepth < CUDepth)?(alfDepth):(CUDepth);
      ctrlNumSU = maxNumSUInLCU >> (setDepth << 1);

      pvFlags->push_back(pcCU->getAlfCtrlFlag(currSU));
      currSU += ctrlNumSU;
    }

    return;
  }


  const UInt  LCUX = pcCU->getCUPelX();
  const UInt  LCUY = pcCU->getCUPelY();

  Bool  bFirst, bValidCU;
  UInt  idx, LPelXSU, TPelYSU;

  bFirst= true;
  while(currSU <= endSU)
  {
    //check picture boundary
    while(!( LCUX + g_auiRasterToPelX[ g_auiZscanToRaster[currSU] ] < m_img_width  ) || 
          !( LCUY + g_auiRasterToPelY[ g_auiZscanToRaster[currSU] ] < m_img_height )
      )
    {
      currSU++;

      if(currSU >= maxNumSUInLCU || currSU > endSU)
      {
        break;
      }
    }

    if(currSU >= maxNumSUInLCU || currSU > endSU)
    {
      break;
    }

    //depth of this CU
    CUDepth = pcCU->getDepth(currSU);

    //choose the min. depth for ALF
    setDepth   = (alfDepth < CUDepth)?(alfDepth):(CUDepth);
    ctrlNumSU = maxNumSUInLCU >> (setDepth << 1);

    if(bFirst)
    {
      if(currSU !=0 )
      {
        currSU = ((UInt)(currSU/ctrlNumSU))* ctrlNumSU;
      }
      bFirst = false;
    }

    bValidCU = false;
    for(idx = currSU; idx < currSU + ctrlNumSU; idx++)
    {
      if(idx < startSU || idx > endSU)
      {
        continue;
      }

      LPelXSU   = LCUX + g_auiRasterToPelX[ g_auiZscanToRaster[idx] ];
      TPelYSU   = LCUY + g_auiRasterToPelY[ g_auiZscanToRaster[idx] ];

      if( !( LPelXSU < m_img_width )  || !( TPelYSU < m_img_height )  )
      {
        continue;
      }

      bValidCU = true;
    }

    if(bValidCU)
    {
      pvFlags->push_back(pcCU->getAlfCtrlFlag(currSU));
    }

    currSU += ctrlNumSU;
  }
}


/** set ALF CU control flags
 * \param [in] uiAlfCtrlDepth ALF CU control depth
 * \param [in] pcPicOrg picture of original signal
 * \param [in] pcPicDec picture before filtering
 * \param [in] pcPicRest picture after filtering
 * \param [out] ruiDist distortion after CU control
 * \param [in,out]vAlfCUCtrlParam ALF CU control parameters 
 */
Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlags_qc(UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam)
{
  ruiDist = 0;
  std::vector<UInt> uiFlags;

  //initial
  for(Int s=0; s< m_uiNumSlicesInPic; s++)
  {
    vAlfCUCtrlParam[s].cu_control_flag = 1;
    vAlfCUCtrlParam[s].alf_max_depth   = uiAlfCtrlDepth;

    vAlfCUCtrlParam[s].alf_cu_flag.reserve(m_uiNumCUsInFrame << ((g_uiMaxCUDepth-1)*2));
    vAlfCUCtrlParam[s].alf_cu_flag.resize(0);
  }

  //LCU-based on/off control
  for( UInt CUAddr = 0; CUAddr < m_pcPic->getNumCUsInFrame() ; CUAddr++ )
  {
    TComDataCU* pcCU = m_pcPic->getCU( CUAddr );
    xSetCUAlfCtrlFlag_qc(pcCU, 0, 0, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist, vAlfCUCtrlParam[0].alf_cu_flag);
  }
  vAlfCUCtrlParam[0].num_alf_cu_flag = (UInt)(vAlfCUCtrlParam[0].alf_cu_flag.size());


  if(m_uiNumSlicesInPic > 1)
  {
    //reset the first slice on/off flags
    vAlfCUCtrlParam[0].alf_cu_flag.resize(0);

    //distribute on/off flags to slices
    std::vector<UInt> vCtrlFlags;
    vCtrlFlags.reserve(1 << ((g_uiMaxCUDepth-1)*2));

    for(Int s=0; s < m_uiNumSlicesInPic; s++)
    {
      if(!m_pcPic->getValidSlice(s))
      {
        continue;
      }
      std::vector< AlfLCUInfo* >& vpAlfLCU = m_pvpAlfLCU[s];
      for(Int i=0; i< vpAlfLCU.size(); i++)
      {
        //get on/off flags for one LCU
        vCtrlFlags.resize(0);
        getCtrlFlagsFromCU(vpAlfLCU[i], &vCtrlFlags, (Int)uiAlfCtrlDepth, m_pcPic->getNumPartInCU());

        for(Int k=0; k< vCtrlFlags.size(); k++)
        {
          vAlfCUCtrlParam[s].alf_cu_flag.push_back( vCtrlFlags[k]);
        }
      } //i (LCU)
      vAlfCUCtrlParam[s].num_alf_cu_flag = (UInt)(vAlfCUCtrlParam[s].alf_cu_flag.size());
    } //s (Slice)
  }
}


Void TEncAdaptiveLoopFilter::xSetCUAlfCtrlFlag_qc(TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, std::vector<UInt>& vCUCtrlFlag)
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
        xSetCUAlfCtrlFlag_qc(pcCU, uiAbsPartIdx, uiDepth+1, uiAlfCtrlDepth, pcPicOrg, pcPicDec, pcPicRest, ruiDist, vCUCtrlFlag);
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
    vCUCtrlFlag.push_back(1);
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
    vCUCtrlFlag.push_back(0);
    for (int i=uiTPelY ;i<=min(uiBPelY,(unsigned int)(pcPicOrg->getHeight()-1))  ;i++)
    {
      for (int j=uiLPelX ;j<=min(uiRPelX,(unsigned int)(pcPicOrg->getWidth()-1)) ;j++)
      { 
        m_maskImg[i][j]=0;
      }
    }
  }
}

Void TEncAdaptiveLoopFilter::xReDesignFilterCoeff_qc(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, Bool bReadCorr)
{
  Int tap = m_pcTempAlfParam->filter_shape;
  Int    LumaStride = pcPicOrg->getStride();
  Pel* pOrg  = pcPicOrg->getLumaAddr();
  Pel* pDec  = pcPicDec->getLumaAddr();
  Pel* pRest = pcPicRest->getLumaAddr();
  xFirstFilteringFrameLuma(pOrg, pDec, pRest, m_pcTempAlfParam, tap, LumaStride); 
  
  if (m_iALFEncodePassReduction)
  {
    if(!m_iUsePreviousFilter)
    {
      saveFilterCoeffToBuffer(m_filterCoeffSym, m_pcTempAlfParam->filters_per_group, m_varIndTab, m_pcTempAlfParam->alf_pcr_region_flag, tap);
    }
  }
}

Void TEncAdaptiveLoopFilter::xCUAdaptiveControl_qc(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost)
{
  if(!m_bAlfCUCtrlEnabled) return;
  Bool bChanged = false;
  std::vector<AlfCUCtrlInfo> vAlfCUCtrlParamTemp(m_vBestAlfCUCtrlParam);

  Pel** maskImgTemp;

  if(m_iALFEncodePassReduction == 2)
  {
    initMatrix_Pel(&maskImgTemp, m_img_height, m_img_width);
  }

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

    for (UInt uiRD = 0; uiRD <= m_iALFNumOfRedesign; uiRD++)
    {
      if (uiRD)
      {
        // re-design filter coefficients
        xReDesignFilterCoeff_qc(pcPicOrg, pcPicDec, m_pcPicYuvTmp, true); //use filtering of mine
      }
      
      UInt64 uiRate, uiDist;
      Double dCost;
     //m_pcPicYuvTmp: filtered signal, pcPicDec: orig reconst
      xSetCUAlfCtrlFlags_qc(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, vAlfCUCtrlParamTemp); 
      xCalcRDCost(m_pcTempAlfParam, uiRate, uiDist, dCost, &vAlfCUCtrlParamTemp);
      if (dCost < rdMinCost)
      {
        bChanged = true;
        m_vBestAlfCUCtrlParam = vAlfCUCtrlParamTemp;
        uiBestDepth = uiDepth;
        rdMinCost = dCost;
        ruiMinDist = uiDist;
        ruiMinRate = uiRate;
        m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
        //save maskImg
        xCopyTmpAlfCtrlFlagsFrom();
        if(m_iALFEncodePassReduction == 2)
        {
          ::memcpy(maskImgTemp[0], m_maskImg[0], sizeof(Pel)*m_img_height* m_img_width);
        }
      }
    }
  }

  if(bChanged)
  {
    if(m_iALFEncodePassReduction == 2)
    {
      UInt uiDepth = uiBestDepth;
      ::memcpy(m_maskImg[0], maskImgTemp[0], sizeof(Pel)*m_img_height* m_img_width);
      xCopyTmpAlfCtrlFlagsTo();
  
      copyALFParam(&cFrmAlfParam, m_pcBestAlfParam);

      m_pcEntropyCoder->setAlfCtrl(true);
      m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
      copyALFParam(m_pcTempAlfParam, &cFrmAlfParam);

      xReDesignFilterCoeff_qc(pcPicOrg, pcPicDec, m_pcPicYuvTmp, true); //use filtering of mine

      UInt64 uiRate, uiDist;
      Double dCost;
      xSetCUAlfCtrlFlags_qc(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, vAlfCUCtrlParamTemp); 
      xCalcRDCost(m_pcTempAlfParam, uiRate, uiDist, dCost, &vAlfCUCtrlParamTemp);
      if (dCost < rdMinCost)
      {
        rdMinCost = dCost;
        ruiMinDist = uiDist;
        ruiMinRate = uiRate;
        m_pcPicYuvTmp->copyToPicLuma(m_pcPicYuvBest);
        copyALFParam(m_pcBestAlfParam, m_pcTempAlfParam);
        xCopyTmpAlfCtrlFlagsFrom();
        m_vBestAlfCUCtrlParam = vAlfCUCtrlParamTemp;
      }
    }

    m_pcEntropyCoder->setAlfCtrl(true);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(uiBestDepth);
    xCopyTmpAlfCtrlFlagsTo();

    m_pcPicYuvBest->copyToPicLuma(pcPicRest);//copy m_pcPicYuvBest to pcPicRest
    xCopyDecToRestCUs(pcPicDec, pcPicRest); //pcPicRest = pcPicDec
  }
  else
  {
    m_pcEntropyCoder->setAlfCtrl(false);
    m_pcEntropyCoder->setMaxAlfCtrlDepth(0);
  }
  freeALFParam(&cFrmAlfParam);

  if(m_iALFEncodePassReduction == 2)
  {
    destroyMatrix_Pel(maskImgTemp);
  }
}

#define ROUND(a)  (((a) < 0)? (int)((a) - 0.5) : (int)((a) + 0.5))
#define REG              0.0001
#define REG_SQR          0.0000001

//Find filter coeff related
Int TEncAdaptiveLoopFilter::gnsCholeskyDec(Double **inpMatr, Double outMatr[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], Int noEq)
{ 
  Int i, j, k;     /* Looping Variables */
  Double scale;       /* scaling factor for each row */
  Double invDiag[ALF_MAX_NUM_COEF];  /* Vector of the inverse of diagonal entries of outMatr */
  
  //  Cholesky decomposition starts
  
  for(i = 0; i < noEq; i++)
  {
    for(j = i; j < noEq; j++)
    {
      /* Compute the scaling factor */
      scale = inpMatr[i][j];
      if ( i > 0) 
      {
        for( k = i - 1 ; k >= 0 ; k--)
        {
          scale -= outMatr[k][j] * outMatr[k][i];
        }
      }
      /* Compute i'th row of outMatr */
      if(i == j)
      {
        if(scale <= REG_SQR ) // if(scale <= 0 )  /* If inpMatr is singular */
        {
          return 0;
        }
        else
        {
           /* Normal operation */
           invDiag[i] =  1.0 / (outMatr[i][i] = sqrt(scale));
        }
      }
      else
      {
        outMatr[i][j] = scale * invDiag[i]; /* Upper triangular part          */
        outMatr[j][i] = 0.0;              /* Lower triangular part set to 0 */
      }                    
    }
  }
  return 1; /* Signal that Cholesky factorization is successfully performed */
}


Void TEncAdaptiveLoopFilter::gnsTransposeBacksubstitution(Double U[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], Double rhs[], Double x[], Int order)
{
  Int i,j;              /* Looping variables */
  Double sum;              /* Holds backsubstitution from already handled rows */
  
  /* Backsubstitution starts */
  x[0] = rhs[0] / U[0][0];               /* First row of U'                   */
  for (i = 1; i < order; i++)
  {         /* For the rows 1..order-1           */
    
    for (j = 0, sum = 0.0; j < i; j++) /* Backsubst already solved unknowns */
    {
      sum += x[j] * U[j][i];
    }
    x[i] = (rhs[i] - sum) / U[i][i];       /* i'th component of solution vect.  */
  }
}

Void  TEncAdaptiveLoopFilter::gnsBacksubstitution(Double R[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], Double z[ALF_MAX_NUM_COEF], Int R_size, Double A[MAX_SQR_FILT_LENGTH])
{
  Int i, j;
  Double sum;
  
  R_size--;
  
  A[R_size] = z[R_size] / R[R_size][R_size];
  
  for (i = R_size-1; i >= 0; i--)
  {
    for (j = i + 1, sum = 0.0; j <= R_size; j++)
    {
      sum += R[i][j] * A[j];
    }
    
    A[i] = (z[i] - sum) / R[i][i];
  }
}


Int TEncAdaptiveLoopFilter::gnsSolveByChol(Double **LHS, Double *rhs, Double *x, Int noEq)
{
  assert(noEq > 0);

  Double aux[ALF_MAX_NUM_COEF];     /* Auxiliary vector */
  Double U[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF];    /* Upper triangular Cholesky factor of LHS */
  Int  i, singular;          /* Looping variable */
  
  /* The equation to be solved is LHSx = rhs */
  
  /* Compute upper triangular U such that U'*U = LHS */
  if(gnsCholeskyDec(LHS, U, noEq)) /* If Cholesky decomposition has been successful */
  {
    singular = 1;
    /* Now, the equation is  U'*U*x = rhs, where U is upper triangular
     * Solve U'*aux = rhs for aux
     */
    gnsTransposeBacksubstitution(U, rhs, aux, noEq);         
    
    /* The equation is now U*x = aux, solve it for x (new motion coefficients) */
    gnsBacksubstitution(U, aux, noEq, x);   
    
  }
  else /* LHS was singular */ 
  {
    singular = 0;
    
    /* Regularize LHS */
    for(i=0; i < noEq; i++)
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
      x[0] = 1.0;
      for (i = 1; i < noEq; i++ )
      {
        x[i] = 0.0;
      }
    }
  }  
  return singular;
}

Void TEncAdaptiveLoopFilter::add_A(Double **Amerged, Double ***A, Int start, Int stop, Int size)
{ 
  Int i, j, ind;          /* Looping variable */
  
  for (i = 0; i < size; i++)
  {
    for (j = 0; j < size; j++)
    {
      Amerged[i][j] = 0;
      for (ind = start; ind <= stop; ind++)
      {
        Amerged[i][j] += A[ind][i][j];
      }
    }
  }
}

Void TEncAdaptiveLoopFilter::add_b(Double *bmerged, Double **b, Int start, Int stop, Int size)
{ 
  Int i, ind;          /* Looping variable */
  
  for (i = 0; i < size; i++)
  {
    bmerged[i] = 0;
    for (ind = start; ind <= stop; ind++)
    {
      bmerged[i] += b[ind][i];
    }
  }
}

Double TEncAdaptiveLoopFilter::calculateErrorCoeffProvided(Double **A, Double *b, Double *c, Int size)
{
  Int i, j;
  Double error, sum = 0;
  
  error = 0;
  for (i = 0; i < size; i++)   //diagonal
  {
    sum = 0;
    for (j = i + 1; j < size; j++)
    {
      sum += (A[j][i] + A[i][j]) * c[j];
    }
    error += (A[i][i] * c[i] + sum - 2 * b[i]) * c[i];
  }
  
  return error;
}

Double TEncAdaptiveLoopFilter::calculateErrorAbs(Double **A, Double *b, Double y, Int size)
{
  Int i;
  Double error, sum;
  Double c[ALF_MAX_NUM_COEF];
  
  gnsSolveByChol(A, b, c, size);
  
  sum = 0;
  for (i = 0; i < size; i++)
  {
    sum += c[i] * b[i];
  }
  error = y - sum;
  
  return error;
}

Double TEncAdaptiveLoopFilter::mergeFiltersGreedy(Double **yGlobalSeq, Double ***EGlobalSeq, Double *pixAccGlobalSeq, Int intervalBest[NO_VAR_BINS][2], Int sqrFiltLength, Int noIntervals)
{
  Int first, ind, ind1, ind2, i, j, bestToMerge ;
  Double error, error1, error2, errorMin;
  static Double pixAcc_temp, error_tab[NO_VAR_BINS],error_comb_tab[NO_VAR_BINS];
  static Int indexList[NO_VAR_BINS], available[NO_VAR_BINS], noRemaining;
  if (noIntervals == NO_FILTERS)
  {
    noRemaining = NO_VAR_BINS;
    for (ind=0; ind<NO_VAR_BINS; ind++)
    {
      indexList[ind] = ind; 
      available[ind] = 1;
      m_pixAcc_merged[ind] = pixAccGlobalSeq[ind];
      memcpy(m_y_merged[ind], yGlobalSeq[ind], sizeof(Double)*sqrFiltLength);
      for (i=0; i < sqrFiltLength; i++)
      {
        memcpy(m_E_merged[ind][i], EGlobalSeq[ind][i], sizeof(Double)*sqrFiltLength);
      }
    }
  }
  // Try merging different matrices
  if (noIntervals == NO_FILTERS)
  {
    for (ind = 0; ind < NO_VAR_BINS; ind++)
    {
      error_tab[ind] = calculateErrorAbs(m_E_merged[ind], m_y_merged[ind], m_pixAcc_merged[ind], sqrFiltLength);
    }
    for (ind = 0; ind < NO_VAR_BINS - 1; ind++)
    {
      ind1 = indexList[ind];
      ind2 = indexList[ind+1];
      
      error1 = error_tab[ind1];
      error2 = error_tab[ind2];
      
      pixAcc_temp = m_pixAcc_merged[ind1] + m_pixAcc_merged[ind2];
      for (i = 0; i < sqrFiltLength; i++)
      {
        m_y_temp[i] = m_y_merged[ind1][i] + m_y_merged[ind2][i];
        for (j = 0; j < sqrFiltLength; j++)
        {
          m_E_temp[i][j] = m_E_merged[ind1][i][j] + m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1] = calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength) - error1 - error2;
    }
  }
  while (noRemaining > noIntervals)
  {
    errorMin = 0; 
    first = 1;
    bestToMerge = 0;
    for (ind = 0; ind < noRemaining - 1; ind++)
    {
      error = error_comb_tab[indexList[ind]];
      if ((error < errorMin || first == 1))
      {
        errorMin = error;
        bestToMerge = ind;
        first = 0;
      }
    }
    ind1 = indexList[bestToMerge];
    ind2 = indexList[bestToMerge+1];
    m_pixAcc_merged[ind1] += m_pixAcc_merged[ind2];
    for (i = 0; i < sqrFiltLength; i++)
    {
      m_y_merged[ind1][i] += m_y_merged[ind2][i];
      for (j = 0; j < sqrFiltLength; j++)
      {
        m_E_merged[ind1][i][j] += m_E_merged[ind2][i][j];
      }
    }
    available[ind2] = 0;
    
    //update error tables
    error_tab[ind1] = error_comb_tab[ind1] + error_tab[ind1] + error_tab[ind2];
    if (indexList[bestToMerge] > 0)
    {
      ind1 = indexList[bestToMerge-1];
      ind2 = indexList[bestToMerge];
      error1 = error_tab[ind1];
      error2 = error_tab[ind2];
      pixAcc_temp = m_pixAcc_merged[ind1] + m_pixAcc_merged[ind2];
      for (i = 0; i < sqrFiltLength; i++)
      {
        m_y_temp[i] = m_y_merged[ind1][i] + m_y_merged[ind2][i];
        for (j = 0; j < sqrFiltLength; j++)
        {
          m_E_temp[i][j] = m_E_merged[ind1][i][j] + m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1] = calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength) - error1 - error2;
    }
    if (indexList[bestToMerge+1] < NO_VAR_BINS - 1)
    {
      ind1 = indexList[bestToMerge];
      ind2 = indexList[bestToMerge+2];
      error1 = error_tab[ind1];
      error2 = error_tab[ind2];
      pixAcc_temp = m_pixAcc_merged[ind1] + m_pixAcc_merged[ind2];
      for (i=0; i<sqrFiltLength; i++)
      {
        m_y_temp[i] = m_y_merged[ind1][i] + m_y_merged[ind2][i];
        for (j=0; j < sqrFiltLength; j++)
        {
          m_E_temp[i][j] = m_E_merged[ind1][i][j] + m_E_merged[ind2][i][j];
        }
      }
      error_comb_tab[ind1] = calculateErrorAbs(m_E_temp, m_y_temp, pixAcc_temp, sqrFiltLength) - error1 - error2;
    }
    
    ind=0;
    for (i = 0; i < NO_VAR_BINS; i++)
    {
      if (available[i] == 1)
      {
        indexList[ind] = i;
        ind++;
      }
    }
    noRemaining--;
  }
  
  errorMin = 0;
  for (ind = 0; ind < noIntervals; ind++)
  {
    errorMin += error_tab[indexList[ind]];
  }
  
  for (ind = 0; ind < noIntervals - 1; ind++)
  {
    intervalBest[ind][0] = indexList[ind]; 
    intervalBest[ind][1] = indexList[ind+1] - 1;
  }
  
  intervalBest[noIntervals-1][0] = indexList[noIntervals-1]; 
  intervalBest[noIntervals-1][1] = NO_VAR_BINS-1;
  
  return(errorMin);
}

Void TEncAdaptiveLoopFilter::roundFiltCoeff(Int *FilterCoeffQuan, Double *FilterCoeff, Int sqrFiltLength, Int factor)
{
  Int i;
  Double diff; 
  Int diffInt, sign; 
  
  for(i = 0; i < sqrFiltLength; i++)
  {
    sign = (FilterCoeff[i] > 0)? 1 : -1; 
    diff = FilterCoeff[i] * sign; 
    diffInt = (Int)(diff * (Double)factor + 0.5); 
    FilterCoeffQuan[i] = diffInt * sign;
  }
}

Double TEncAdaptiveLoopFilter::QuantizeIntegerFilterPP(Double *filterCoeff, Int *filterCoeffQuant, Double **E, Double *y, Int sqrFiltLength, Int *weights)
{
  double error;
  Int factor = (1<<  ((Int)ALF_NUM_BIT_SHIFT)  );
  Int i;
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
  
  checkFilterCoeffValue(filterCoeffQuant, sqrFiltLength, false);

  for (i=0; i<sqrFiltLength; i++)
  {
    filterCoeff[i]=(double)filterCoeffQuant[i]/(double)factor;
  }
  
  error=calculateErrorCoeffProvided(E, y, filterCoeff, sqrFiltLength);
  return(error);
}
Double TEncAdaptiveLoopFilter::findFilterCoeff(double ***EGlobalSeq, double **yGlobalSeq, double *pixAccGlobalSeq, int **filterCoeffSeq, int **filterCoeffQuantSeq, int intervalBest[NO_VAR_BINS][2], int varIndTab[NO_VAR_BINS], int sqrFiltLength, int filters_per_fr, int *weights, double errorTabForce0Coeff[NO_VAR_BINS][2])
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
    errorTabForce0Coeff[filtNo][1] = pixAcc_temp + QuantizeIntegerFilterPP(m_filterCoeff, m_filterCoeffQuant, m_E_temp, m_y_temp, sqrFiltLength, weights);
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

/** Save redesigned filter set to buffer
 * \param filterCoeffPrevSelected filter set buffer
 */
Void TEncAdaptiveLoopFilter::saveFilterCoeffToBuffer(Int **filterSet, Int numFilter, Int* mergeTable, Int mode, Int filtNo)
{
  Int iBufferIndex = m_iCurrentPOC % m_iGOPSize;

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
    if(pbFirstAccess[mode])
    {
      //store merge table
      ::memcpy(m_mergeTableSavedMethods[mode][m_iGOPSize], mergeTable, sizeof(Int)*NO_VAR_BINS);
      //store coefficients
      for(Int varInd=0; varInd< numFilter; varInd++)
      {
        ::memcpy(m_aiFilterCoeffSavedMethods[mode][m_iGOPSize][varInd],filterSet[varInd], sizeof(Int)*ALF_MAX_NUM_COEF );
      }
      //store filter shape
      m_iPreviousFilterShapeMethods[mode][m_iGOPSize]= filtNo; 

      pbFirstAccess[mode] = false;
    }


    //store merge table
    ::memcpy(m_mergeTableSavedMethods[mode][0         ], m_mergeTableSavedMethods[mode][m_iGOPSize], sizeof(Int)*NO_VAR_BINS);
    ::memcpy(m_mergeTableSavedMethods[mode][m_iGOPSize], mergeTable,                          sizeof(Int)*NO_VAR_BINS);

    //store coefficients
    for(Int varInd=0; varInd< NO_VAR_BINS; varInd++)
    {
      ::memcpy(m_aiFilterCoeffSavedMethods[mode][0][varInd],m_aiFilterCoeffSavedMethods[mode][m_iGOPSize][varInd], sizeof(Int)*ALF_MAX_NUM_COEF );
    }

    for(Int varInd=0; varInd< numFilter; varInd++)
    {
      ::memcpy(m_aiFilterCoeffSavedMethods[mode][m_iGOPSize][varInd],filterSet[varInd], sizeof(Int)*ALF_MAX_NUM_COEF );
    }

    //store filter shape
    m_iPreviousFilterShapeMethods[mode][0]= m_iPreviousFilterShapeMethods[mode][m_iGOPSize]; 
    m_iPreviousFilterShapeMethods[mode][m_iGOPSize]= filtNo; 
  }
  else
  {

    //store merge table
    ::memcpy(m_mergeTableSavedMethods[mode][iBufferIndex], mergeTable, sizeof(Int)*NO_VAR_BINS);

    //store coefficients
    for(Int varInd=0; varInd< numFilter; varInd++)
    {
      ::memcpy(m_aiFilterCoeffSavedMethods[mode][iBufferIndex][varInd],filterSet[varInd], sizeof(Int)*ALF_MAX_NUM_COEF );
    }
    //store filter_shape
    m_iPreviousFilterShapeMethods[mode][iBufferIndex]= filtNo; 

  }
}


/** set initial m_maskImg with previous (time-delayed) filters
 * \param pcPicOrg original picture
 * \param pcPicDec reconstructed picture after deblocking
 */
Void TEncAdaptiveLoopFilter::setMaskWithTimeDelayedResults(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec)
{

  static Pel** bestImgMask;
  static Bool bFirst = true;
  if(bFirst)
  {
    initMatrix_Pel(&bestImgMask, m_img_height, m_img_width);
    bFirst = false;
  }

  Pel*    pDec       = pcPicDec->getLumaAddr();
  Pel*    pOrg       = pcPicOrg->getLumaAddr();
  Pel*    pRest      = m_pcPicYuvTmp->getLumaAddr();
  Int     LumaStride = pcPicOrg->getStride();
  Int***   pppCoeffSaved     = m_aiFilterCoeffSavedMethods  [m_uiVarGenMethod];
  Int**    ppMergeTableSaved = m_mergeTableSavedMethods     [m_uiVarGenMethod];
  Int*     pFilterShapeSaved = m_iPreviousFilterShapeMethods[m_uiVarGenMethod];
  Int      iBufIdx;

  UInt64    uiRate, uiDist;
  Double    dCost, dMinCost = MAX_DOUBLE;
  ALFParam  cAlfParam;
  allocALFParam(&cAlfParam);
  cAlfParam.alf_flag        = 0;
  cAlfParam.chroma_idc      = 0;

  //filter frame with the previous time-delayed filters
  Int filtNo;
  Int maxDepth = (pcPicOrg->getWidth() < 1000) ?(2):(g_uiMaxCUDepth);
  m_pcEntropyCoder->setAlfCtrl(true);
  m_pcTempAlfParam->alf_flag = 1;
  m_pcTempAlfParam->alf_pcr_region_flag = m_uiVarGenMethod;

  for (Int index=0; index<2; index++)
  {
    iBufIdx = setFilterIdx(index);
    filtNo = m_pcTempAlfParam->filter_shape = pFilterShapeSaved[iBufIdx];
    assert(filtNo == ALF_STAR5x5 || filtNo == ALF_CROSS9x9);
    m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filtNo]; 
    if(!m_bUseNonCrossALF)
    {
      filterLuma(pRest, pDec, LumaStride, 0, m_img_height-1, 0, m_img_width-1,  filtNo, pppCoeffSaved[iBufIdx], ppMergeTableSaved[iBufIdx], m_varImg);
    }
    else
    {
      xfilterSlicesEncoder(pDec, pRest, LumaStride, filtNo, pppCoeffSaved[iBufIdx], ppMergeTableSaved[iBufIdx], m_varImg);
    }

    for (UInt uiDepth = 0; uiDepth < maxDepth; uiDepth++)
    {
      m_pcEntropyCoder->setMaxAlfCtrlDepth(uiDepth);
      std::vector<AlfCUCtrlInfo> vAlfCUCtrlParamTemp(m_uiNumSlicesInPic);
      xSetCUAlfCtrlFlags_qc(uiDepth, pcPicOrg, pcPicDec, m_pcPicYuvTmp, uiDist, vAlfCUCtrlParamTemp);
      m_pcEntropyCoder->resetEntropy();
      m_pcEntropyCoder->resetBits();
      xEncodeCUAlfCtrlFlags(vAlfCUCtrlParamTemp);
      uiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
      dCost  = (Double)(uiRate) * m_dLambdaLuma + (Double)(uiDist);

      if (dCost < dMinCost)
      {
        dMinCost    = dCost;
        copyALFParam(&cAlfParam, m_pcTempAlfParam);
        ::memcpy(bestImgMask[0], m_maskImg[0], sizeof(Pel)*m_img_height* m_img_width);
      }
    }
  }
  filtNo = cAlfParam.filter_shape;


  ::memcpy(m_maskImg[0], bestImgMask[0], sizeof(Pel)*m_img_height* m_img_width);

  m_pcEntropyCoder->setAlfCtrl(false);
  m_pcEntropyCoder->setMaxAlfCtrlDepth(0);

  // generate filters for future reference
  m_iDesignCurrentFilter = 0; 

  int  filters_per_fr;
  int  lambda_val = (Int)m_dLambdaLuma;

  lambda_val = lambda_val * (1<<(2*g_uiBitIncrement));

  if(!m_bUseNonCrossALF)
  {
    xstoreInBlockMatrix(0, 0, m_img_height, m_img_width, true, true, pOrg, pDec, cAlfParam.filter_shape, LumaStride);
  }
  else
  { 
    xstoreInBlockMatrixforSlices(pOrg, pDec, cAlfParam.filter_shape, LumaStride);
  }
  xfindBestFilterVarPred(m_yGlobalSym[filtNo], m_EGlobalSym[filtNo], m_pixAcc, 
    m_filterCoeffSym, m_filterCoeffSymQuant, 
    filtNo, &filters_per_fr, 
    m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambda_val);

  saveFilterCoeffToBuffer(m_filterCoeffSym, filters_per_fr, m_varIndTab, cAlfParam.alf_pcr_region_flag, filtNo);
  m_iDesignCurrentFilter = 1;

  freeALFParam(&cAlfParam);

}


/** set ALF encoding parameters
 * \param pcPic picture pointer
 */
Void TEncAdaptiveLoopFilter::setALFEncodingParam(TComPic *pcPic)
{
  if(m_iALFEncodePassReduction)
  {
    m_iALFNumOfRedesign = 0;
    m_iCurrentPOC = m_pcPic->getPOC();
    if((pcPic->getSlice(0)->getSliceType() == I_SLICE) || (m_iGOPSize==8 && (m_iCurrentPOC % 4 == 0)))
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

/** set filter buffer index
 * \param index the processing order of time-delayed filtering
 */
Int TEncAdaptiveLoopFilter::setFilterIdx(Int index)
{
  Int iBufIdx;

  if (m_iGOPSize == 8)
  {
    switch(m_iCurrentPOC % m_iGOPSize)
    {
    case 0:
      {
        iBufIdx = (index == 0)?0:m_iGOPSize;
      }
      break;
    case 1:
      {
        iBufIdx = (index == 0)?0:2;
      }
      break;
    case 2:
      {
        iBufIdx = (index == 0)?0:4;
      }
      break;
    case 3:
      {
        iBufIdx = (index == 0)?2:4;
      }
      break;
    case 4:
      {
        iBufIdx = (index == 0)?0:m_iGOPSize;
      }
      break;
    case 5:
      {
        iBufIdx = (index == 0)?4:6;
      }
      break;
    case 6:
      {
        iBufIdx = (index == 0)?4:m_iGOPSize;
      }
      break;
    case 7:
      {
        iBufIdx = (index == 0)?6:m_iGOPSize;
      }
      break;
    default:
      {
        printf("error\n");
        assert(0);
      }
    }
  }
  else
  {
    iBufIdx = (index == 0)?0:m_iGOPSize;
  }

  return iBufIdx;
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
  Pel* pDec = pcPicDec->getLumaAddr();

  calcVar(m_varImg, pDec, LumaStride, m_uiVarGenMethod);

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
    setMaskWithTimeDelayedResults(pcPicOrg, pcPicDec);
  }
}



/** Estimate RD cost of all filter size & store the best one
 * \param ImgOrg original picture
 * \param ImgDec reconstructed picture after deblocking
 * \param Sride  line buffer size of picture buffer
 * \param pcAlfSaved the best Alf parameters 
 * \returns ruiDist             estimated distortion
 * \returns ruiRate             required bits
 * \returns rdCost              estimated R-D cost
 */

Void  TEncAdaptiveLoopFilter::decideFilterShapeLuma(Pel* ImgOrg, Pel* ImgDec, Int Stride, ALFParam* pcAlfSaved, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost)
{
  static Double **ySym, ***ESym;
  Int    lambda_val = ((Int) m_dLambdaLuma) * (1<<(2*g_uiBitIncrement));
  Int    filtNo, filters_per_fr;
  Int64  iEstimatedDist;
  UInt64 uiRate;
  Double dEstimatedCost, dEstimatedMinCost = MAX_DOUBLE;;

  UInt   uiBitShift = (g_uiBitIncrement<<1);
  Int64  iEstimateDistBeforeFilter;
  Int*   coeffNoFilter[NUM_ALF_FILTER_SHAPE][NO_VAR_BINS];
  for(Int filter_shape = 0; filter_shape < NUM_ALF_FILTER_SHAPE; filter_shape++)
  {
    for(Int i=0; i< NO_VAR_BINS; i++)
    {
      coeffNoFilter[filter_shape][i]= new Int[ALF_MAX_NUM_COEF];
      ::memset(coeffNoFilter[filter_shape][i], 0, sizeof(Int)*ALF_MAX_NUM_COEF);
      coeffNoFilter[filter_shape][i][ m_sqrFiltLengthTab[filter_shape]-1 ] = (1 << ((Int)ALF_NUM_BIT_SHIFT));
    }
  }

  m_pcTempAlfParam->alf_flag = 1;
  m_pcTempAlfParam->chroma_idc = 0;
  m_pcTempAlfParam->alf_pcr_region_flag = m_uiVarGenMethod;

  for (int filter_shape = 0; filter_shape < NUM_ALF_FILTER_SHAPE ;filter_shape ++)
  {
    m_pcTempAlfParam->filter_shape = filtNo = filter_shape;
    m_pcTempAlfParam->num_coeff = m_sqrFiltLengthTab[filtNo] ;  

    ESym     = m_EGlobalSym     [filtNo];
    ySym     = m_yGlobalSym     [filtNo];

    if(!m_bUseNonCrossALF)
    {
      xstoreInBlockMatrix(0, 0, m_img_height, m_img_width, true, true, ImgOrg, ImgDec, filter_shape, Stride);
    }
    else
    {
      xstoreInBlockMatrixforSlices(ImgOrg, ImgDec, filter_shape, Stride);
    }
    xfindBestFilterVarPred(ySym, ESym, m_pixAcc, m_filterCoeffSym, m_filterCoeffSymQuant, filtNo, &filters_per_fr, m_varIndTab, NULL, m_varImg, m_maskImg, NULL, lambda_val);

    //estimate R-D cost
    uiRate         = xcodeFiltCoeff(m_filterCoeffSymQuant, filtNo, m_varIndTab, filters_per_fr, m_pcTempAlfParam);
    iEstimatedDist = xEstimateFiltDist(filters_per_fr, m_varIndTab, ESym, ySym, m_filterCoeffSym, m_pcTempAlfParam->num_coeff);
    iEstimateDistBeforeFilter = xEstimateFiltDist(filters_per_fr, m_varIndTab, ESym, ySym, coeffNoFilter[filter_shape], m_pcTempAlfParam->num_coeff);
    iEstimatedDist -= iEstimateDistBeforeFilter;
    dEstimatedCost = (Double)(uiRate) * m_dLambdaLuma + (Double)(iEstimatedDist);

    if(dEstimatedCost < dEstimatedMinCost)
    {
      dEstimatedMinCost   = dEstimatedCost;
      copyALFParam(pcAlfSaved, m_pcTempAlfParam); 
      iEstimatedDist += iEstimateDistBeforeFilter;

      for(Int i=0; i< filters_per_fr; i++ )
      {
        iEstimatedDist += (((Int64)m_pixAcc_merged[i]) >> uiBitShift);
      }
      ruiDist = (iEstimatedDist > 0)?((UInt64)iEstimatedDist):(0);
      rdCost  = dEstimatedMinCost + (Double)(ruiDist);
      ruiRate = uiRate;
    }
  }

  if (!m_iUsePreviousFilter)
  {
    decodeFilterSet(pcAlfSaved, m_varIndTab, m_filterCoeffSym);
    saveFilterCoeffToBuffer(m_filterCoeffSym, pcAlfSaved->filters_per_group, m_varIndTab, pcAlfSaved->alf_pcr_region_flag, pcAlfSaved->filter_shape);
  }

  if( m_iUsePreviousFilter )
  {
    UInt64 uiOffRegionDistortion = 0;
    Int    iPelDiff;
    Pel*   pOrgTemp = (Pel*)ImgOrg;
    Pel*   pDecTemp = (Pel*)ImgDec;
    for(Int y=0; y< m_img_height; y++)
    {
      for(Int x=0; x< m_img_width; x++)
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
  
  // if ALF_STAR5x5 is selected, the distortion of 2 skipped lines per LCU should be added.
  if(pcAlfSaved->filter_shape == ALF_STAR5x5)
  {
    Int    iPelDiff;
    UInt64  uiSkipPelsDistortion = 0;
    Pel   *pOrgTemp, *pDecTemp;
    for(Int y= m_lineIdxPadTop-1; y< m_img_height - m_lcuHeight ; y += m_lcuHeight)
    {
      pOrgTemp = ImgOrg + y*Stride;
      pDecTemp = ImgDec + y*Stride;
      for(Int x=0; x< m_img_width; x++)
      {
        if(m_maskImg[y][x] == 1)
        {
          iPelDiff = pOrgTemp[x] - pDecTemp[x];
          uiSkipPelsDistortion += (UInt64)(  (iPelDiff*iPelDiff) >> uiBitShift );
        }
      }

      pOrgTemp += Stride;
      pDecTemp += Stride;
      for(Int x=0; x< m_img_width; x++)
      {
        if(m_maskImg[y+1][x] == 1)
        {
          iPelDiff = pOrgTemp[x] - pDecTemp[x];
          uiSkipPelsDistortion += (UInt64)(  (iPelDiff*iPelDiff) >> uiBitShift );
        }
      }
    }
    ruiDist += uiSkipPelsDistortion;
    rdCost  += (Double)uiSkipPelsDistortion;
  }

  for(Int filter_shape = 0; filter_shape < NUM_ALF_FILTER_SHAPE; filter_shape++)
  {
    for(Int i=0; i< NO_VAR_BINS; i++)
    {
      delete[] coeffNoFilter[filter_shape][i];
    }
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
  Double pdcoeff[ALF_MAX_NUM_COEF];
  //variable
  Int    i,j;
  Int64  iDist;
  Double dDist, dsum;

  for(i=0; i< iFiltLength; i++)
  {
    pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<< ((Int)ALF_NUM_BIT_SHIFT) );
  }

  dDist =0;
  for(i=0; i< iFiltLength; i++)
  {
    dsum= ((Double)ppdE[i][i]) * pdcoeff[i];
    for(j=i+1; j< iFiltLength; j++)
    {
      dsum += (Double)(2*ppdE[i][j])* pdcoeff[j];
    }

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
      {
        m_E_merged[f][j][i] = 0;
      }

      //clean m_y_merged
      m_y_merged[f][j] = 0;
    }
    m_pixAcc_merged[f] = 0;
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
      {
        ppdDstE[j][i] += ppdSrcE[j][i];
      }

      pdDsty[j] += pdSrcy[j];
    }
    m_pixAcc_merged[ VarIndTab[varInd]  ] += m_pixAcc[varInd];

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

/** Calculate ALF grouping indices for ALF slices
 * \param varmap grouping indices buffer
 * \param imgY_Dec picture buffer
 * \param pad_size (max. filter tap)/2
 * \param fl  VAR_SIZE
 * \param img_stride picture buffer stride
 */
Void TEncAdaptiveLoopFilter::xfilterSlicesEncoder(Pel* ImgDec, Pel* ImgRest, Int iStride, Int filtNo, Int** filterCoeff, Int* mergeTable, Pel** varImg)
{
  Pel* pPicSrc   = (Pel *)ImgDec;
  Pel* pPicSlice = m_pcSliceYuvTmp->getLumaAddr();

  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    if(!m_pcPic->getValidSlice(s)) 
    {
      continue;
    }
    std::vector< std::vector<AlfLCUInfo*> > & vpSliceTileAlfLCU = m_pvpSliceTileAlfLCU[s];

    for(Int t=0; t< (Int)vpSliceTileAlfLCU.size(); t++)
    {
      std::vector<AlfLCUInfo*> & vpAlfLCU = vpSliceTileAlfLCU[t];
      copyRegion(vpAlfLCU, pPicSlice, pPicSrc, iStride);
      extendRegionBorder(vpAlfLCU, pPicSlice, iStride);
      filterLumaRegion(vpAlfLCU, pPicSlice, ImgRest, iStride, filtNo, filterCoeff, mergeTable, varImg);
    }
  }
}

/** Calculate block autocorrelations and crosscorrelations for ALF slices
 * \param ImgOrg original picture
 * \param ImgDec picture before filtering
 * \param tap  filter tap size
 * \param iStride picture buffer stride
 */
Void   TEncAdaptiveLoopFilter::xstoreInBlockMatrixforSlices(Pel* ImgOrg, Pel* ImgDec, Int tap, Int iStride)
{
  Pel* pPicSrc   = (Pel *)ImgDec;
  Pel* pPicSlice = m_pcSliceYuvTmp->getLumaAddr();

  UInt iLastValidSliceID =0;
  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    if(m_pcPic->getValidSlice(s))
    {
      iLastValidSliceID = s;
    }
  }

  for(UInt s=0; s<= iLastValidSliceID; s++)
  {
    if(!m_pcPic->getValidSlice(s))
    {
      continue;
    }
    std::vector< std::vector<AlfLCUInfo*> > & vpSliceTileAlfLCU = m_pvpSliceTileAlfLCU[s];
    Int numValidTilesInSlice = (Int)vpSliceTileAlfLCU.size();
    for(Int t=0; t< numValidTilesInSlice; t++)
    {
      std::vector<AlfLCUInfo*> & vpAlfLCU = vpSliceTileAlfLCU[t];
      copyRegion(vpAlfLCU, pPicSlice, pPicSrc, iStride);
      extendRegionBorder(vpAlfLCU, pPicSlice, iStride);
      xstoreInBlockMatrixforRegion(vpAlfLCU, ImgOrg, pPicSlice, tap, iStride, (s==0)&&(t==0), (s== iLastValidSliceID)&&(t==numValidTilesInSlice-1));
    }
  }
}

/** Calculate block autocorrelations and crosscorrelations for one ALF region
 * \param vpAlfLCU ALF LCU data container
 * \param ImgOrg original picture
 * \param ImgDec picture before filtering
 * \param tap  filter tap size
 * \param iStride picture buffer stride
 * \param bFirstSlice  true for the first processing slice of the picture
 * \param bLastSlice true for the last processing slice of the picture
 */
Void   TEncAdaptiveLoopFilter::xstoreInBlockMatrixforRegion(std::vector< AlfLCUInfo* > &vpAlfLCU, 
                                                              Pel* ImgOrg, Pel* ImgDec, 
                                                              Int tap, Int iStride, 
                                                              Bool bFirstSlice, 
                                                              Bool bLastSlice
                                                              )
{

  UInt uiNumLCUs = (UInt)vpAlfLCU.size();
  Int iHeight, iWidth;
  Int ypos, xpos;
  Bool bFirstLCU, bLastLCU;
  Bool bFirstSGU, bLastSGU;
  UInt numSGUs;

  for(UInt i=0; i< uiNumLCUs; i++)
  {
    bFirstLCU = (i==0);
    bLastLCU  = (i== uiNumLCUs -1);
    AlfLCUInfo& cAlfLCU = *(vpAlfLCU[i]); 
    numSGUs = cAlfLCU.numSGU;
    for(UInt j=0; j< numSGUs; j++)
    {
      bFirstSGU= (j ==0);
      bLastSGU = (j == numSGUs -1);

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
  Int chromaFormatShift = 1;

  UInt iLastValidSliceID =0;
  for(UInt s=0; s< m_uiNumSlicesInPic; s++)
  {
    if(m_pcPic->getValidSlice(s))
    {
      iLastValidSliceID = s;
    }
  }

  for(UInt s=0; s<= iLastValidSliceID; s++)
  {
    if(!m_pcPic->getValidSlice(s))
    {
      continue;
    }
    std::vector< std::vector<AlfLCUInfo*> > & vpSliceTileAlfLCU = m_pvpSliceTileAlfLCU[s];
    Int numValidTilesInSlice = (Int)vpSliceTileAlfLCU.size();
    for(Int t=0; t< numValidTilesInSlice; t++)
    {
      std::vector<AlfLCUInfo*> & vpAlfLCU = vpSliceTileAlfLCU[t];
      copyRegion(vpAlfLCU, pPicSlice, pPicSrc, iCmpStride, chromaFormatShift);
      extendRegionBorder(vpAlfLCU, pPicSlice, iCmpStride, chromaFormatShift);
      xCalcCorrelationFuncforChromaRegion(vpAlfLCU, pOrg, pPicSlice, iTap, iCmpStride,(s== iLastValidSliceID)&&(t== numValidTilesInSlice-1), chromaFormatShift);
    }
  }
}

/** Calculate autocorrelations and crosscorrelations for one chroma slice
 * \param vpAlfLCU ALF LCU data container
 * \param pOrg original picture
 * \param pCmp picture before filtering
 * \param iTap  filter tap size
 * \param iStride picture buffer stride
 * \param bLastSlice the last processing slice of picture
 */
Void TEncAdaptiveLoopFilter::xCalcCorrelationFuncforChromaRegion(std::vector< AlfLCUInfo* > &vpAlfLCU, Pel* pOrg, Pel* pCmp, Int filtNo, Int iStride, Bool bLastSlice, Int iFormatShift)
{
  UInt uiNumLCUs = (UInt)vpAlfLCU.size();

  Int iHeight, iWidth;
  Int ypos, xpos;
  Bool bLastLCU;
  Bool bLastSGU;
  UInt numSGUs;

  for(UInt i=0; i< uiNumLCUs; i++)
  {
    bLastLCU  = (i== uiNumLCUs -1);

    AlfLCUInfo& cAlfLCU = *(vpAlfLCU[i]); 
    numSGUs = cAlfLCU.numSGU;
    for(UInt j=0; j< numSGUs; j++)
    {
      bLastSGU = (j == numSGUs -1);
      ypos    = (Int)(cAlfLCU[j].posY   >> iFormatShift);
      xpos    = (Int)(cAlfLCU[j].posX   >> iFormatShift);
      iHeight = (Int)(cAlfLCU[j].height >> iFormatShift);
      iWidth  = (Int)(cAlfLCU[j].width  >> iFormatShift);
      xCalcCorrelationFunc(ypos, xpos, pOrg, pCmp, filtNo, iWidth, iHeight, iStride, iStride, (bLastSlice && bLastLCU && bLastSGU) );
    }
  }
}

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
    m_pcTempAlfParam->filter_shape_chroma = iShape;
    m_pcTempAlfParam->num_coeff_chroma = num_coeff;

    // keep original corr pointer
    Double **ppdTmpCorr = m_ppdAlfCorr;

    // calc Cb matrix
    m_pcTempAlfParam->chroma_idc = 2;
    m_ppdAlfCorr = m_ppdAlfCorrCb;
    for(Int i=0; i<ALF_MAX_NUM_COEF; i++)
    {
      ::memset(m_ppdAlfCorr[i], 0, sizeof(Double) * (ALF_MAX_NUM_COEF + 1));
    }
    Pel *pOrg = pcPicOrg->getCbAddr();
    Pel *pCmp = pcPicDec->getCbAddr();
    if(!m_bUseNonCrossALF)
    {
      xCalcCorrelationFunc(0, 0, pOrg, pCmp, iShape, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride(), true);
    }
    else
    {
      xCalcCorrelationFuncforChromaSlices(ALF_Cb, pOrg, pCmp, iShape, pcPicOrg->getCStride(), pcPicDec->getCStride());
    }

    // calc Cr matrix
    m_pcTempAlfParam->chroma_idc = 1;
    m_ppdAlfCorr = m_ppdAlfCorrCr;
    for(Int i=0; i<ALF_MAX_NUM_COEF; i++)
    {
      ::memset(m_ppdAlfCorr[i], 0, sizeof(Double) * (ALF_MAX_NUM_COEF + 1));
    }
    pOrg = pcPicOrg->getCrAddr();
    pCmp = pcPicDec->getCrAddr();
    if(!m_bUseNonCrossALF)
    {
      xCalcCorrelationFunc(0, 0, pOrg, pCmp, iShape, (pcPicOrg->getWidth()>>1), (pcPicOrg->getHeight()>>1), pcPicOrg->getCStride(), pcPicDec->getCStride(), true);
    }
    else
    {
      xCalcCorrelationFuncforChromaSlices(ALF_Cr, pOrg, pCmp, iShape, pcPicOrg->getCStride(), pcPicDec->getCStride());
    }

    // restore original corr pointer
    m_ppdAlfCorr = ppdTmpCorr;

    // calc original dist
    memset(qh, 0, sizeof(Int)*num_coeff);
    qh[num_coeff-1] = 1<<((Int)ALF_NUM_BIT_SHIFT);
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
  xFilteringFrameChroma(m_pcTempAlfParam, pcPicOrg, pcPicDec, pcPicRest);

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
      {
        pcPicDec->copyToPicCb(pcPicRest);
      }
      if(m_pcTempAlfParam->chroma_idc&0x01)
      {
        pcPicDec->copyToPicCr(pcPicRest);
      }
      
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
  Double pdcoeff[ALF_MAX_NUM_COEF];
  Int    i,j;
  Int64  iDist;
  Double dDist, dsum;
  for(i=0; i< iSqrFiltLength; i++)
  {
    pdcoeff[i]= (Double)piCoeff[i] / (Double)(1<< ((Int)ALF_NUM_BIT_SHIFT) );
  }

  dDist =0;
  for(i=0; i< iSqrFiltLength; i++)
  {
    dsum= ((Double)ppdCorr[i][i]) * pdcoeff[i];
    for(j=i+1; j< iSqrFiltLength; j++)
    {
      dsum += (Double)(2*ppdCorr[i][j])* pdcoeff[j];
    }

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
  Int* piTmpCoef;
  piTmpCoef = new Int[ALF_MAX_NUM_COEF];
  memcpy(piTmpCoef, pAlfParam->coeff_chroma, sizeof(Int)*pAlfParam->num_coeff_chroma);

  predictALFCoeffChroma(pAlfParam);

  m_pcEntropyCoder->resetEntropy();
  m_pcEntropyCoder->resetBits();
  m_pcEntropyCoder->encodeAlfParam(pAlfParam);
  uiRate = m_pcEntropyCoder->getNumberOfWrittenBits();
  if (m_vBestAlfCUCtrlParam.size() != 0)
  {
    for(UInt s=0; s< m_uiNumSlicesInPic; s++)
    {
      if(!m_pcPic->getValidSlice(s))
      {
        continue;
      }
      m_pcEntropyCoder->resetEntropy();
      m_pcEntropyCoder->resetBits();
      m_pcEntropyCoder->encodeAlfCtrlParam( m_vBestAlfCUCtrlParam[s], m_uiNumCUsInFrame);
      uiRate += m_pcEntropyCoder->getNumberOfWrittenBits();
    }
  }
  else
  {
    uiRate += m_uiNumSlicesInPic;
  }
  memcpy(pAlfParam->coeff_chroma, piTmpCoef, sizeof(int)*pAlfParam->num_coeff_chroma);
  delete[] piTmpCoef;
  piTmpCoef = NULL;

  return uiRate;
}

//! \}
