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

/** \file     TComDataCU.h
    \brief    CU data structure (header)
    \todo     not all entities are documented
*/

#ifndef _TCOMDATACU_
#define _TCOMDATACU_

#include <assert.h>

// Include files
#include "CommonDef.h"
#include "TComMotionInfo.h"
#include "TComSlice.h"
#include "TComRdCost.h"
#include "TComPattern.h"

#include <algorithm>
#include <vector>

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CU data structure class
class TComDataCU
{
private:
  
  // -------------------------------------------------------------------------------------------------------------------
  // class pointers
  // -------------------------------------------------------------------------------------------------------------------
  
  TComPic*      m_pcPic;              ///< picture class pointer
  TComSlice*    m_pcSlice;            ///< slice header pointer
  TComPattern*  m_pcPattern;          ///< neighbour access class pointer
  
  // -------------------------------------------------------------------------------------------------------------------
  // CU description
  // -------------------------------------------------------------------------------------------------------------------
  
  UInt          m_uiCUAddr;           ///< CU address in a slice
  UInt          m_uiAbsIdxInLCU;      ///< absolute address in a CU. It's Z scan order
  UInt          m_uiCUPelX;           ///< CU position in a pixel (X)
  UInt          m_uiCUPelY;           ///< CU position in a pixel (Y)
  UInt          m_uiNumPartition;     ///< total number of minimum partitions in a CU
  UChar*        m_puhWidth;           ///< array of widths
  UChar*        m_puhHeight;          ///< array of heights
  UChar*        m_puhDepth;           ///< array of depths
  Int           m_unitSize;           ///< size of a "minimum partition"
  
  // -------------------------------------------------------------------------------------------------------------------
  // CU data
  // -------------------------------------------------------------------------------------------------------------------
  
  Char*         m_pePartSize;         ///< array of partition sizes
  Char*         m_pePredMode;         ///< array of prediction modes
  UChar*        m_phQP;               ///< array of QP values
  UChar*        m_puhTrIdx;           ///< array of transform indices
  UChar*        m_puhCbf[3];          ///< array of coded block flags (CBF)
  TComCUMvField m_acCUMvField[2];     ///< array of motion vectors
  TCoeff*       m_pcTrCoeffY;         ///< transformed coefficient buffer (Y)
  TCoeff*       m_pcTrCoeffCb;        ///< transformed coefficient buffer (Cb)
  TCoeff*       m_pcTrCoeffCr;        ///< transformed coefficient buffer (Cr)
  
#if E057_INTRA_PCM
  Pel*          m_pcIPCMSampleY;      ///< PCM sample buffer (Y)
  Pel*          m_pcIPCMSampleCb;     ///< PCM sample buffer (Cb)
  Pel*          m_pcIPCMSampleCr;     ///< PCM sample buffer (Cr)
#endif

#if MTK_NONCROSS_INLOOP_FILTER
  Int*          m_piSliceSUMap;       ///< pointer of slice ID map
#endif


  // -------------------------------------------------------------------------------------------------------------------
  // neighbour access variables
  // -------------------------------------------------------------------------------------------------------------------
  
  TComDataCU*   m_pcCUAboveLeft;      ///< pointer of above-left CU
  TComDataCU*   m_pcCUAboveRight;     ///< pointer of above-right CU
  TComDataCU*   m_pcCUAbove;          ///< pointer of above CU
  TComDataCU*   m_pcCULeft;           ///< pointer of left CU
  TComDataCU*   m_apcCUColocated[2];  ///< pointer of temporally colocated CU's for both directions
  TComMvField   m_cMvFieldA;          ///< motion vector of position A
  TComMvField   m_cMvFieldB;          ///< motion vector of position B
  TComMvField   m_cMvFieldC;          ///< motion vector of position C
  TComMv        m_cMvPred;            ///< motion vector predictor
  
  // -------------------------------------------------------------------------------------------------------------------
  // coding tool information
  // -------------------------------------------------------------------------------------------------------------------
  
