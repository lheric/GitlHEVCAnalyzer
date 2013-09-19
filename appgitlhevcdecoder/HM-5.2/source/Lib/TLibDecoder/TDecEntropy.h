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

/** \file     TDecEntropy.h
    \brief    entropy decoder class (header)
*/

#ifndef __TDECENTROPY__
#define __TDECENTROPY__

#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/TComSlice.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComPrediction.h"
#include "TLibCommon/TComAdaptiveLoopFilter.h"
#include "TLibCommon/TComSampleAdaptiveOffset.h"

class TDecSbac;
class TDecCavlc;
class SEImessages;
#if PARAMSET_VLC_CLEANUP
class ParameterSetManagerDecoder;
#endif

//! \ingroup TLibDecoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// entropy decoder pure class
class TDecEntropyIf
{
public:
  //  Virtual list for SBAC/CAVLC
#if !PARAMSET_VLC_CLEANUP
  virtual Void setAlfCtrl(Bool bAlfCtrl)  = 0;
  virtual Void setMaxAlfCtrlDepth(UInt uiMaxAlfCtrlDepth)  = 0;
  
  virtual Void  resetEntropy          (Int  iQp, Int iID) = 0;
#endif
  virtual Void  resetEntropy          (TComSlice* pcSlice)                = 0;
  virtual Void  setBitstream          ( TComInputBitstream* p )  = 0;

#if OL_FLUSH
  virtual Void  decodeFlush()                                                                      = 0;
#endif

  virtual Void  parseSPS                  ( TComSPS* pcSPS )                                      = 0;
  virtual Void  parsePPS                  ( TComPPS* pcPPS )                                      = 0;
#if PARAMSET_VLC_CLEANUP
  virtual Void  parseAPS                  ( TComAPS* pAPS  )                                      = 0;
#endif
  virtual void parseSEI(SEImessages&) = 0;
#if PARAMSET_VLC_CLEANUP
  virtual Void parseSliceHeader          ( TComSlice*& rpcSlice, ParameterSetManagerDecoder *parameterSetManager, AlfCUCtrlInfo &alfCUCtrl )                                = 0;
#else
  virtual Void parseSliceHeader          ( TComSlice*& rpcSlice )                                = 0;
  virtual Void  parseWPPTileInfoToSliceHeader  ( TComSlice*& rpcSlice )                           = 0;
#endif
  virtual Void  parseTerminatingBit       ( UInt& ruilsLast )                                     = 0;
  
  virtual Void parseMVPIdx        ( Int& riMVPIdx ) = 0;
  
public:
  virtual Void parseSkipFlag      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseSplitFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseMergeFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx ) = 0;
  virtual Void parseMergeIndex    ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parsePartSize      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parsePredMode      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  
  virtual Void parseIntraDirLumaAng( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  
  virtual Void parseIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  
  virtual Void parseInterDir      ( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  virtual Void parseRefFrmIdx     ( TComDataCU* pcCU, Int& riRefFrmIdx, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList ) = 0;
  virtual Void parseMvd           ( TComDataCU* pcCU, UInt uiAbsPartAddr, UInt uiPartIdx, UInt uiDepth, RefPicList eRefList ) = 0;
  
  virtual Void parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize ) = 0;
  virtual Void parseQtCbf         ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth ) = 0;
  virtual Void parseQtRootCbf     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf ) = 0;
  
  virtual Void parseDeltaQP       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth ) = 0;
  
  virtual Void parseIPCMInfo     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth) = 0;

  virtual Void parseCoeffNxN( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType ) = 0;

#if !PARAMSET_VLC_CLEANUP
  virtual Void parseAlfFlag       ( UInt& ruiVal           ) = 0;
  virtual Void parseAlfUvlc       ( UInt& ruiVal           ) = 0;
  virtual Void parseAlfSvlc       ( Int&  riVal            ) = 0;
  virtual Void parseAlfCtrlDepth  ( UInt& ruiAlfCtrlDepth  ) = 0;
  virtual Void parseAlfCtrlFlag   ( UInt &ruiAlfCtrlFlag ) = 0;
#endif

  /// set slice granularity
  virtual Void setSliceGranularity(Int iSliceGranularity) = 0;

  /// get slice granularity
  virtual Int  getSliceGranularity()                      = 0;

#if !PARAMSET_VLC_CLEANUP
  virtual Void parseSaoFlag       ( UInt& ruiVal           ) = 0;
  virtual Void parseSaoUvlc       ( UInt& ruiVal           ) = 0;
  virtual Void parseSaoSvlc       ( Int&  riVal            ) = 0;
#endif
  virtual Void readTileMarker   ( UInt& uiTileIdx, UInt uiBitsUsed ) = 0;
  virtual Void updateContextTables( SliceType eSliceType, Int iQp ) = 0;
  
#if !PARAMSET_VLC_CLEANUP
  virtual Void parseAPSInitInfo   (TComAPS& cAPS) = 0;
  virtual Void parseScalingList   ( TComScalingList* scalingList ) = 0;
  virtual Void parseDFFlag(UInt& ruiVal, const Char *pSymbolName) = 0;
  virtual Void parseDFSvlc(Int&  riVal, const Char *pSymbolName) = 0;
#endif

  virtual ~TDecEntropyIf() {}
};

