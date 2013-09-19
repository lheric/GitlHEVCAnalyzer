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

/** \file     TEncCfg.h
    \brief    encoder configuration class (header)
*/

#ifndef __TENCCFG__
#define __TENCCFG__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TLibCommon/CommonDef.h"
#include <assert.h>

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder configuration class
class TEncCfg
{
protected:
  //==== File I/O ========
  Int       m_iFrameRate;
  Int       m_FrameSkip;
  Int       m_iSourceWidth;
  Int       m_iSourceHeight;
  Int       m_iFrameToBeEncoded;
  
  //====== Coding Structure ========
  UInt      m_uiIntraPeriod;
  UInt      m_uiDecodingRefreshType;            ///< the type of decoding refresh employed for the random access.
  Int       m_iGOPSize;
  Int       m_iRateGOPSize;
  Int       m_iNumOfReference;
  Int       m_iNumOfReferenceB_L0;
  Int       m_iNumOfReferenceB_L1;
  
  Int       m_iQP;                              //  if (AdaptiveQP == OFF)
  
  Int       m_aiTLayerQPOffset[MAX_TLAYER];
  Int       m_aiPad[2];
  

  Int       m_iMaxRefPicNum;                     ///< this is used to mimic the sliding mechanism used by the decoder
                                                 // TODO: We need to have a common sliding mechanism used by both the encoder and decoder

  Bool      m_bTLayering;                        ///< indicates whether temporal IDs are set based on the hierarchical coding structure
  Bool      m_abTLayerSwitchingFlag[MAX_TLAYER]; ///< temporal layer switching flags corresponding to temporal layer
#if DISABLE_4x4_INTER
  Bool      m_bDisInter4x4;
#endif
  //======= Transform =============
  UInt      m_uiQuadtreeTULog2MaxSize;
  UInt      m_uiQuadtreeTULog2MinSize;
  UInt      m_uiQuadtreeTUMaxDepthInter;
  UInt      m_uiQuadtreeTUMaxDepthIntra;
  
  //====== B Slice ========
  Bool      m_bHierarchicalCoding;              //  hierarchical-B coding
  
  //====== Entropy Coding ========
  Int       m_iSymbolMode;                      //  (CAVLC, CABAC)
  
  //====== Loop/Deblock Filter ========
  Bool      m_bLoopFilterDisable;
  Int       m_iLoopFilterAlphaC0Offset;
  Int       m_iLoopFilterBetaOffset;
  
#if MTK_SAO
  Bool      m_bUseSAO;
#endif

  //====== Motion search ========
  Int       m_iFastSearch;                      //  0:Full search  1:Diamond  2:PMVFAST
  Int       m_iSearchRange;                     //  0:Full frame
  Int       m_bipredSearchRange;

  //====== Quality control ========
  Int       m_iMaxDeltaQP;                      //  Max. absolute delta QP (1:default)
#if SUB_LCU_DQP
  Int       m_iMaxCuDQPDepth;                   //  Max. depth for a minimum CuDQP (0:default)
#endif
#if QP_ADAPTATION
  Bool      m_bUseAdaptiveQP;
  Int       m_iQPAdaptationRange;
#endif
  
  //====== Tool list ========
  Bool      m_bUseSBACRD;
  Bool      m_bUseALF;
#if MQT_ALF_NPASS
  Int       m_iALFEncodePassReduction;
#endif
  Bool      m_bUseASR;
  Bool      m_bUseHADME;
  Bool      m_bUseGPB;
  Bool      m_bUseLComb;
  Bool      m_bLCMod;
  Bool      m_bUseRDOQ;
  Bool      m_bUseLDC;
  Bool      m_bUsePAD;
  Bool      m_bUseNRF;
  Bool      m_bUseBQP;
  Bool      m_bUseFastEnc;
#if EARLY_CU_DETERMINATION
  Bool      m_bUseEarlyCU;
#endif
#if CBF_FAST_MODE
  Bool      m_bUseCbfFastMode;
#endif
  Bool      m_bUseMRG; // SOPH:
#if LM_CHROMA 
  Bool      m_bUseLMChroma; 
#endif

  Int*      m_aidQP;
  UInt      m_uiDeltaQpRD;
  
