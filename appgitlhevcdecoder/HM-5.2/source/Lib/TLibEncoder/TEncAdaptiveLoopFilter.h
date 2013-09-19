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

/** \file     TEncAdaptiveLoopFilter.h
 \brief    estimation part of adaptive loop filter class (header)
 */

#ifndef __TENCADAPTIVELOOPFILTER__
#define __TENCADAPTIVELOOPFILTER__

#include "TLibCommon/TComAdaptiveLoopFilter.h"
#include "TLibCommon/TComPic.h"

#include "TEncEntropy.h"
#include "TEncSbac.h"
#include "TLibCommon/TComBitCounter.h"

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// estimation part of adaptive loop filter class
class TEncAdaptiveLoopFilter : public TComAdaptiveLoopFilter
{
private:
  ///
  /// variables for correlation calculation
  ///
  Double** m_ppdAlfCorr;
  Double* m_pdDoubleAlfCoeff;
  Double** m_ppdAlfCorrCb;
  Double** m_ppdAlfCorrCr;
  double ***m_yGlobalSym;
  double ****m_EGlobalSym;
  double *m_pixAcc;
  double **m_y_merged;
  double ***m_E_merged;
  double *m_pixAcc_merged;
  double *m_y_temp;
  double **m_E_temp;

  ///
  /// ALF parameters
  ///
  ALFParam* m_pcBestAlfParam;
  ALFParam* m_pcTempAlfParam;
  ALFParam* pcAlfParamShape0;
  ALFParam* pcAlfParamShape1;
  ALFParam *m_tempALFp;

  ///
  /// temporary picture buffers or pointers
  ///
  TComPicYuv* m_pcPicYuvBest;
  TComPicYuv* m_pcPicYuvTmp;
  TComPicYuv* pcPicYuvRecShape0;
  TComPicYuv* pcPicYuvRecShape1;

  ///
  /// temporary filter buffers or pointers
  ///
  Int    *m_filterCoeffQuantMod;
  double *m_filterCoeff;
  Int    *m_filterCoeffQuant;
  Int    **m_filterCoeffSymQuant;
  Int    **m_diffFilterCoeffQuant;
  Int    **m_FilterCoeffQuantTemp;
  Int**  m_mergeTableSavedMethods[NUM_ALF_CLASS_METHOD];
  Int*** m_aiFilterCoeffSavedMethods[NUM_ALF_CLASS_METHOD];  //!< time-delayed filter set buffer
  Int*   m_iPreviousFilterShapeMethods[NUM_ALF_CLASS_METHOD];

  ///
  /// coding control parameters
  ///
  Double m_dLambdaLuma;
  Double m_dLambdaChroma;
  Int  m_iUsePreviousFilter;     //!< for N-pass encoding- 1: time-delayed filtering is allowed. 0: not allowed.
  Int  m_iDesignCurrentFilter;   //!< for N-pass encoding- 1: design filters for current slice. 0: design filters for future slice reference
  Int  m_iGOPSize;                //!< GOP size
  Int  m_iCurrentPOC;             //!< POC
  Int  m_iALFEncodePassReduction; //!< 0: 16-pass encoding, 1: 1-pass encoding, 2: 2-pass encoding

  Int  m_iALFMaxNumberFilters;    //!< ALF Max Number Filters per unit

  Int  m_iALFNumOfRedesign;       //!< number of redesigning filter for each CU control depth

  ///
  /// variables for on/off control
  ///
  Pel **m_maskImg;
  Bool m_bAlfCUCtrlEnabled;                         //!< if input ALF CU control param is NULL, this variable is set to be false (Disable CU control)
  std::vector<AlfCUCtrlInfo> m_vBestAlfCUCtrlParam; //!< ALF CU control parameters container to store the ALF CU control parameters after RDO

  ///
  /// miscs. 
  ///
  TEncEntropy* m_pcEntropyCoder;
private:
  // init / uninit internal variables
  Void xInitParam      ();
  Void xUninitParam    ();