  Bool*         m_pbMergeFlag;        ///< array of merge flags
  UChar*        m_puhMergeIndex;      ///< array of merge candidate indices
#if !MRG_AMVP_FIXED_IDX_F470
  UChar*        m_apuhNeighbourCandIdx[ MRG_MAX_NUM_CANDS ];///< array of motion vector predictor candidates indices
#endif
#if AMP_MRG
  Bool          m_bIsMergeAMP;
#endif
  UChar*        m_puhLumaIntraDir;    ///< array of intra directions (luma)
  UChar*        m_puhChromaIntraDir;  ///< array of intra directions (chroma)
  UChar*        m_puhInterDir;        ///< array of inter directions
  Char*         m_apiMVPIdx[2];       ///< array of motion vector predictor candidates
  Char*         m_apiMVPNum[2];       ///< array of number of possible motion vectors predictors
  Bool*         m_puiAlfCtrlFlag;     ///< array of ALF flags
  Bool*         m_puiTmpAlfCtrlFlag;  ///< temporal array of ALF flags
  
#if E057_INTRA_PCM
  Bool*         m_pbIPCMFlag;         ///< array of intra_pcm flags
#endif

  // -------------------------------------------------------------------------------------------------------------------
  // misc. variables
  // -------------------------------------------------------------------------------------------------------------------
  
  Bool          m_bDecSubCu;          ///< indicates decoder-mode
  Double        m_dTotalCost;         ///< sum of partition RD costs
  UInt          m_uiTotalDistortion;  ///< sum of partition distortion
  UInt          m_uiTotalBits;        ///< sum of partition bits
#if FINE_GRANULARITY_SLICES
  UInt          m_uiTotalBins;       ///< sum of partition bins
  UInt*         m_uiSliceStartCU;    ///< Start CU address of current slice
  UInt*         m_uiEntropySliceStartCU; ///< Start CU address of current slice
#else
  UInt          m_uiSliceStartCU;    ///< Start CU address of current slice
  UInt          m_uiEntropySliceStartCU; ///< Start CU address of current slice
#endif
  
protected:
  
  /// add possible motion vector predictor candidates
  Bool          xAddMVPCand           ( AMVPInfo* pInfo, RefPicList eRefPicList, Int iRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir );
  Bool          xAddMVPCandOrder      ( AMVPInfo* pInfo, RefPicList eRefPicList, Int iRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir );

  Void          deriveRightBottomIdx        ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxRB );
  Bool          xGetColMVP( RefPicList eRefPicList, Int uiCUAddr, Int uiPartUnitIdx, TComMv& rcMv, Int& riRefIdx );
  
  /// remove redundant candidates
  Void          xUniqueMVPCand        ( AMVPInfo* pInfo );

  Void xCheckCornerCand( TComDataCU* pcCorner, UInt uiCornerIdx, UInt uiIter, Bool& rbValidCand );
  /// compute required bits to encode MVD (used in AMVP)
  UInt          xGetMvdBits           ( TComMv cMvd );
  UInt          xGetComponentBits     ( Int iVal );
  
  /// compute scaling factor from POC difference
  Int           xGetDistScaleFactor   ( Int iCurrPOC, Int iCurrRefPOC, Int iColPOC, Int iColRefPOC );
  
#if FT_TCTR_AMVP || FT_TCTR_MRG
  Void xDeriveCenterIdx( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxCenter );
  Bool xGetCenterCol( UInt uiPartIdx, RefPicList eRefPicList, int iRefIdx, TComMv *pcMv );
#endif
  
#if MRG_AMVP_ADD_CAND_F470
  Void xCheckDuplicateCand(TComMvField* pcMvFieldNeighbours, UChar* puhInterDirNeighbours, bool* pbCandIsInter, UInt& ruiArrayAddr);
#endif

#if SUB_LCU_DQP
  Int           getLastValidPartIdx   ( Int iAbsPartIdx );
#endif

public:
  TComDataCU();
  virtual ~TComDataCU();
  
  // -------------------------------------------------------------------------------------------------------------------
  // create / destroy / initialize / copy
  // -------------------------------------------------------------------------------------------------------------------
  
  Void          create                ( UInt uiNumPartition, UInt uiWidth, UInt uiHeight, Bool bDecSubCu, Int unitSize );
  Void          destroy               ();
  
  Void          initCU                ( TComPic* pcPic, UInt uiCUAddr );
#if SUB_LCU_DQP
  Void          initEstData           ( UInt uiDepth, UInt uiQP );
#else
  Void          initEstData           ();
#endif
#if SUB_LCU_DQP
  Void          initSubCU             ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth, UInt uiQP );
#else
  Void          initSubCU             ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );
