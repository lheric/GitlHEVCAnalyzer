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

/** \file     TComAdaptiveLoopFilter.h
    \brief    adaptive loop filter class (header)
*/

#ifndef __TCOMADAPTIVELOOPFILTER__
#define __TCOMADAPTIVELOOPFILTER__

#include "CommonDef.h"
#include "TComPic.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constants
// ====================================================================================================================

#define ALF_MAX_NUM_COEF      9                                       //!< maximum number of filter coefficients
#define MAX_SQR_FILT_LENGTH   41                                      //!< ((max_horizontal_tap * max_vertical_tap) / 2 + 1) = ((11 * 5) / 2 + 1)

#define ALF_NUM_BIT_SHIFT     8                                       ///< bit shift parameter for quantization of ALF param.

#define VAR_SIZE_H            4
#define VAR_SIZE_W            4
#define NO_VAR_BINS           16 
#define NO_FILTERS            16
#define MAX_SCAN_VAL          13
#define MAX_EXP_GOLOMB        16

///
/// Chroma component ID
///
enum AlfChromaID
{
  ALF_Cb = 0,
  ALF_Cr = 1
};

///
/// Adaptation mode ID
///
enum ALFClassficationMethod
{
  ALF_BA =0,
  ALF_RA,
  NUM_ALF_CLASS_METHOD
};

///
/// Filter shape
///
enum ALFFilterShape
{
  ALF_STAR5x5 = 0,
  ALF_CROSS9x9,
  NUM_ALF_FILTER_SHAPE
};

extern Int depthIntShape0Sym[10];
extern Int depthIntShape1Sym[10];
extern Int *pDepthIntTabShapes[NUM_ALF_FILTER_SHAPE];

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// ALF CU control parameters
struct AlfCUCtrlInfo
{
  Int  cu_control_flag;                    //!< slice-level ALF CU control enabled/disabled flag 
  UInt num_alf_cu_flag;                    //!< number of ALF CU control flags
  UInt alf_max_depth;                      //!< ALF CU control depth
  std::vector<UInt> alf_cu_flag;           //!< ALF CU control flags (container)

  const AlfCUCtrlInfo& operator= (const AlfCUCtrlInfo& src);  //!< "=" operator
  AlfCUCtrlInfo():cu_control_flag(0), num_alf_cu_flag(0), alf_max_depth(0) {} //!< constructor

};


///
/// LCU-based ALF processing info
///
struct AlfLCUInfo
{
  TComDataCU* pcCU;            //!< TComDataCU pointer
  Int         sliceID;        //!< slice ID
  Int         tileID;         //!< tile ID
  UInt        numSGU;        //!< number of slice granularity blocks 
  UInt        startSU;       //!< starting SU z-scan address in LCU
  UInt        endSU;         //!< ending SU z-scan address in LCU
  Bool        bAllSUsInLCUInSameSlice; //!< true: all SUs in this LCU belong to same slice
  std::vector<NDBFBlockInfo*> vpAlfBlock; //!< container for filter block pointers

  NDBFBlockInfo& operator[] (Int idx) { return *( vpAlfBlock[idx]); } //!< [] operator
  AlfLCUInfo():pcCU(NULL), sliceID(0), tileID(0), numSGU(0), startSU(0), endSU(0), bAllSUsInLCUInSameSlice(false) {} //!< constructor
};


///
/// adaptive loop filter class
///
class TComAdaptiveLoopFilter
{

protected: //protected member variables

  // filter shape information
  static Int weightsShape0Sym[10];
  static Int weightsShape1Sym[10];
  static Int *weightsTabShapes[NUM_ALF_FILTER_SHAPE];
  static Int m_sqrFiltLengthTab[NUM_ALF_FILTER_SHAPE];

  // temporary buffer
  TComPicYuv*   m_pcTempPicYuv;                          ///< temporary picture buffer for ALF processing
  TComPicYuv* m_pcSliceYuvTmp;    //!< temporary picture buffer pointer when non-across slice boundary ALF is enabled