  // ALF on/off control related functions
  Void xCreateTmpAlfCtrlFlags   ();
  Void xDestroyTmpAlfCtrlFlags  ();
  Void xCopyTmpAlfCtrlFlagsTo   ();
  Void xCopyTmpAlfCtrlFlagsFrom ();
  Void getCtrlFlagsFromCU(AlfLCUInfo* pcAlfLCU, std::vector<UInt> *pvFlags, Int iAlfDepth, UInt uiMaxNumSUInLCU);
  Void xEncodeCUAlfCtrlFlags  (std::vector<AlfCUCtrlInfo> &vAlfCUCtrlParam);
  Void xEncodeCUAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
  Void xCUAdaptiveControl_qc           ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost );
  Void xSetCUAlfCtrlFlags_qc            (UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam);
  Void xSetCUAlfCtrlFlag_qc             (TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiAlfCtrlDepth, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, std::vector<UInt>& vCUCtrlFlag);

  // functions related to correlation computation
  Void xstoreInBlockMatrix(Int ypos, Int xpos, Int iheight, Int iwidth, Bool bResetBlockMatrix, Bool bSymmCopyBlockMatrix, Pel* pImgOrg, Pel* pImgPad, Int filtNo, Int stride); //!< Calculate correlations for luma
  Void xstoreInBlockMatrixforRegion(std::vector< AlfLCUInfo* > &vpAlfLCU, Pel* ImgOrg, Pel* ImgDec, Int tap, Int iStride, Bool bFirstSlice, Bool bLastSlice); //!< Calculate block autocorrelations and crosscorrelations for one ALF slice
  Void xstoreInBlockMatrixforSlices  (Pel* ImgOrg, Pel* ImgDec, Int tap, Int iStride); //!< Calculate block autocorrelations and crosscorrelations for ALF slices
  Void xCalcCorrelationFunc(Int ypos, Int xpos, Pel* pImgOrg, Pel* pImgPad, Int filtNo, Int iWidth, Int iHeight, Int iOrgStride, Int iCmpStride, Bool bSymmCopyBlockMatrix); //!< Calculate correlations for chroma
  Void xCalcCorrelationFuncforChromaRegion(std::vector< AlfLCUInfo* > &vpAlfLCU, Pel* pOrg, Pel* pCmp, Int filtNo, Int iStride, Bool bLastSlice, Int iFormatShift); //!< Calculate autocorrelations and crosscorrelations for one chroma slice
  Void xCalcCorrelationFuncforChromaSlices  (Int ComponentID, Pel* pOrg, Pel* pCmp, Int iTap, Int iOrgStride, Int iCmpStride); //!< Calculate autocorrelations and crosscorrelations for chroma slices
  // functions related to filtering
  Void xFilterCoefQuickSort   ( Double *coef_data, Int *coef_num, Int upper, Int lower );
  Void xQuantFilterCoef       ( Double* h, Int* qh, Int tap, int bit_depth );
  Void xClearFilterCoefInt    ( Int* qh, Int N );
  Void xCopyDecToRestCUs      ( TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void xCopyDecToRestCU       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void xFilteringFrameChroma  (ALFParam* pcAlfParam, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest);
  Int  setFilterIdx(Int index); //!< set filter buffer index
  Void setInitialMask(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec); //!< set initial m_maskImg
  Void saveFilterCoeffToBuffer(Int **filterSet, Int numFilter, Int* mergeTable, Int mode, Int filtNo); //!< save filter coefficients to buffer
  Void setMaskWithTimeDelayedResults(TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec); //!< set initial m_maskImg with previous (time-delayed) filters
  Void decideFilterShapeLuma(Pel* ImgOrg, Pel* ImgDec, Int Stride, ALFParam* pcAlfSaved, UInt64& ruiRate, UInt64& ruiDist,Double& rdCost); //!< Estimate RD cost of all filter size & store the best one
  Void   xFilterTapDecisionChroma      (UInt64 uiLumaRate, TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiDist, UInt64& ruiBits);
  Int64  xFastFiltDistEstimationChroma (Double** ppdCorr, Int* piCoeff, Int iSqrFiltLength);
  Void  xfilterSlicesEncoder(Pel* ImgDec, Pel* ImgRest, Int iStride, Int filtNo, Int** filterCoeff, Int* mergeTable, Pel** varImg); //!< Calculate ALF grouping indices for ALF slices
  Void  setALFEncodingParam(TComPic *pcPic); //!< set ALF encoding parameters
  Void xReDesignFilterCoeff_qc          (TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec,  TComPicYuv* pcPicRest, Bool bReadCorr);
  Void xFirstFilteringFrameLuma (Pel* imgOrg, Pel* imgDec, Pel* imgRest, ALFParam* ALFp, Int filtNo, Int stride);
  Void xFilteringFrameLuma(Pel* imgOrg, Pel* imgPad, Pel* imgFilt, ALFParam* ALFp, Int filtNo, Int stride);
  Void xEncALFLuma  ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest, UInt64& ruiMinRate, UInt64& ruiMinDist, Double& rdMinCost ); //!< estimate adaptation mode and filter shape

  // distortion / misc functions
  UInt64 xCalcSSD             ( Pel* pOrg, Pel* pCmp, Int iWidth, Int iHeight, Int iStride );
  Void  xCalcRDCost          ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost, std::vector<AlfCUCtrlInfo>* pvAlfCUCtrlParam = NULL);
  Void  xCalcRDCost          ( ALFParam* pAlfParam, UInt64& ruiRate, UInt64 uiDist, Double& rdCost, std::vector<AlfCUCtrlInfo>* pvAlfCUCtrlParam = NULL);
  Void  xCalcRDCostChroma    ( TComPicYuv* pcPicOrg, TComPicYuv* pcPicCmp, ALFParam* pAlfParam, UInt64& ruiRate, UInt64& ruiDist, Double& rdCost );
  Int64 xFastFiltDistEstimation(Double** ppdE, Double* pdy, Int* piCoeff, Int iFiltLength); //!< Estimate filtering distortion by correlation values and filter coefficients
  Int64 xEstimateFiltDist      (Int filters_per_fr, Int* VarIndTab, Double*** pppdE, Double** ppdy, Int** ppiCoeffSet, Int iFiltLength); //!< Estimate total filtering cost of all groups  
  UInt64 xCalcRateChroma               (ALFParam* pAlfParam);
  Void   xCalcALFCoeffChroma           (Int iChromaIdc, Int iShape, Int* piCoeff);


  /// code filter coefficients
  UInt xcodeFiltCoeff(Int **filterCoeffSymQuant, Int filter_shape, Int varIndTab[], Int filters_per_fr_best, ALFParam* ALFp);
  Void xfindBestFilterVarPred(double **ySym, double ***ESym, double *pixAcc, Int **filterCoeffSym, Int **filterCoeffSymQuant,Int filter_shape, Int *filters_per_fr_best, Int varIndTab[], Pel **imgY_rec, Pel **varImg, Pel **maskImg, Pel **imgY_pad, double lambda_val);
  double xfindBestCoeffCodMethod(int **filterCoeffSymQuant, int filter_shape, int sqrFiltLength, int filters_per_fr, double errorForce0CoeffTab[NO_VAR_BINS][2], double lambda);
  Int xsendAllFiltersPPPred(int **FilterCoeffQuant, int filter_shape, int sqrFiltLength, int filters_per_group, int createBistream, ALFParam* ALFp);
  Int xcodeAuxInfo(int filters_per_fr, int varIndTab[NO_VAR_BINS], int filter_shape, ALFParam* ALFp);
  Int xcodeFilterCoeff(int **pDiffQFilterCoeffIntPP, int filter_shape, int sqrFiltLength, int filters_per_group, int createBitstream);
  Int lengthGolomb(int coeffVal, int k);
  Int lengthPredFlags(int force0, int predMethod, int codedVarBins[NO_VAR_BINS], int filters_per_group, int createBitstream);
  Int lengthFilterCoeffs(int sqrFiltLength, int filters_per_group, int pDepthInt[], int **FilterCoeff, int kMinTab[], int createBitstream);
  Void predictALFCoeffLumaEnc(ALFParam* pcAlfParam, Int **pfilterCoeffSym, Int filter_shape); //!< prediction of luma ALF coefficients
  //cholesky related
  Int   xGauss( Double **a, Int N );
  Double findFilterCoeff(double ***EGlobalSeq, double **yGlobalSeq, double *pixAccGlobalSeq, int **filterCoeffSeq,int **filterCoeffQuantSeq, int intervalBest[NO_VAR_BINS][2], int varIndTab[NO_VAR_BINS], int sqrFiltLength, int filters_per_fr, int *weights, double errorTabForce0Coeff[NO_VAR_BINS][2]);
  Double QuantizeIntegerFilterPP(Double *filterCoeff, Int *filterCoeffQuant, Double **E, Double *y, Int sqrFiltLength, Int *weights);
  Void roundFiltCoeff(int *FilterCoeffQuan, double *FilterCoeff, int sqrFiltLength, int factor);
  double mergeFiltersGreedy(double **yGlobalSeq, double ***EGlobalSeq, double *pixAccGlobalSeq, int intervalBest[NO_VAR_BINS][2], int sqrFiltLength, int noIntervals);
  double calculateErrorAbs(double **A, double *b, double y, int size);
  double calculateErrorCoeffProvided(double **A, double *b, double *c, int size);
  Void add_b(double *bmerged, double **b, int start, int stop, int size);
  Void add_A(double **Amerged, double ***A, int start, int stop, int size);
  Int  gnsSolveByChol(double **LHS, double *rhs, double *x, int noEq);
  Void gnsBacksubstitution(double R[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], double z[ALF_MAX_NUM_COEF], int R_size, double A[ALF_MAX_NUM_COEF]);
  Void gnsTransposeBacksubstitution(double U[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], double rhs[], double x[],int order);
  Int  gnsCholeskyDec(double **inpMatr, double outMatr[ALF_MAX_NUM_COEF][ALF_MAX_NUM_COEF], int noEq);

