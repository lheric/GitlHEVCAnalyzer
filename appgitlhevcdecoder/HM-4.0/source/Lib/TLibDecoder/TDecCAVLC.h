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
  
#if E057_INTRA_PCM
  Void  xReadPCMAlignZero     ();
#endif

  UInt  xGetBit             ();
  Int   xReadVlc            ( Int n );
#if CAVLC_COEF_LRG_BLK
  Void  xParseCoeff         ( TCoeff* scoeff, Int blockType, Int blSize
#if CAVLC_RUNLEVEL_TABLE_REM
                            , Int isIntra
#endif
                            );
#else
  Void  xParseCoeff4x4      ( TCoeff* scoeff, Int iTableNumber );
  Void  xParseCoeff8x8      ( TCoeff* scoeff, Int iTableNumber );
#endif
  Void  xRunLevelIndInv     (LastCoeffStruct *combo, Int maxrun, UInt lrg1Pos, UInt cn);
#if RUNLEVEL_TABLE_CUT
#if CAVLC_RUNLEVEL_TABLE_REM
  Void  xRunLevelIndInterInv(LastCoeffStruct *combo, Int maxrun, UInt cn, UInt scale);
#else
  Void  xRunLevelIndInterInv(LastCoeffStruct *combo, Int maxrun, UInt cn);
#endif
#endif
  
private:
  TComInputBitstream*   m_pcBitstream;
  UInt                  m_uiCoeffCost;
  Bool                  m_bRunLengthCoding;
  UInt                  m_uiRun;
  Bool m_bAlfCtrl;
  UInt m_uiMaxAlfCtrlDepth;
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
  Int           m_iSliceGranularity; //!< slice granularity
#endif
  UInt                      m_uiLPTableD4[3][32];
#if !CAVLC_COEF_LRG_BLK
  UInt                      m_uiLPTableD8[10][128];
#endif
  UInt                      m_uiLastPosVlcIndex[10];
  
#if FIXED_MPM
  UInt                      m_uiIntraModeTableD17[17];
  UInt                      m_uiIntraModeTableD34[34];
#elif MTK_DCM_MPM
  UInt                      m_uiIntraModeTableD17[2][16];
  UInt                      m_uiIntraModeTableD34[2][33];
#else
  UInt                      m_uiIntraModeTableD17[16];
  UInt                      m_uiIntraModeTableD34[33];
#endif
#if AMP
  UInt                      m_uiSplitTableD[4][11];
#else
  UInt                      m_uiSplitTableD[4][7];
#endif
#if CAVLC_RQT_CBP
  UInt                      m_uiCBP_YUV_TableD[4][8];
  UInt                      m_uiCBP_YS_TableD[2][4];
  UInt                      m_uiCBP_YCS_TableD[2][8];
  UInt                      m_uiCBP_4Y_TableD[2][15];
  UInt                      m_uiCBP_4Y_VlcIdx;
#else
  UInt                      m_uiBlkCBPTableD[2][15];
  UInt                      m_uiCBPTableD[2][8];
  UInt                      m_uiCbpVlcIdx[2];
  UInt                      m_uiBlkCbpVlcIdx;
#endif


  
  Int                   m_iRefFrame0[1000];
  Int                   m_iRefFrame1[1000];
  Bool                  m_bMVres0[1000];
  Bool                  m_bMVres1[1000];
  UInt                  m_uiMI1TableD[9];
  UInt                  m_uiMI2TableD[15]; 
  UInt                  m_uiMITableVlcIdx;

#if CAVLC_COUNTER_ADAPT
#if CAVLC_RQT_CBP
  UChar         m_ucCBP_YUV_TableCounter[4][4];
  UChar         m_ucCBP_4Y_TableCounter[2][2];
  UChar         m_ucCBP_YS_TableCounter[2][3];
  UChar         m_ucCBP_YCS_TableCounter[2][4];
  UChar         m_ucCBP_YUV_TableCounterSum[4];
  UChar         m_ucCBP_4Y_TableCounterSum[2];
  UChar         m_ucCBP_YS_TableCounterSum[2];
  UChar         m_ucCBP_YCS_TableCounterSum[2];