  Bool      m_bUseConstrainedIntraPred;
#if E057_INTRA_PCM
  UInt      m_uiPCMLog2MinSize;
#endif
  //====== Slice ========
  Int       m_iSliceMode;
  Int       m_iSliceArgument; 
  //====== Entropy Slice ========
  Int       m_iEntropySliceMode;
  Int       m_iEntropySliceArgument;
#if FINE_GRANULARITY_SLICES
  Int       m_iSliceGranularity;
#endif
#if MTK_NONCROSS_INLOOP_FILTER
  Bool      m_bLFCrossSliceBoundaryFlag;
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_BIT_DEPTH_SYNTAX
  Bool      m_bPCMInputBitDepthFlag;
  UInt      m_uiPCMBitDepthLuma;
  UInt      m_uiPCMBitDepthChroma;
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX 
  Bool      m_bPCMFilterDisableFlag;
#endif
  bool m_pictureDigestEnabled; ///< enable(1)/disable(0) md5 computation and SEI signalling

#if REF_SETTING_FOR_LD
  Bool      m_bUseNewRefSetting;
#endif

public:
  TEncCfg()          {}
  virtual ~TEncCfg() {}
  
  Void      setFrameRate                    ( Int   i )      { m_iFrameRate = i; }
  Void      setFrameSkip                    ( unsigned int i ) { m_FrameSkip = i; }
  Void      setSourceWidth                  ( Int   i )      { m_iSourceWidth = i; }
  Void      setSourceHeight                 ( Int   i )      { m_iSourceHeight = i; }
  Void      setFrameToBeEncoded             ( Int   i )      { m_iFrameToBeEncoded = i; }
  
  //====== Coding Structure ========
  Void      setIntraPeriod                  ( Int   i )      { m_uiIntraPeriod = (UInt)i; }
  Void      setDecodingRefreshType          ( Int   i )      { m_uiDecodingRefreshType = (UInt)i; }
  Void      setGOPSize                      ( Int   i )      { m_iGOPSize = i; }
  Void      setRateGOPSize                  ( Int   i )      { m_iRateGOPSize = i; }
  Void      setNumOfReference               ( Int   i )      { m_iNumOfReference = i; }
  Void      setNumOfReferenceB_L0           ( Int   i )      { m_iNumOfReferenceB_L0 = i; }
  Void      setNumOfReferenceB_L1           ( Int   i )      { m_iNumOfReferenceB_L1 = i; }
  
  Void      setQP                           ( Int   i )      { m_iQP = i; }
  
  Void      setTemporalLayerQPOffset        ( Int*  piTemporalLayerQPOffset )      { for ( Int i = 0; i < MAX_TLAYER; i++ ) m_aiTLayerQPOffset[i] = piTemporalLayerQPOffset[i]; }
  Void      setPad                          ( Int*  iPad                   )      { for ( Int i = 0; i < 2; i++ ) m_aiPad[i] = iPad[i]; }
  
  Int       getMaxRefPicNum                 ()                              { return m_iMaxRefPicNum;           }
  Void      setMaxRefPicNum                 ( Int iMaxRefPicNum )           { m_iMaxRefPicNum = iMaxRefPicNum;  }

  Bool      getTLayering                    ()                              { return m_bTLayering;              } 
  Void      setTLayering                    ( Bool bTLayering )             { m_bTLayering = bTLayering;        }
  Bool      getTLayerSwitchingFlag          ( UInt uiTLayer )               { assert (uiTLayer < MAX_TLAYER ); return  m_abTLayerSwitchingFlag[uiTLayer];                   }
  Void      setTLayerSwitchingFlag          ( Bool* pbTLayerSwitchingFlag ) { for ( Int i = 0; i < MAX_TLAYER; i++ ) m_abTLayerSwitchingFlag[i] = pbTLayerSwitchingFlag[i]; }

#if DISABLE_4x4_INTER
  Bool      getDisInter4x4                  ()              { return m_bDisInter4x4;        }
  Void      setDisInter4x4                  ( Bool b )      { m_bDisInter4x4  = b;          }
#endif
  //======== Transform =============
  Void      setQuadtreeTULog2MaxSize        ( UInt  u )      { m_uiQuadtreeTULog2MaxSize = u; }
  Void      setQuadtreeTULog2MinSize        ( UInt  u )      { m_uiQuadtreeTULog2MinSize = u; }
  Void      setQuadtreeTUMaxDepthInter      ( UInt  u )      { m_uiQuadtreeTUMaxDepthInter = u; }
  Void      setQuadtreeTUMaxDepthIntra      ( UInt  u )      { m_uiQuadtreeTUMaxDepthIntra = u; }
  