  //filter coefficients buffer
  Int **m_filterCoeffSym;

  //classification
  Int      m_varIndTab[NO_VAR_BINS];
  UInt     m_uiVarGenMethod;
  Pel** m_varImgMethods[NUM_ALF_CLASS_METHOD];
  Pel** m_varImg;

  //parameters
  Int   m_img_height;
  Int   m_img_width;
  Bool  m_bUseNonCrossALF;       //!< true for performing non-cross slice boundary ALF
  UInt  m_uiNumSlicesInPic;      //!< number of slices in picture
  Int   m_iSGDepth;              //!< slice granularity depth
  UInt  m_uiNumCUsInFrame;

  Int m_lcuHeight;
  Int m_lineIdxPadBot;
  Int m_lineIdxPadTop;

  Int m_lcuHeightChroma;
  Int m_lineIdxPadBotChroma;
  Int m_lineIdxPadTopChroma;

  //slice
  TComPic* m_pcPic;
  AlfLCUInfo** m_ppSliceAlfLCUs;
  std::vector< AlfLCUInfo* > *m_pvpAlfLCU;
  std::vector< std::vector< AlfLCUInfo* > > *m_pvpSliceTileAlfLCU;

private: //private member variables


protected: //protected methods

  Void InitAlfLCUInfo(AlfLCUInfo& rAlfLCU, Int sliceID, Int tileID, TComDataCU* pcCU, UInt maxNumSUInLCU);