#endif
  Void          setOutsideCUPart      ( UInt uiAbsPartIdx, UInt uiDepth );

  Void          copySubCU             ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );
  Void          copyInterPredInfoFrom ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList );
  Void          copyPartFrom          ( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth );
  
  Void          copyToPic             ( UChar uiDepth );
  Void          copyToPic             ( UChar uiDepth, UInt uiPartIdx, UInt uiPartDepth );
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for CU description
  // -------------------------------------------------------------------------------------------------------------------
  
  TComPic*      getPic                ()                        { return m_pcPic;           }
  TComSlice*    getSlice              ()                        { return m_pcSlice;         }
  UInt&         getAddr               ()                        { return m_uiCUAddr;        }
  UInt&         getZorderIdxInCU      ()                        { return m_uiAbsIdxInLCU; }
#if FINE_GRANULARITY_SLICES
  UInt          getSCUAddr            ()                        { return m_uiCUAddr*(1<<(m_pcSlice->getSPS()->getMaxCUDepth()<<1))+m_uiAbsIdxInLCU;}
#endif
  UInt          getCUPelX             ()                        { return m_uiCUPelX;        }
  UInt          getCUPelY             ()                        { return m_uiCUPelY;        }
  TComPattern*  getPattern            ()                        { return m_pcPattern;       }
  
  UChar*        getDepth              ()                        { return m_puhDepth;        }
  UChar         getDepth              ( UInt uiIdx )            { return m_puhDepth[uiIdx]; }
  Void          setDepth              ( UInt uiIdx, UChar  uh ) { m_puhDepth[uiIdx] = uh;   }
  
  Void          setDepthSubParts      ( UInt uiDepth, UInt uiAbsPartIdx );
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for CU data
  // -------------------------------------------------------------------------------------------------------------------
  
  Char*         getPartitionSize      ()                        { return m_pePartSize;        }
  PartSize      getPartitionSize      ( UInt uiIdx )            { return static_cast<PartSize>( m_pePartSize[uiIdx] ); }
  Void          setPartitionSize      ( UInt uiIdx, PartSize uh){ m_pePartSize[uiIdx] = uh;   }
  Void          setPartSizeSubParts   ( PartSize eMode, UInt uiAbsPartIdx, UInt uiDepth );
  
  Char*         getPredictionMode     ()                        { return m_pePredMode;        }
  PredMode      getPredictionMode     ( UInt uiIdx )            { return static_cast<PredMode>( m_pePredMode[uiIdx] ); }
  Void          setPredictionMode     ( UInt uiIdx, PredMode uh){ m_pePredMode[uiIdx] = uh;   }
  Void          setPredModeSubParts   ( PredMode eMode, UInt uiAbsPartIdx, UInt uiDepth );
  
  UChar*        getWidth              ()                        { return m_puhWidth;          }
  UChar         getWidth              ( UInt uiIdx )            { return m_puhWidth[uiIdx];   }
  Void          setWidth              ( UInt uiIdx, UChar  uh ) { m_puhWidth[uiIdx] = uh;     }
  
  UChar*        getHeight             ()                        { return m_puhHeight;         }
  UChar         getHeight             ( UInt uiIdx )            { return m_puhHeight[uiIdx];  }
  Void          setHeight             ( UInt uiIdx, UChar  uh ) { m_puhHeight[uiIdx] = uh;    }
  
  Void          setSizeSubParts       ( UInt uiWidth, UInt uiHeight, UInt uiAbsPartIdx, UInt uiDepth );
  
  UChar*        getQP                 ()                        { return m_phQP;              }
  UChar         getQP                 ( UInt uiIdx )            { return m_phQP[uiIdx];       }
  Void          setQP                 ( UInt uiIdx, UChar  uh ) { m_phQP[uiIdx] = uh;         }
  Void          setQPSubParts         ( UInt uiQP,   UInt uiAbsPartIdx, UInt uiDepth );
#if SUB_LCU_DQP
  UChar         getLastCodedQP        ( UInt uiAbsPartIdx );