  //====== b; Slice ========
  Void      setHierarchicalCoding           ( Bool  b )      { m_bHierarchicalCoding = b; }
  
  //====== Entropy Coding ========
  Void      setSymbolMode                   ( Int   i )      { m_iSymbolMode = i; }
  
  //====== Loop/Deblock Filter ========
  Void      setLoopFilterDisable            ( Bool  b )      { m_bLoopFilterDisable       = b; }
  Void      setLoopFilterAlphaC0Offset      ( Int   i )      { m_iLoopFilterAlphaC0Offset = i; }
  Void      setLoopFilterBetaOffset         ( Int   i )      { m_iLoopFilterBetaOffset    = i; }
  
  //====== Motion search ========
  Void      setFastSearch                   ( Int   i )      { m_iFastSearch = i; }
  Void      setSearchRange                  ( Int   i )      { m_iSearchRange = i; }
  Void      setBipredSearchRange            ( Int   i )      { m_bipredSearchRange = i; }

  //====== Quality control ========
  Void      setMaxDeltaQP                   ( Int   i )      { m_iMaxDeltaQP = i; }
#if SUB_LCU_DQP
  Void      setMaxCuDQPDepth                ( Int   i )      { m_iMaxCuDQPDepth = i; }
#endif
#if QP_ADAPTATION
  Void      setUseAdaptiveQP                ( Bool  b )      { m_bUseAdaptiveQP = b; }
  Void      setQPAdaptationRange            ( Int   i )      { m_iQPAdaptationRange = i; }
#endif
  
  //====== Sequence ========
  Int       getFrameRate                    ()      { return  m_iFrameRate; }
  unsigned int getFrameSkip                 ()      { return  m_FrameSkip; }
  Int       getSourceWidth                  ()      { return  m_iSourceWidth; }
  Int       getSourceHeight                 ()      { return  m_iSourceHeight; }
  Int       getFrameToBeEncoded             ()      { return  m_iFrameToBeEncoded; }
  
  //==== Coding Structure ========
  UInt      getIntraPeriod                  ()      { return  m_uiIntraPeriod; }
  UInt      getDecodingRefreshType          ()      { return  m_uiDecodingRefreshType; }
  Int       getGOPSize                      ()      { return  m_iGOPSize; }
  Int       getRateGOPSize                  ()      { return  m_iRateGOPSize; }
  Int       getNumOfReference               ()      { return  m_iNumOfReference; }
  Int       getNumOfReferenceB_L0           ()      { return  m_iNumOfReferenceB_L0; }
  Int       getNumOfReferenceB_L1           ()      { return  m_iNumOfReferenceB_L1; }
  
  Int       getQP                           ()      { return  m_iQP; }
  
  Int       getTemporalLayerQPOffset        ( Int i )      { assert (i < MAX_TLAYER ); return  m_aiTLayerQPOffset[i]; }
  Int       getPad                          ( Int i )      { assert (i < 2 );                      return  m_aiPad[i]; }
  
  //======== Transform =============
  UInt      getQuadtreeTULog2MaxSize        ()      const { return m_uiQuadtreeTULog2MaxSize; }
  UInt      getQuadtreeTULog2MinSize        ()      const { return m_uiQuadtreeTULog2MinSize; }
  UInt      getQuadtreeTUMaxDepthInter      ()      const { return m_uiQuadtreeTUMaxDepthInter; }
  UInt      getQuadtreeTUMaxDepthIntra      ()      const { return m_uiQuadtreeTUMaxDepthIntra; }
  
  //==== b; Slice ========
  Bool      getHierarchicalCoding           ()      { return  m_bHierarchicalCoding; }
  
