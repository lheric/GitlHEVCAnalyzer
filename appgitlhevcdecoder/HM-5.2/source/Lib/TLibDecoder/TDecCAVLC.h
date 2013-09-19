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

/** \file     TDecCAVLC.h
    \brief    CAVLC decoder class (header)
*/

#ifndef __TDECCAVLC__
#define __TDECCAVLC__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TDecEntropy.h"

//! \ingroup TLibDecoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

class SEImessages;

/// CAVLC decoder class
class TDecCavlc : public TDecEntropyIf
{
public:
  TDecCavlc();
  virtual ~TDecCavlc();
  
protected:
  Void  xReadCode             (UInt   uiLength, UInt& ruiCode);
  Void  xReadUvlc             (UInt&  ruiVal);
  Void  xReadSvlc             (Int&   riVal);
  Void  xReadFlag             (UInt&  ruiCode);
  Void  xReadEpExGolomb       ( UInt& ruiSymbol, UInt uiCount );
  Void  xReadExGolombLevel    ( UInt& ruiSymbol );
  Void  xReadUnaryMaxSymbol   ( UInt& ruiSymbol, UInt uiMaxSymbol );
#if ENC_DEC_TRACE
  Void  xReadCodeTr           (UInt  length, UInt& rValue, const Char *pSymbolName);
  Void  xReadUvlcTr           (              UInt& rValue, const Char *pSymbolName);
  Void  xReadSvlcTr           (               Int& rValue, const Char *pSymbolName);
  Void  xReadFlagTr           (              UInt& rValue, const Char *pSymbolName);
#endif
  
  Void  xReadPCMAlignZero     ();

  UInt  xGetBit             ();
  
  void  parseShortTermRefPicSet            (TComPPS* pcPPS, TComReferencePictureSet* pcRPS, Int idx);
private:
  TComInputBitstream*   m_pcBitstream;
#if !PARAMSET_VLC_CLEANUP
  Bool m_bAlfCtrl;
  UInt m_uiMaxAlfCtrlDepth;
#endif
  Int           m_iSliceGranularity; //!< slice granularity
  
public:

  /// rest entropy coder by intial QP and IDC in CABAC
  Void  resetEntropy        (Int  iQp, Int iID) { printf("Not supported yet\n"); assert(0); exit(1);}
  Void  resetEntropy        ( TComSlice* pcSlice  );
  Void  setBitstream        ( TComInputBitstream* p )   { m_pcBitstream = p; }
#if !PARAMSET_VLC_CLEANUP
  Void  setAlfCtrl          ( Bool bAlfCtrl )            { m_bAlfCtrl = bAlfCtrl; }
  Void  setMaxAlfCtrlDepth  ( UInt uiMaxAlfCtrlDepth )  { m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth; }
#endif
  /// set slice granularity
  Void setSliceGranularity(Int iSliceGranularity)  {m_iSliceGranularity = iSliceGranularity;}

  /// get slice granularity
  Int  getSliceGranularity()                       {return m_iSliceGranularity;             }
  Void  parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize );
  Void  parseQtCbf          ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void  parseQtRootCbf      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf );
#if !PARAMSET_VLC_CLEANUP
  Void  parseAlfFlag        ( UInt& ruiVal );
  Void  parseAlfSvlc        ( Int&  riVal  );
  Void  parseAlfUvlc        ( UInt& ruiVal );
  Void  parseSaoFlag        ( UInt& ruiVal );
  Void  parseSaoUvlc        ( UInt& ruiVal );
  Void  parseSaoSvlc        ( Int&  riVal  );
#endif  
  Void  parseSPS            ( TComSPS* pcSPS );
  Void  parsePPS            ( TComPPS* pcPPS);
  Void  parseSEI(SEImessages&);
#if PARAMSET_VLC_CLEANUP
  Void  parseAPS             ( TComAPS* pAPS );
  Void  parseSliceHeader    ( TComSlice*& rpcSlice, ParameterSetManagerDecoder *parameterSetManager, AlfCUCtrlInfo &alfCUCtrl );
#else
  Void  parseSliceHeader    ( TComSlice*& rpcSlice );
  Void parseWPPTileInfoToSliceHeader(TComSlice*& rpcSlice);
#endif
  Void  parseTerminatingBit ( UInt& ruiBit );
  
  Void  parseMVPIdx         ( Int& riMVPIdx );
  
  Void  parseSkipFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseMergeFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx );
  Void parseMergeIndex      ( TComDataCU* pcCU, UInt& ruiMergeIndex, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseSplitFlag       ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePartSize        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parsePredMode        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void parseIntraDirLumaAng ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void parseIntraDirChroma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void parseInterDir        ( TComDataCU* pcCU, UInt& ruiInterDir, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseRefFrmIdx       ( TComDataCU* pcCU, Int& riRefFrmIdx,  UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
  Void parseMvd             ( TComDataCU* pcCU, UInt uiAbsPartAddr,UInt uiPartIdx,    UInt uiDepth, RefPicList eRefList );
  
  Void parseDeltaQP         ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseCoeffNxN        ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType );
  
  Void parseIPCMInfo        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);

#if !PARAMSET_VLC_CLEANUP
  Void parseAlfCtrlDepth    ( UInt& ruiAlfCtrlDepth );
  Void parseAlfCtrlFlag     ( UInt &ruiAlfCtrlFlag );
#endif
  Void readTileMarker     ( UInt& uiTileIdx, UInt uiBitsUsed );
  Void updateContextTables  ( SliceType eSliceType, Int iQp ) { return; }
#if OL_FLUSH
  Void decodeFlush() {};
#endif

#if !PARAMSET_VLC_CLEANUP
  /// parse APF flags before SAO and ALF parameters
  Void parseAPSInitInfo(TComAPS& cAPS);
#endif

  Void xParsePredWeightTable ( TComSlice* pcSlice );
  Void  parseScalingList               ( TComScalingList* scalingList );
  Void  xDecodeDPCMScalingListMatrix   ( TComScalingList *scalingList, Int* data, UInt sizeId, UInt listId);
  Void  xReadScalingListCode           ( TComScalingList *scalingList, Int* buf,  UInt sizeId, UInt listId);
  Void parseDFFlag         ( UInt& ruiVal, const Char *pSymbolName );
  Void parseDFSvlc         ( Int&  riVal,  const Char *pSymbolName  );
#if PARAMSET_VLC_CLEANUP
protected:
  Void  xParseSaoParam       ( SAOParam* pSaoParam );
  Void  xParseSaoSplitParam  ( SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr );
  Void  xParseSaoOffsetParam ( SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr );
  Void  xParseAlfParam       ( ALFParam* pAlfParam );
  Void  xParseAlfCuControlParam(AlfCUCtrlInfo& cAlfParam, Int iNumCUsInPic);
  Int   xGolombDecode        ( Int k );
#endif
};

//! \}

#endif // !defined(AFX_TDECCAVLC_H__9732DD64_59B0_4A41_B29E_1A5B18821EAD__INCLUDED_)