#else
  UChar         m_ucCBFTableCounter    [2][4];
  UChar         m_ucBlkCBPTableCounter [2][2];
  UChar         m_ucCBFTableCounterSum[2];
  UChar         m_ucBlkCBPTableCounterSum[2];
#endif

  UChar         m_ucMI1TableCounter[4];
  UChar         m_ucSplitTableCounter[4][4];
  UChar         m_ucSplitTableCounterSum[4];
  UChar         m_ucMI1TableCounterSum;
#endif

  
public:
  Void  resetEntropy        ( TComSlice* pcSlice  );
  Void  setBitstream        ( TComInputBitstream* p )   { m_pcBitstream = p; }
  Void  setAlfCtrl          ( Bool bAlfCtrl )            { m_bAlfCtrl = bAlfCtrl; }
  Void  setMaxAlfCtrlDepth  ( UInt uiMaxAlfCtrlDepth )  { m_uiMaxAlfCtrlDepth = uiMaxAlfCtrlDepth; }
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
  /// set slice granularity
  Void setSliceGranularity(Int iSliceGranularity)  {m_iSliceGranularity = iSliceGranularity;}

  /// get slice granularity
  Int  getSliceGranularity()                       {return m_iSliceGranularity;             }
#endif
  Void  parseTransformSubdivFlag( UInt& ruiSubdivFlag, UInt uiLog2TransformBlockSize );
  Void  parseQtCbf          ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void  parseQtRootCbf      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiQtRootCbf );
  Void  parseAlfFlag        ( UInt& ruiVal );
  Void  parseAlfUvlc        ( UInt& ruiVal );
  Void  parseAlfSvlc        ( Int&  riVal  );
#if MTK_SAO
  Void  parseAoFlag        ( UInt& ruiVal );
  Void  parseAoUvlc        ( UInt& ruiVal );
  Void  parseAoSvlc        ( Int&  riVal  );
#endif
  
  Void  parseSPS            ( TComSPS* pcSPS );
  Void  parsePPS            ( TComPPS* pcPPS);
  void parseSEI(SEImessages&);
  Void  parseSliceHeader    ( TComSlice*& rpcSlice );
  Void  parseTerminatingBit ( UInt& ruiBit );
  
  Void  parseMVPIdx         ( TComDataCU* pcCU, Int& riMVPIdx, Int iMVPNum, UInt uiAbsPartIdx, UInt uiDepth, RefPicList eRefList );
  
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
#if CAVLC_RQT_CBP
  Void parseCbfTrdiv        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiTrDepth, UInt uiDepth, UInt& uiSubdiv );
  UInt xGetFlagPattern      ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth,  UInt& uiSubdiv );
#endif
  Void parseCbf             ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth );
  Void parseBlockCbf        ( TComDataCU* pcCU, UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth, UInt uiDepth, UInt uiQPartNum );
  Void parseCoeffNxN        ( TComDataCU* pcCU, TCoeff* pcCoef, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight, UInt uiDepth, TextType eTType );
  
#if E057_INTRA_PCM
  Void parseIPCMInfo        ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth);
#endif

  Void parseAlfCtrlDepth    ( UInt& ruiAlfCtrlDepth );
  Void parseAlfCtrlFlag     ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth );
  Void parseAlfFlagNum      ( UInt& ruiVal, UInt minValue, UInt depth );
  Void parseAlfCtrlFlag     ( UInt &ruiAlfCtrlFlag );
};

//! \}

#endif // !defined(AFX_TDECCAVLC_H__9732DD64_59B0_4A41_B29E_1A5B18821EAD__INCLUDED_)