#endif
  
  UChar*        getTransformIdx       ()                        { return m_puhTrIdx;          }
  UChar         getTransformIdx       ( UInt uiIdx )            { return m_puhTrIdx[uiIdx];   }
  Void          setTrIdxSubParts      ( UInt uiTrIdx, UInt uiAbsPartIdx, UInt uiDepth );
  
  UInt          getQuadtreeTULog2MinSizeInCU( UInt uiIdx );
  
  TComCUMvField* getCUMvField         ( RefPicList e )          { return  &m_acCUMvField[e];  }
  
  TCoeff*&      getCoeffY             ()                        { return m_pcTrCoeffY;        }
  TCoeff*&      getCoeffCb            ()                        { return m_pcTrCoeffCb;       }
  TCoeff*&      getCoeffCr            ()                        { return m_pcTrCoeffCr;       }
  
#if E057_INTRA_PCM
  Pel*&         getPCMSampleY         ()                        { return m_pcIPCMSampleY;     }
  Pel*&         getPCMSampleCb        ()                        { return m_pcIPCMSampleCb;    }
  Pel*&         getPCMSampleCr        ()                        { return m_pcIPCMSampleCr;    }
#endif

  UChar         getCbf    ( UInt uiIdx, TextType eType )                  { return m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx];  }
  UChar*        getCbf    ( TextType eType )                              { return m_puhCbf[g_aucConvertTxtTypeToIdx[eType]];         }
  UChar         getCbf    ( UInt uiIdx, TextType eType, UInt uiTrDepth )  { return ( ( getCbf( uiIdx, eType ) >> uiTrDepth ) & 0x1 ); }
  Void          setCbf    ( UInt uiIdx, TextType eType, UChar uh )        { m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx] = uh;    }
  Void          clearCbf  ( UInt uiIdx, TextType eType, UInt uiNumParts );
  UChar         getQtRootCbf          ( UInt uiIdx )                      { return getCbf( uiIdx, TEXT_LUMA, 0 ) || getCbf( uiIdx, TEXT_CHROMA_U, 0 ) || getCbf( uiIdx, TEXT_CHROMA_V, 0 ); }
  
  Void          setCbfSubParts        ( UInt uiCbfY, UInt uiCbfU, UInt uiCbfV, UInt uiAbsPartIdx, UInt uiDepth          );
  Void          setCbfSubParts        ( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiDepth                    );
#if HHI_MRG_SKIP
  Void          setCbfSubParts        ( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth    );
#endif
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for coding tool information
  // -------------------------------------------------------------------------------------------------------------------
  
  Bool*         getMergeFlag          ()                        { return m_pbMergeFlag;               }
  Bool          getMergeFlag          ( UInt uiIdx )            { return m_pbMergeFlag[uiIdx];        }
  Void          setMergeFlag          ( UInt uiIdx, Bool b )    { m_pbMergeFlag[uiIdx] = b;           }
  Void          setMergeFlagSubParts  ( Bool bMergeFlag, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );

  UChar*        getMergeIndex         ()                        { return m_puhMergeIndex;                         }
  UChar         getMergeIndex         ( UInt uiIdx )            { return m_puhMergeIndex[uiIdx];                  }
  Void          setMergeIndex         ( UInt uiIdx, UInt uiMergeIndex ) { m_puhMergeIndex[uiIdx] = uiMergeIndex;  }
  Void          setMergeIndexSubParts ( UInt uiMergeIndex, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );
#if !MRG_AMVP_FIXED_IDX_F470
  UChar*        getNeighbourCandIdx         ( UInt uiCandIdx )            { return m_apuhNeighbourCandIdx[uiCandIdx];           }
  UChar         getNeighbourCandIdx         ( UInt uiCandIdx, UInt uiIdx ){ return m_apuhNeighbourCandIdx[uiCandIdx][uiIdx];           }
  Void          setNeighbourCandIdx         ( UInt uiCandIdx, UInt uiIdx, UChar uhNeighCands ) { m_apuhNeighbourCandIdx[uiCandIdx][uiIdx] = uhNeighCands;}
  Void          setNeighbourCandIdxSubParts ( UInt uiCandIdx, UChar uhNumCands, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );
#endif
  template <typename T>
  Void          setSubPart            ( T bParameter, T* pbBaseLCU, UInt uiCUAddr, UInt uiCUDepth, UInt uiPUIdx );

#if AMP_MRG
  Void          setMergeAMP( Bool b )      { m_bIsMergeAMP = b; }
  Bool          getMergeAMP( )             { return m_bIsMergeAMP; }