/// entropy decoder class
class TDecEntropy
{
private:
  TDecEntropyIf*  m_pcEntropyDecoderIf;
  TComPrediction* m_pcPrediction;
  UInt    m_uiBakAbsPartIdx;
  UInt    m_uiBakChromaOffset;
  
public:
  Void init (TComPrediction* p) {m_pcPrediction = p;}
  Void decodePUWise       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU );
  Void decodeInterDirPU   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx );
  Void decodeRefFrmIdxPU  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList );
  Void decodeMvdPU        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList );
  Void decodeMVPIdxPU     ( TComDataCU* pcSubCU, UInt uiPartAddr, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList );
  
  Void    setEntropyDecoder           ( TDecEntropyIf* p );
  Void    setBitstream                ( TComInputBitstream* p ) { m_pcEntropyDecoderIf->setBitstream(p);                    }
#if !PARAMSET_VLC_CLEANUP
  Void    resetEntropy                (Int  iQp, Int iID) { m_pcEntropyDecoderIf->resetEntropy(iQp, iID);                    }
#endif
  Void    resetEntropy                ( TComSlice* p)           { m_pcEntropyDecoderIf->resetEntropy(p);                    }

  Void    decodeSPS                   ( TComSPS* pcSPS     )    { m_pcEntropyDecoderIf->parseSPS(pcSPS);                    }
  Void    decodePPS                   ( TComPPS* pcPPS     )    { m_pcEntropyDecoderIf->parsePPS(pcPPS);                    }
#if PARAMSET_VLC_CLEANUP
  Void    decodeAPS                   ( TComAPS* pAPS      )    { m_pcEntropyDecoderIf->parseAPS(pAPS);}
#endif
  void decodeSEI(SEImessages& seis) { m_pcEntropyDecoderIf->parseSEI(seis); }
#if PARAMSET_VLC_CLEANUP
  Void    decodeSliceHeader           ( TComSlice*& rpcSlice, ParameterSetManagerDecoder *parameterSetManager, AlfCUCtrlInfo &alfCUCtrl )  { m_pcEntropyDecoderIf->parseSliceHeader(rpcSlice, parameterSetManager, alfCUCtrl);         }
#else
  Void    decodeSliceHeader           ( TComSlice*& rpcSlice )  { m_pcEntropyDecoderIf->parseSliceHeader(rpcSlice);         }
  Void    decodeWPPTileInfoToSliceHeader  ( TComSlice*& rpcSlice )  { m_pcEntropyDecoderIf->parseWPPTileInfoToSliceHeader(rpcSlice); }
#endif
  Void    decodeTerminatingBit        ( UInt& ruiIsLast )       { m_pcEntropyDecoderIf->parseTerminatingBit(ruiIsLast);     }
  
#if !PARAMSET_VLC_CLEANUP
  // Adaptive Loop filter
  Void decodeAlfParam(ALFParam* pAlfParam);
  //--Adaptive Loop filter
#endif
  
  TDecEntropyIf* getEntropyDecoder() { return m_pcEntropyDecoderIf; }
  
public:
  Void decodeSplitFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeSkipFlag          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeMergeFlag         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx );
  Void decodeMergeIndex        ( TComDataCU* pcSubCU, UInt uiPartIdx, UInt uiPartAddr, PartSize eCUMode, UChar* puhInterDirNeighbours, TComMvField* pcMvFieldNeighbours, UInt uiDepth );
#if !PARAMSET_VLC_CLEANUP
  Void decodeAlfCtrlParam      (AlfCUCtrlInfo& cAlfParam, Int iNumCUsInPic);
#endif
  Void decodePredMode          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodePartSize          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void decodeIPCMInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );

  Void decodePredInfo          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU );
  
  Void decodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void decodeTransformIdx      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void decodeQP                ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void readTileMarker         ( UInt& uiTileIdx, UInt uiBitsUsed )  {  m_pcEntropyDecoderIf->readTileMarker( uiTileIdx, uiBitsUsed ); }
  Void updateContextTables    ( SliceType eSliceType, Int iQp ) { m_pcEntropyDecoderIf->updateContextTables( eSliceType, iQp ); }
  
  
private:
  Void xDecodeTransformSubdiv  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3 );
  
  Void xDecodeCoeff            ( TComDataCU* pcCU, UInt uiLumaOffset, UInt uiChromaOffset, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, Bool& bCodeDQP );
public:
  Void decodeCoeff             ( TComDataCU* pcCU                 , UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, Bool& bCodeDQP );
  
  // ALF-related
#if !PARAMSET_VLC_CLEANUP
  Void decodeAux(ALFParam* pAlfParam);
  Void decodeFilt(ALFParam* pAlfParam);
  Void readFilterCodingParams(ALFParam* pAlfParam);
  Void readFilterCoeffs(ALFParam* pAlfParam);
  Void decodeFilterCoeff (ALFParam* pAlfParam);
  Int golombDecode(Int k);
#endif

  /// set slice granularity
  Void setSliceGranularity (Int iSliceGranularity) {m_pcEntropyDecoderIf->setSliceGranularity(iSliceGranularity);}

#if !PARAMSET_VLC_CLEANUP
  Void decodeSaoOnePart       (SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr);
  Void decodeQuadTreeSplitFlag(SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr);
  Void decodeSaoParam         (SAOParam* pSaoParam);
#endif
#if OL_FLUSH
  Void decodeFlush() { m_pcEntropyDecoderIf->decodeFlush(); }
#endif

#if !PARAMSET_VLC_CLEANUP
  Void decodeAPSInitInfo       (TComAPS& cAPS) {m_pcEntropyDecoderIf->parseAPSInitInfo(cAPS);}
  Void decodeScalingList       ( TComScalingList* scalingList ) { m_pcEntropyDecoderIf->parseScalingList(scalingList); }
  Void decodeDFParams (TComAPS* pcAPS);
#endif

};// END CLASS DEFINITION TDecEntropy

//! \}

#endif // __TDECENTROPY__