  Void createRegionIndexMap(Pel **imgY_var, Int img_width, Int img_height); //!< create RA index for regions
  Void calcVar(Pel **imgYvar, Pel *imgYpad, Int stride, Int adaptationMode); //!< Calculate ALF grouping indices for block-based (BA) mode
  Void filterLuma(Pel *pImgYRes, Pel *pImgYPad, Int stride, Int ypos, Int yposEnd, Int xpos, Int xposEnd, Int filtNo, Int** filterSet, Int* mergeTable, Pel** ppVarImg); //!< filtering operation for luma region
  Void filterChroma(Pel *pImgRes, Pel *pImgPad, Int stride, Int ypos, Int yposEnd, Int xpos, Int xposEnd, Int filtNo, Int* coef);
  Void filterChromaRegion(std::vector<AlfLCUInfo*> &vpAlfLCU, Pel* pDec, Pel* pRest, Int stride, Int *coeff, Int filtNo, Int chromaFormatShift); //!< filtering operation for chroma region
  Void xCUAdaptive   (TComPic* pcPic, Int filtNo, Pel *imgYFilt, Pel *imgYRec, Int Stride);
  Void xSubCUAdaptive(TComDataCU* pcCU, Int filtNo, Pel *imgYFilt, Pel *imgYRec, UInt uiAbsPartIdx, UInt uiDepth, Int Stride);
  Void reconstructFilterCoeffs(ALFParam* pcAlfParam,int **pfilterCoeffSym);
  Void predictALFCoeffLuma  ( ALFParam* pAlfParam );                    //!< prediction of luma ALF coefficients
  Void checkFilterCoeffValue( Int *filter, Int filterLength, Bool isChroma );
  Void decodeFilterSet(ALFParam* pcAlfParam, Int* varIndTab, Int** filterCoeff);
  Void xALFChroma   ( ALFParam* pcAlfParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void xFilterChromaSlices(Int componentID, TComPicYuv* pcPicDecYuv, TComPicYuv* pcPicRestYuv, Int *coeff, Int filtNo, Int chromaFormatShift);
  Void xFilterChromaOneCmp(Int componentID, TComPicYuv *pDecYuv, TComPicYuv *pRestYuv, Int shape, Int *pCoeff);
  Void xALFLuma( TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam, TComPicYuv* pcPicDec, TComPicYuv* pcPicRest );
  Void setAlfCtrlFlags(AlfCUCtrlInfo* pAlfParam, TComDataCU *pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt &idx);
  Void transferCtrlFlagsFromAlfParam(std::vector<AlfCUCtrlInfo>& vAlfParamSlices); //!< Copy ALF CU control flags from ALF parameters for slices  
  Void transferCtrlFlagsFromAlfParamOneSlice(std::vector<AlfLCUInfo*> &vpAlfLCU, Bool bCUCtrlEnabled, Int iAlfDepth, std::vector<UInt>& vCtrlFlags); //!< Copy ALF CU control flags from ALF parameter for one slice
  Void extendBorderCoreFunction(Pel* pPel, Int stride, Bool* pbAvail, UInt width, UInt height, UInt extSize); //!< Extend slice boundary border  
  Void copyRegion(std::vector<AlfLCUInfo*> &vpAlfLCU, Pel* pPicDst, Pel* pPicSrc, Int stride, Int formatShift = 0);
  Void extendRegionBorder(std::vector<AlfLCUInfo*> &vpAlfLCU, Pel* pPelSrc, Int stride, Int formatShift = 0);
  Void filterLumaRegion (std::vector<AlfLCUInfo*> &vpAlfLCU, Pel* imgDec, Pel* imgRest, Int stride, Int filtNo, Int** filterCoeff, Int* mergeTable, Pel** varImg);
  Void xCUAdaptiveRegion(std::vector<AlfLCUInfo*> &vpAlfLCU, Pel* imgDec, Pel* imgRest, Int stride, Int filtNo, Int** filterCoeff, Int* mergeTable, Pel** varImg);
  Int  getCtrlFlagsFromAlfParam(AlfLCUInfo* pcAlfLCU, Int iAlfDepth, UInt* puiFlags);

  Void xPCMRestoration        (TComPic* pcPic);
  Void xPCMCURestoration      (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth);
  Void xPCMSampleRestoration  (TComDataCU* pcCU, UInt uiAbsZorderIdx, UInt uiDepth, TextType ttText);

public: //public methods, interface functions

  TComAdaptiveLoopFilter();
  virtual ~TComAdaptiveLoopFilter() {}

  Void create  ( Int iPicWidth, Int iPicHeight, UInt uiMaxCUWidth, UInt uiMaxCUHeight, UInt uiMaxCUDepth );
  Void destroy ();
  Void predictALFCoeffChroma  ( ALFParam* pAlfParam );                  //!< prediction of chroma ALF coefficients

  Void ALFProcess             ( TComPic* pcPic, ALFParam* pcAlfParam, std::vector<AlfCUCtrlInfo>& vAlfCUCtrlParam ); ///< interface function for ALF process

  Void allocALFParam  ( ALFParam* pAlfParam ); //!< allocate ALF parameters
  Void freeALFParam   ( ALFParam* pAlfParam ); //!< free ALF parameters
  Void copyALFParam   ( ALFParam* pDesAlfParam, ALFParam* pSrcAlfParam ); //!< copy ALF parameters

  Int  getNumCUsInPic()  {return m_uiNumCUsInFrame;} //!< get number of LCU in picture for ALF process


  Void createPicAlfInfo (TComPic* pcPic, Int numSlicesInPic = 1);
  Void destroyPicAlfInfo();

  Void PCMLFDisableProcess    ( TComPic* pcPic);                        ///< interface function for ALF process 

protected: //memory allocation
  Void destroyMatrix_Pel(Pel **m2D);
  Void destroyMatrix_int(int **m2D);
  Void initMatrix_int(int ***m2D, int d1, int d2);
  Void initMatrix_Pel(Pel ***m2D, int d1, int d2);
  Void destroyMatrix4D_double(double ****m4D, int d1, int d2);
  Void destroyMatrix3D_double(double ***m3D, int d1);
  Void destroyMatrix_double(double **m2D);
  Void initMatrix4D_double(double *****m4D, int d1, int d2, int d3, int d4);
  Void initMatrix3D_double(double ****m3D, int d1, int d2, int d3);
  Void initMatrix_double(double ***m2D, int d1, int d2);
  Void no_mem_exit(const char *where);
  Void xError(const char *text, int code);
};

//! \}

#endif