#endif

  UChar*        getLumaIntraDir       ()                        { return m_puhLumaIntraDir;           }
  UChar         getLumaIntraDir       ( UInt uiIdx )            { return m_puhLumaIntraDir[uiIdx];    }
  Void          setLumaIntraDir       ( UInt uiIdx, UChar  uh ) { m_puhLumaIntraDir[uiIdx] = uh;      }
  Void          setLumaIntraDirSubParts( UInt uiDir,  UInt uiAbsPartIdx, UInt uiDepth );
  
  UChar*        getChromaIntraDir     ()                        { return m_puhChromaIntraDir;         }
  UChar         getChromaIntraDir     ( UInt uiIdx )            { return m_puhChromaIntraDir[uiIdx];  }
  Void          setChromaIntraDir     ( UInt uiIdx, UChar  uh ) { m_puhChromaIntraDir[uiIdx] = uh;    }
  Void          setChromIntraDirSubParts( UInt uiDir,  UInt uiAbsPartIdx, UInt uiDepth );
  
  UChar*        getInterDir           ()                        { return m_puhInterDir;               }
  UChar         getInterDir           ( UInt uiIdx )            { return m_puhInterDir[uiIdx];        }
  Void          setInterDir           ( UInt uiIdx, UChar  uh ) { m_puhInterDir[uiIdx] = uh;          }
  Void          setInterDirSubParts   ( UInt uiDir,  UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );
  
  Bool*         getAlfCtrlFlag        ()                        { return m_puiAlfCtrlFlag;            }
  Bool          getAlfCtrlFlag        ( UInt uiIdx )            { return m_puiAlfCtrlFlag[uiIdx];     }
  Void          setAlfCtrlFlag        ( UInt uiIdx, Bool uiFlag){ m_puiAlfCtrlFlag[uiIdx] = uiFlag;   }
  Void          setAlfCtrlFlagSubParts( Bool uiFlag, UInt uiAbsPartIdx, UInt uiDepth );
  
  Void          createTmpAlfCtrlFlag  ();
  Void          destroyTmpAlfCtrlFlag ();
  Void          copyAlfCtrlFlagToTmp  ();
  Void          copyAlfCtrlFlagFromTmp();
  
#if E057_INTRA_PCM
  Bool*         getIPCMFlag          ()                        { return m_pbIPCMFlag;               }
  Bool          getIPCMFlag          (UInt uiIdx )             { return m_pbIPCMFlag[uiIdx];        }
  Void          setIPCMFlag          (UInt uiIdx, Bool b )     { m_pbIPCMFlag[uiIdx] = b;           }
  Void          setIPCMFlagSubParts  (Bool bIpcmFlag, UInt uiAbsPartIdx, UInt uiDepth);
#endif

#if MTK_NONCROSS_INLOOP_FILTER
  /// get slice ID for SU
  Int           getSUSliceID          (UInt uiIdx)              {return m_piSliceSUMap[uiIdx];      } 

  /// get the pointer of slice ID map
  Int*          getSliceSUMap         ()                        {return m_piSliceSUMap;             }

  /// set the pointer of slice ID map
  Void          setSliceSUMap         (Int *pi)                 {m_piSliceSUMap = pi;               }
#endif


  // -------------------------------------------------------------------------------------------------------------------
  // member functions for accessing partition information
  // -------------------------------------------------------------------------------------------------------------------
  
  Void          getPartIndexAndSize   ( UInt uiPartIdx, UInt& ruiPartAddr, Int& riWidth, Int& riHeight );
  UChar         getNumPartInter       ();
  Bool          isFirstAbsZorderIdxInDepth (UInt uiAbsPartIdx, UInt uiDepth);
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for motion vector
  // -------------------------------------------------------------------------------------------------------------------
  
  Void          getMvField            ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList, TComMvField& rcMvField );
  
  AMVP_MODE     getAMVPMode           ( UInt uiIdx );
  Void          fillMvpCand           ( UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, AMVPInfo* pInfo );

  Void          setMVPIdx             ( RefPicList eRefPicList, UInt uiIdx, Int iMVPIdx)  { m_apiMVPIdx[eRefPicList][uiIdx] = iMVPIdx;  }
  Int           getMVPIdx             ( RefPicList eRefPicList, UInt uiIdx)               { return m_apiMVPIdx[eRefPicList][uiIdx];     }
  Char*         getMVPIdx             ( RefPicList eRefPicList )                          { return m_apiMVPIdx[eRefPicList];            }

  Void          setMVPNum             ( RefPicList eRefPicList, UInt uiIdx, Int iMVPNum ) { m_apiMVPNum[eRefPicList][uiIdx] = iMVPNum;  }
  Int           getMVPNum             ( RefPicList eRefPicList, UInt uiIdx )              { return m_apiMVPNum[eRefPicList][uiIdx];     }
  Char*         getMVPNum             ( RefPicList eRefPicList )                          { return m_apiMVPNum[eRefPicList];            }
  
  Void          setMVPIdxSubParts     ( Int iMVPIdx, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );
  Void          setMVPNumSubParts     ( Int iMVPNum, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth );
  
  Void          clipMv                ( TComMv&     rcMv     );
  Void          getMvPredLeft         ( TComMv&     rcMvPred )   { rcMvPred = m_cMvFieldA.getMv(); }
  Void          getMvPredAbove        ( TComMv&     rcMvPred )   { rcMvPred = m_cMvFieldB.getMv(); }
  Void          getMvPredAboveRight   ( TComMv&     rcMvPred )   { rcMvPred = m_cMvFieldC.getMv(); }
  