public:
  TEncAdaptiveLoopFilter          ();
  virtual ~TEncAdaptiveLoopFilter () {}

  Void startALFEnc(TComPic* pcPic, TEncEntropy* pcEntropyCoder); //!< allocate temporal memory
  Void endALFEnc(); //!< destroy temporal memory

#if ALF_CHROMA_LAMBDA  
  Void ALFProcess(ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>* pvAlfCtrlParam, Double dLambdaLuma, Double dLambdaChroma, UInt64& ruiDist, UInt64& ruiBits); //!< estimate ALF parameters
#else
  Void ALFProcess(ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>* pvAlfCtrlParam, Double dLambda, UInt64& ruiDist, UInt64& ruiBits); //!< estimate ALF parameters
#endif

  Void setGOPSize(Int val) { m_iGOPSize = val; } //!< set GOP size
  Void setALFEncodePassReduction (Int iVal) {m_iALFEncodePassReduction = iVal;} //!< set N-pass encoding. 0: 16(14)-pass encoding, 1: 1-pass encoding, 2: 2-pass encoding

  Void setALFMaxNumberFilters    (Int iVal) {m_iALFMaxNumberFilters = iVal;} //!< set ALF Max Number of Filters

  Void createAlfGlobalBuffers(Int iALFEncodePassReduction); //!< create ALF global buffers
  Void destroyAlfGlobalBuffers(); //!< destroy ALF global buffers
  Void PCMLFDisableProcess (TComPic* pcPic);
};

//! \}

#endif