  //==== Entropy Coding ========
  Int       getSymbolMode                   ()      { return  m_iSymbolMode; }
  
  //==== Loop/Deblock Filter ========
  Bool      getLoopFilterDisable            ()      { return  m_bLoopFilterDisable;       }
  Int       getLoopFilterAlphaC0Offget      ()      { return  m_iLoopFilterAlphaC0Offset; }
  Int       getLoopFilterBetaOffget         ()      { return  m_iLoopFilterBetaOffset;    }
  
  //==== Motion search ========
  Int       getFastSearch                   ()      { return  m_iFastSearch; }
  Int       getSearchRange                  ()      { return  m_iSearchRange; }

  //==== Quality control ========
  Int       getMaxDeltaQP                   ()      { return  m_iMaxDeltaQP; }
#if SUB_LCU_DQP
  Int       getMaxCuDQPDepth                ()      { return  m_iMaxCuDQPDepth; }
#endif
#if QP_ADAPTATION
  Bool      getUseAdaptiveQP                ()      { return  m_bUseAdaptiveQP; }
  Int       getQPAdaptationRange            ()      { return  m_iQPAdaptationRange; }
#endif
  
  //==== Tool list ========
  Void      setUseSBACRD                    ( Bool  b )     { m_bUseSBACRD  = b; }
  Void      setUseASR                       ( Bool  b )     { m_bUseASR     = b; }
  Void      setUseHADME                     ( Bool  b )     { m_bUseHADME   = b; }
  Void      setUseALF                       ( Bool  b )     { m_bUseALF   = b; }
  Void      setUseGPB                       ( Bool  b )     { m_bUseGPB     = b; }
  Void      setUseLComb                     ( Bool  b )     { m_bUseLComb   = b; }
  Void      setLCMod                        ( Bool  b )     { m_bLCMod   = b;    }
  Void      setUseRDOQ                      ( Bool  b )     { m_bUseRDOQ    = b; }
  Void      setUseLDC                       ( Bool  b )     { m_bUseLDC     = b; }
  Void      setUsePAD                       ( Bool  b )     { m_bUsePAD     = b; }
  Void      setUseNRF                       ( Bool  b )     { m_bUseNRF     = b; }
  Void      setUseBQP                       ( Bool  b )     { m_bUseBQP     = b; }
  Void      setUseFastEnc                   ( Bool  b )     { m_bUseFastEnc = b; }
#if EARLY_CU_DETERMINATION
  Void      setUseEarlyCU                   ( Bool  b )     { m_bUseEarlyCU = b; }
#endif
#if CBF_FAST_MODE
  Void      setUseCbfFastMode            ( Bool  b )     { m_bUseCbfFastMode = b; }
#endif
  Void      setUseMRG                       ( Bool  b )     { m_bUseMRG     = b; } // SOPH:
  Void      setUseConstrainedIntraPred      ( Bool  b )     { m_bUseConstrainedIntraPred = b; }
#if E057_INTRA_PCM && E192_SPS_PCM_BIT_DEPTH_SYNTAX
  Void      setPCMInputBitDepthFlag         ( Bool  b )     { m_bPCMInputBitDepthFlag = b; }
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  Void      setPCMFilterDisableFlag         ( Bool  b )     {  m_bPCMFilterDisableFlag = b; }
#endif
#if E057_INTRA_PCM
  Void      setPCMLog2MinSize               ( UInt u )     { m_uiPCMLog2MinSize = u;      }
#endif
  Void      setdQPs                         ( Int*  p )     { m_aidQP       = p; }
  Void      setDeltaQpRD                    ( UInt  u )     {m_uiDeltaQpRD  = u; }
  Bool      getUseSBACRD                    ()      { return m_bUseSBACRD;  }
  Bool      getUseASR                       ()      { return m_bUseASR;     }
  Bool      getUseHADME                     ()      { return m_bUseHADME;   }
  Bool      getUseALF                       ()      { return m_bUseALF;     }
#if MQT_ALF_NPASS
  Void      setALFEncodePassReduction       (Int i)  { m_iALFEncodePassReduction = i; }
  Int       getALFEncodePassReduction       ()       { return m_iALFEncodePassReduction; }
#endif
  Bool      getUseGPB                       ()      { return m_bUseGPB;     }
  Bool      getUseLComb                     ()      { return m_bUseLComb;   }
  Bool      getLCMod                        ()      { return m_bLCMod; }
  Bool      getUseRDOQ                      ()      { return m_bUseRDOQ;    }
  Bool      getUseLDC                       ()      { return m_bUseLDC;     }
  Bool      getUsePAD                       ()      { return m_bUsePAD;     }
  Bool      getUseNRF                       ()      { return m_bUseNRF;     }
  Bool      getUseBQP                       ()      { return m_bUseBQP;     }
  Bool      getUseFastEnc                   ()      { return m_bUseFastEnc; }
#if EARLY_CU_DETERMINATION
  Bool      getUseEarlyCU                   ()      { return m_bUseEarlyCU; }
#endif
#if CBF_FAST_MODE
  Bool      getUseCbfFastMode           ()      { return m_bUseCbfFastMode; }
#endif
  Bool      getUseMRG                       ()      { return m_bUseMRG;     } // SOPH:
  Bool      getUseConstrainedIntraPred      ()      { return m_bUseConstrainedIntraPred; }
#if E057_INTRA_PCM && E192_SPS_PCM_BIT_DEPTH_SYNTAX
  Bool      getPCMInputBitDepthFlag         ()      { return m_bPCMInputBitDepthFlag;   } 
#endif
#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX
  Bool      getPCMFilterDisableFlag         ()      { return m_bPCMFilterDisableFlag;   } 
#endif
#if E057_INTRA_PCM
  UInt      getPCMLog2MinSize               ()      { return  m_uiPCMLog2MinSize;  }
#endif

#if LM_CHROMA 
  Bool getUseLMChroma                       ()      { return m_bUseLMChroma;        }
  Void setUseLMChroma                       ( Bool b ) { m_bUseLMChroma  = b;       }
#endif