#if AMVP_BUFFERCOMPRESS
  Void          compressMV            ();
#endif 
  
  // -------------------------------------------------------------------------------------------------------------------
  // utility functions for neighbouring information
  // -------------------------------------------------------------------------------------------------------------------
  
  TComDataCU*   getCULeft                   () { return m_pcCULeft;       }
  TComDataCU*   getCUAbove                  () { return m_pcCUAbove;      }
  TComDataCU*   getCUAboveLeft              () { return m_pcCUAboveLeft;  }
  TComDataCU*   getCUAboveRight             () { return m_pcCUAboveRight; }
  TComDataCU*   getCUColocated              ( RefPicList eRefPicList ) { return m_apcCUColocated[eRefPicList]; }
  
  TComDataCU*   getPULeft                   ( UInt&  uiLPartUnitIdx , UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );
#if REDUCE_UPPER_MOTION_DATA
  TComDataCU*   getPUAbove                  ( UInt&  uiAPartUnitIdx , UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true, Bool MotionDataCompresssion = false );
  TComDataCU*   getPUAboveLeft              ( UInt&  uiALPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true, Bool MotionDataCompresssion = false );
  TComDataCU*   getPUAboveRight             ( UInt&  uiARPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true, Bool MotionDataCompresssion = false );
#else
  TComDataCU*   getPUAbove                  ( UInt&  uiAPartUnitIdx , UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );
  TComDataCU*   getPUAboveLeft              ( UInt&  uiALPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );
  TComDataCU*   getPUAboveRight             ( UInt&  uiARPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );
#endif
  TComDataCU*   getPUBelowLeft              ( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );

#if SUB_LCU_DQP
  TComDataCU*   getQpMinCuLeft                   ( UInt&  uiLPartUnitIdx , UInt uiCurrAbsIdxInLCU, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );
  UChar         getRefQP                         ( UInt   uiCurrAbsIdxInLCU                       );
#endif

  TComDataCU*   getPUAboveRightAdi          ( UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx, UInt uiPartUnitOffset = 1, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );
  TComDataCU*   getPUBelowLeftAdi           ( UInt& uiBLPartUnitIdx, UInt uiPuHeight, UInt uiCurrPartUnitIdx, UInt uiPartUnitOffset = 1, Bool bEnforceSliceRestriction=true, Bool bEnforceEntropySliceRestriction=true );
  
  Void          deriveLeftRightTopIdx       ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT );
  Void          deriveLeftBottomIdx         ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLB );
  
  Void          deriveLeftRightTopIdxAdi    ( UInt& ruiPartIdxLT, UInt& ruiPartIdxRT, UInt uiPartOffset, UInt uiPartDepth );
  Void          deriveLeftBottomIdxAdi      ( UInt& ruiPartIdxLB, UInt  uiPartOffset, UInt uiPartDepth );
  
  Bool          hasEqualMotion              ( UInt uiAbsPartIdx, TComDataCU* pcCandCU, UInt uiCandAbsPartIdx );
  Bool          avoidMergeCandidate         ( UInt uiAbsPartIdx, UInt uiPUIdx, UInt uiDepth, TComDataCU* pcCandCU, UInt uiCandAbsPartIdx );
