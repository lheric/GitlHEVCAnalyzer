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

/** \file     TEncEntropy.h
    \brief    entropy encoder class (header)
*/

#ifndef __TENCENTROPY__
#define __TENCENTROPY__

#include "TLibCommon/TComSlice.h"
#include "TLibCommon/TComDataCU.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/ContextModel.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/TComAdaptiveLoopFilter.h"
#include "TLibCommon/TComSampleAdaptiveOffset.h"

class TEncSbac;
class TEncCavlc;
class SEI;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// entropy encoder pure class
class TEncEntropyIf
{
public:
  virtual Bool getAlfCtrl()                = 0;
  virtual UInt getMaxAlfCtrlDepth()                = 0;
  virtual Void setAlfCtrl(Bool bAlfCtrl)                = 0;
  virtual Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth)                = 0;
  
  virtual Void  resetEntropy          ()                = 0;
  virtual Void  setBitstream          ( TComBitIf* p )  = 0;
  virtual Void  setSlice              ( TComSlice* p )  = 0;
  virtual Void  resetBits             ()                = 0;
  virtual Void  resetCoeffCost        ()                = 0;
  virtual UInt  getNumberOfWrittenBits()                = 0;
  virtual UInt  getCoeffCost          ()                = 0;

  virtual Void  codeSPS                 ( TComSPS* pcSPS )                                      = 0;
  virtual Void  codePPS                 ( TComPPS* pcPPS )                                      = 0;
  virtual void codeSEI(const SEI&) = 0;
  virtual Void  codeSliceHeader         ( TComSlice* pcSlice )                                  = 0;
  virtual Void codeTileMarkerFlag      ( TComSlice* pcSlice )                                  = 0;

  virtual Void  codeSliceHeaderSubstreamTable( TComSlice* pcSlice )                             = 0;
  virtual Void  codeTerminatingBit      ( UInt uilsLast )                                       = 0;
  virtual Void  codeSliceFinish         ()                                                      = 0;
#if OL_FLUSH
  virtual Void  codeFlush               ()                                                      = 0;
  virtual Void  encodeStart             ()                                                      = 0;
#endif
  
  virtual Void codeAlfCtrlDepth() = 0;
  virtual Void codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList ) = 0;
  virtual Void codeScalingList   ( TComScalingList* scalingList )      = 0;
  
public:
  virtual Void codeAlfCtrlFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  
  virtual Void codeSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeMergeIndex    ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  
  virtual Void codePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void codePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  
  virtual Void codeIPCMInfo      ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;

  virtual Void codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx ) = 0;
  virtual Void codeQtCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth ) = 0;
  virtual Void codeQtRootCbf     ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  
  virtual Void codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeInterDir      ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeRefFrmIdx     ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )      = 0;
  virtual Void codeMvd           ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )      = 0;
  virtual Void codeDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx ) = 0;
  virtual Void codeCoeffNxN      ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType ) = 0;
  
  virtual Void codeAlfFlag          ( UInt uiCode ) = 0;
  virtual Void codeAlfUvlc          ( UInt uiCode ) = 0;
  virtual Void codeAlfSvlc          ( Int   iCode ) = 0;
  /// set slice granularity
  virtual Void setSliceGranularity(Int iSliceGranularity) = 0;

  /// get slice granularity
  virtual Int  getSliceGranularity()                      = 0;

  virtual Void codeAlfCtrlFlag      ( UInt uiSymbol ) = 0;
  virtual Void codeSaoFlag          ( UInt uiCode ) = 0;
  virtual Void codeSaoUvlc          ( UInt uiCode ) = 0;
  virtual Void codeSaoSvlc          ( Int   iCode ) = 0;
  virtual Void estBit               (estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType) = 0;
  
  virtual Void updateContextTables ( SliceType eSliceType, Int iQp, Bool bExecuteFinish )   = 0;
  virtual Void updateContextTables ( SliceType eSliceType, Int iQp )   = 0;
  virtual Void writeTileMarker             ( UInt uiTileIdx, UInt uiBitsUsed ) = 0;

  virtual Void codeAPSInitInfo  (TComAPS* pcAPS)= 0;
  virtual Void codeFinish       (Bool bEnd)= 0;

  virtual Void codeDFFlag (UInt uiCode, const Char *pSymbolName) = 0;
  virtual Void codeDFSvlc (Int iCode, const Char *pSymbolName)   = 0;

  virtual ~TEncEntropyIf() {}

};

/// entropy encoder class
class TEncEntropy
{
private:
  UInt    m_uiBakAbsPartIdx;
  UInt    m_uiBakChromaOffset;

public:
  Void    setEntropyCoder           ( TEncEntropyIf* e, TComSlice* pcSlice );
  Void    setBitstream              ( TComBitIf* p )          { m_pcEntropyCoderIf->setBitstream(p);  }
  Void    resetBits                 ()                        { m_pcEntropyCoderIf->resetBits();      }
  Void    resetCoeffCost            ()                        { m_pcEntropyCoderIf->resetCoeffCost(); }
  UInt    getNumberOfWrittenBits    ()                        { return m_pcEntropyCoderIf->getNumberOfWrittenBits(); }
  UInt    getCoeffCost              ()                        { return  m_pcEntropyCoderIf->getCoeffCost(); }
  Void    resetEntropy              ()                        { m_pcEntropyCoderIf->resetEntropy();  }
  