  Int*      getdQPs                         ()      { return m_aidQP;       }
  UInt      getDeltaQpRD                    ()      { return m_uiDeltaQpRD; }

  //====== Slice ========
  Void  setSliceMode                   ( Int  i )       { m_iSliceMode = i;              }
  Void  setSliceArgument               ( Int  i )       { m_iSliceArgument = i;          }
  Int   getSliceMode                   ()              { return m_iSliceMode;           }
  Int   getSliceArgument               ()              { return m_iSliceArgument;       }
  //====== Entropy Slice ========
  Void  setEntropySliceMode            ( Int  i )      { m_iEntropySliceMode = i;       }
  Void  setEntropySliceArgument        ( Int  i )      { m_iEntropySliceArgument = i;   }
  Int   getEntropySliceMode            ()              { return m_iEntropySliceMode;    }
  Int   getEntropySliceArgument        ()              { return m_iEntropySliceArgument;}
#if FINE_GRANULARITY_SLICES
  Void  setSliceGranularity            ( Int  i )      { m_iSliceGranularity = i;       }
  Int   getSliceGranularity            ()              { return m_iSliceGranularity;    }
#endif
#if MTK_NONCROSS_INLOOP_FILTER
  Void      setLFCrossSliceBoundaryFlag     ( Bool   bValue  )    { m_bLFCrossSliceBoundaryFlag = bValue; }
  Bool      getLFCrossSliceBoundaryFlag     ()                    { return m_bLFCrossSliceBoundaryFlag;   }
#endif
#if MTK_SAO
  Void      setUseSAO                  (Bool bVal)     {m_bUseSAO = bVal;}
  Bool      getUseSAO                  ()              {return m_bUseSAO;}
#endif

  void setPictureDigestEnabled(bool b) { m_pictureDigestEnabled = b; }
  bool getPictureDigestEnabled() { return m_pictureDigestEnabled; }

#if REF_SETTING_FOR_LD
  Void      setUseNewRefSetting    ( Bool b ) { m_bUseNewRefSetting = b;    }
  Bool      getUseNewRefSetting    ()         { return m_bUseNewRefSetting; }
#endif

};

//! \}

#endif // !defined(AFX_TENCCFG_H__6B99B797_F4DA_4E46_8E78_7656339A6C41__INCLUDED_)