#if MRG_AMVP_FIXED_IDX_F470
  Void          getInterMergeCandidates       ( UInt uiAbsPartIdx, UInt uiPUIdx, UInt uiDepth, TComMvField* pcMFieldNeighbours, UChar* puhInterDirNeighbours, Int& numValidMergeCand );
#else
  Void          getInterMergeCandidates       ( UInt uiAbsPartIdx, UInt uiPUIdx, UInt uiDepth, TComMvField* pcMFieldNeighbours, UChar* puhInterDirNeighbours, UInt* puiNeighbourCandIdx );
#endif
  Void          deriveLeftRightTopIdxGeneral  ( PartSize eCUMode, UInt uiAbsPartIdx, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT );
  Void          deriveLeftBottomIdxGeneral    ( PartSize eCUMode, UInt uiAbsPartIdx, UInt uiPartIdx, UInt& ruiPartIdxLB );
  
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for modes
  // -------------------------------------------------------------------------------------------------------------------
  
  Bool          isIntra   ( UInt uiPartIdx )  { return m_pePredMode[ uiPartIdx ] == MODE_INTRA; }
  Bool          isSkipped ( UInt uiPartIdx );                                                     ///< SKIP (no residual)
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for symbol prediction (most probable / mode conversion)
  // -------------------------------------------------------------------------------------------------------------------
  
  Int           getMostProbableIntraDirLuma     ( UInt uiAbsPartIdx                                       );
  
  UInt          getIntraSizeIdx                 ( UInt uiAbsPartIdx                                       );
  Void          convertTransIdx                 ( UInt uiAbsPartIdx, UInt uiTrIdx, UInt& ruiLumaTrMode, UInt& ruiChromaTrMode );
  
  Int           getLeftIntraDirLuma             ( UInt uiAbsPartIdx );
  Int           getAboveIntraDirLuma            ( UInt uiAbsPartIdx );

#if FIXED_MPM
  Void          getAllowedChromaDir             ( UInt uiAbsPartIdx, UInt* uiModeList );
  Int           getIntraDirLumaPredictor        ( UInt uiAbsPartIdx, Int* uiIntraDirPred, Int* piMode = NULL );
#elif MTK_DCM_MPM
  Int           getIntraDirLumaPredictor        ( UInt uiAbsPartIdx, Int uiIntraDirPred[]                 );
#endif

#if !AVOID_NEIGHBOR_REF_F470
  Bool          isSuroundingRefIdxException     ( UInt   uiAbsPartIdx );
#endif
  
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for SBAC context
  // -------------------------------------------------------------------------------------------------------------------
  
  UInt          getCtxSplitFlag                 ( UInt   uiAbsPartIdx, UInt uiDepth                   );
  UInt          getCtxQtCbf                     ( UInt   uiAbsPartIdx, TextType eType, UInt uiTrDepth );
  UInt          getCtxQtRootCbf                 ( UInt   uiAbsPartIdx                                 );
#if !DNB_REF_FRAME_IDX
  UInt          getCtxRefIdx                    ( UInt   uiAbsPartIdx, RefPicList eRefPicList         );
#endif
  UInt          getCtxSkipFlag                  ( UInt   uiAbsPartIdx                                 );
#if !DNB_ALF_CTRL_FLAG
  UInt          getCtxAlfCtrlFlag               ( UInt   uiAbsPartIdx                                 );
#endif
  UInt          getCtxInterDir                  ( UInt   uiAbsPartIdx                                 );
#if !DNB_INTRA_CHR_PRED_MODE
  UInt          getCtxIntraDirChroma            ( UInt   uiAbsPartIdx                                 );
#endif
#if !DNB_MERGE_FLAG
  UInt          getCtxMergeFlag                 ( UInt uiAbsPartIdx                                   );
#endif
  
#if FINE_GRANULARITY_SLICES
  UInt          getSliceStartCU         ( UInt pos )                  { return m_uiSliceStartCU[pos-m_uiAbsIdxInLCU];                                                                                          }
  UInt          getEntropySliceStartCU  ( UInt pos )                  { return m_uiEntropySliceStartCU[pos-m_uiAbsIdxInLCU];                                                                                   }
  UInt&         getTotalBins            ()                            { return m_uiTotalBins;                                                                                                  }