  Void    encodeSliceHeader         ( TComSlice* pcSlice );
  Void    encodeTileMarkerFlag       (TComSlice* pcSlice) {m_pcEntropyCoderIf->codeTileMarkerFlag(pcSlice);}
  Void    encodeSliceHeaderSubstreamTable( TComSlice* pcSlice );
  Void    encodeTerminatingBit      ( UInt uiIsLast );
  Void    encodeSliceFinish         ();
#if OL_FLUSH
  Void    encodeFlush               ();
  Void    encodeStart               ();
#endif

  
  Void encodeAlfParam(ALFParam* pAlfParam);
  
  TEncEntropyIf*      m_pcEntropyCoderIf;
  
public:
  // SPS
  Void encodeSPS               ( TComSPS* pcSPS );
  Void encodePPS               ( TComPPS* pcPPS );
  void encodeSEI(const SEI&);
  Bool getAlfCtrl() {return m_pcEntropyCoderIf->getAlfCtrl();}
  UInt getMaxAlfCtrlDepth() {return m_pcEntropyCoderIf->getMaxAlfCtrlDepth();}
  Void setAlfCtrl(Bool bAlfCtrl) {m_pcEntropyCoderIf->setAlfCtrl(bAlfCtrl);}
  Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth) {m_pcEntropyCoderIf->setMaxAlfCtrlDepth(uiMaxAlfCtrlDepth);}
  
  Void encodeSplitFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD = false );
  Void encodeSkipFlag          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodePUWise       ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodeInterDirPU   ( TComDataCU* pcSubCU, UInt uiAbsPartIdx  );
  Void encodeRefFrmIdxPU  ( TComDataCU* pcSubCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void encodeMvdPU        ( TComDataCU* pcSubCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void encodeMVPIdxPU     ( TComDataCU* pcSubCU, UInt uiAbsPartIdx, RefPicList eRefList );
  Void encodeMergeFlag    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPUIdx );
  Void encodeMergeIndex   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiPUIdx, Bool bRD = false );
  Void encodeAlfCtrlFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );

  /// set slice granularity
  Void setSliceGranularity (Int iSliceGranularity) {m_pcEntropyCoderIf->setSliceGranularity(iSliceGranularity);}

  /// encode ALF CU control flag
  Void encodeAlfCtrlFlag(UInt uiFlag);

  Void encodeAlfCtrlParam(AlfCUCtrlInfo& cAlfParam, Int iNumCUsInPic);

  Void encodePredMode          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodePartSize          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD = false );
  Void encodeIPCMInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodePredInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void encodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  
  Void encodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  
  Void encodeTransformIdx      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bRD = false );
  Void encodeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx );
  Void encodeQtCbf             ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  Void encodeQtRootCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx );
  Void encodeQP                ( TComDataCU* pcCU, UInt uiAbsPartIdx, Bool bRD = false );
  Void updateContextTables     ( SliceType eSliceType, Int iQp, Bool bExecuteFinish )   { m_pcEntropyCoderIf->updateContextTables( eSliceType, iQp, bExecuteFinish );     }
  Void updateContextTables     ( SliceType eSliceType, Int iQp )                        { m_pcEntropyCoderIf->updateContextTables( eSliceType, iQp, true );               }
  Void writeTileMarker              ( UInt uiTileIdx, UInt uiBitsUsed ) { m_pcEntropyCoderIf->writeTileMarker( uiTileIdx, uiBitsUsed ); }
  
  Void encodeAPSInitInfo          (TComAPS* pcAPS) {m_pcEntropyCoderIf->codeAPSInitInfo(pcAPS);}
  Void encodeFinish               (Bool bEnd) {m_pcEntropyCoderIf->codeFinish(bEnd);}
  Void encodeScalingList       ( TComScalingList* scalingList );
  Void encodeDFParams          (TComAPS* pcAPS);

private:
  Void xEncodeTransformSubdiv  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3 );
  Void xEncodeCoeff            ( TComDataCU* pcCU, UInt uiLumaOffset, UInt uiChromaOffset, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, Bool& bCodeDQP );
public:
  Void encodeCoeff             ( TComDataCU* pcCU,                 UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, Bool& bCodeDQP );
  
  Void encodeCoeffNxN         ( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiTrWidth, UInt uiTrHeight, UInt uiDepth, TextType eType );
  
  Void estimateBit             ( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, TextType eTType);
  
  // ALF-related
  Void codeAuxCountBit(ALFParam* pAlfParam, Int64* ruiRate);
  Void codeFiltCountBit(ALFParam* pAlfParam, Int64* ruiRate);
  Void codeAux (ALFParam* pAlfParam);
  Void codeFilt (ALFParam* pAlfParam);
  Int codeFilterCoeff(ALFParam* ALFp);
  Int writeFilterCodingParams(int minKStart, int minScanVal, int maxScanVal, int kMinTab[]);

  Int writeFilterCoeffs(int sqrFiltLength, int filters_per_group, int pDepthInt[], 
                        int **FilterCoeff, int kMinTab[]);
  Int golombEncode(int coeff, int k);
  Int lengthGolomb(int coeffVal, int k);
  Void    encodeSaoOnePart       (SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr);
  Void    encodeQuadTreeSplitFlag(SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr);
  Void    encodeSaoParam         (SAOParam* pSaoParam);

  static Int countNonZeroCoeffs( TCoeff* pcCoef, UInt uiSize );

};// END CLASS DEFINITION TEncEntropy

//! \}

#endif // __TENCENTROPY__