#else
  Void          setSliceStartCU  ( UInt uiStartCU )    { m_uiSliceStartCU = uiStartCU;    }  
  UInt          getSliceStartCU  ()                    { return m_uiSliceStartCU;         }
  Void          setEntropySliceStartCU ( UInt uiStartCU ) { m_uiEntropySliceStartCU = uiStartCU;     }  
  UInt          getEntropySliceStartCU ()                 { return m_uiEntropySliceStartCU;          }
#endif
  // -------------------------------------------------------------------------------------------------------------------
  // member functions for RD cost storage
  // -------------------------------------------------------------------------------------------------------------------
  
  Double&       getTotalCost()                  { return m_dTotalCost;        }
  UInt&         getTotalDistortion()            { return m_uiTotalDistortion; }
  UInt&         getTotalBits()                  { return m_uiTotalBits;       }
  UInt&         getTotalNumPart()               { return m_uiNumPartition;    }

#if QC_MDCS
  UInt          getCoefScanIdx(UInt uiAbsPartIdx, UInt uiWidth, Bool bIsLuma, Bool bIsIntra);
#endif //QC_MDCS

#if NSQT
  Bool useNonSquareTrans( UInt uiTrMode );
  Void getPixOffset( UInt uiTrMode, UInt ui, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiPix_X, UInt& uiPix_Y, TextType eTxt );
#endif

};

namespace RasterAddress
{
  /** Check whether 2 addresses point to the same column
   * \param addrA          First address in raster scan order
   * \param addrB          Second address in raters scan order
   * \param numUnitsPerRow Number of units in a row
   * \return Result of test
   */
  static inline Bool isEqualCol( Int addrA, Int addrB, Int numUnitsPerRow )
  {
    // addrA % numUnitsPerRow == addrB % numUnitsPerRow
    return (( addrA ^ addrB ) &  ( numUnitsPerRow - 1 ) ) == 0;
  }
  
  /** Check whether 2 addresses point to the same row
   * \param addrA          First address in raster scan order
   * \param addrB          Second address in raters scan order
   * \param numUnitsPerRow Number of units in a row
   * \return Result of test
   */
  static inline Bool isEqualRow( Int addrA, Int addrB, Int numUnitsPerRow )
  {
    // addrA / numUnitsPerRow == addrB / numUnitsPerRow
    return (( addrA ^ addrB ) &~ ( numUnitsPerRow - 1 ) ) == 0;
  }
  
  /** Check whether 2 addresses point to the same row or column
   * \param addrA          First address in raster scan order
   * \param addrB          Second address in raters scan order
   * \param numUnitsPerRow Number of units in a row
   * \return Result of test
   */
  static inline Bool isEqualRowOrCol( Int addrA, Int addrB, Int numUnitsPerRow )
  {
    return isEqualCol( addrA, addrB, numUnitsPerRow ) | isEqualRow( addrA, addrB, numUnitsPerRow );
  }
  
  /** Check whether one address points to the first column
   * \param addr           Address in raster scan order
   * \param numUnitsPerRow Number of units in a row
   * \return Result of test
   */
  static inline Bool isZeroCol( Int addr, Int numUnitsPerRow )
  {
    // addr % numUnitsPerRow == 0
    return ( addr & ( numUnitsPerRow - 1 ) ) == 0;
  }
  
  /** Check whether one address points to the first row
   * \param addr           Address in raster scan order
   * \param numUnitsPerRow Number of units in a row
   * \return Result of test
   */
  static inline Bool isZeroRow( Int addr, Int numUnitsPerRow )
  {
    // addr / numUnitsPerRow == 0
    return ( addr &~ ( numUnitsPerRow - 1 ) ) == 0;
  }
  
  /** Check whether one address points to a column whose index is smaller than a given value
   * \param addr           Address in raster scan order
   * \param val            Given column index value
   * \param numUnitsPerRow Number of units in a row
   * \return Result of test
   */
  static inline Bool lessThanCol( Int addr, Int val, Int numUnitsPerRow )
  {
    // addr % numUnitsPerRow < val
    return ( addr & ( numUnitsPerRow - 1 ) ) < val;
  }
  
  /** Check whether one address points to a row whose index is smaller than a given value
   * \param addr           Address in raster scan order
   * \param val            Given row index value
   * \param numUnitsPerRow Number of units in a row
   * \return Result of test
   */
  static inline Bool lessThanRow( Int addr, Int val, Int numUnitsPerRow )
  {
    // addr / numUnitsPerRow < val
    return addr < val * numUnitsPerRow;
  }
};

//! \}

#endif
