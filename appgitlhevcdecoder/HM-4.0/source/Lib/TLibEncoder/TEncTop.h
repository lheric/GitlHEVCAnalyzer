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

/** \file     TEncTop.h
    \brief    encoder class (header)
*/

#ifndef __TENCTOP__
#define __TENCTOP__

// Include files
#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPrediction.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/AccessUnit.h"

#include "TLibVideoIO/TVideoIOYuv.h"

#include "TEncCfg.h"
#include "TEncGOP.h"
#include "TEncSlice.h"
#include "TEncEntropy.h"
#include "TEncCavlc.h"
#include "TEncSbac.h"
#include "TEncSearch.h"
#include "TEncAdaptiveLoopFilter.h"
#if QP_ADAPTATION
#include "TEncPreanalyzer.h"
#endif

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// encoder class
class TEncTop : public TEncCfg
{
private:
  // picture
  Int                     m_iPOCLast;                     ///< time index (POC)
  Int                     m_iNumPicRcvd;                  ///< number of received pictures
  UInt                    m_uiNumAllPicCoded;             ///< number of coded pictures
  TComList<TComPic*>      m_cListPic;                     ///< dynamic list of pictures
  
  // encoder search
  TEncSearch              m_cSearch;                      ///< encoder search class
  TEncEntropy*            m_pcEntropyCoder;                     ///< entropy encoder 
  TEncCavlc*              m_pcCavlcCoder;                       ///< CAVLC encoder  
  // coding tool
  TComTrQuant             m_cTrQuant;                     ///< transform & quantization class
  TComLoopFilter          m_cLoopFilter;                  ///< deblocking filter class
#if MTK_SAO
  TEncSampleAdaptiveOffset  m_cEncSAO;                    ///< sample adaptive offset class
#endif
  TEncAdaptiveLoopFilter  m_cAdaptiveLoopFilter;          ///< adaptive loop filter class
  TEncEntropy             m_cEntropyCoder;                ///< entropy encoder
  TEncCavlc               m_cCavlcCoder;                  ///< CAVLC encoder
  TEncSbac                m_cSbacCoder;                   ///< SBAC encoder
  TEncBinCABAC            m_cBinCoderCABAC;               ///< bin coder CABAC
  
  // processing unit
  TEncGOP                 m_cGOPEncoder;                  ///< GOP encoder
  TEncSlice               m_cSliceEncoder;                ///< slice encoder
  TEncCu                  m_cCuEncoder;                   ///< CU encoder
  // SPS
  TComSPS                 m_cSPS;                         ///< SPS
  TComPPS                 m_cPPS;                         ///< PPS
  
  // RD cost computation
  TComBitCounter          m_cBitCounter;                  ///< bit counter for RD optimization
  TComRdCost              m_cRdCost;                      ///< RD cost computation class
  TEncSbac***             m_pppcRDSbacCoder;              ///< temporal storage for RD computation
  TEncSbac                m_cRDGoOnSbacCoder;             ///< going on SBAC model for RD stage
  TEncBinCABAC***         m_pppcBinCoderCABAC;            ///< temporal CABAC state storage for RD computation
  TEncBinCABAC            m_cRDGoOnBinCoderCABAC;         ///< going on bin coder CABAC for RD stage
  
#if QP_ADAPTATION
  // quality control
  TEncPreanalyzer         m_cPreanalyzer;                 ///< image characteristics analyzer for TM5-step3-like adaptive QP
#endif

protected:
  Void  xGetNewPicBuffer  ( TComPic*& rpcPic );           ///< get picture buffer which will be processed
  Void  xInitSPS          ();                             ///< initialize SPS from encoder options
  Void  xInitPPS          ();                             ///< initialize PPS from encoder options
  
public:
  TEncTop();
  virtual ~TEncTop();
  
  Void      create          ();
  Void      destroy         ();
  Void      init            ();
  Void      deletePicBuffer ();
  
  // -------------------------------------------------------------------------------------------------------------------
  // member access functions
  // -------------------------------------------------------------------------------------------------------------------
  
  TComList<TComPic*>*     getListPic            () { return  &m_cListPic;             }
  TEncSearch*             getPredSearch         () { return  &m_cSearch;              }
  
  TComTrQuant*            getTrQuant            () { return  &m_cTrQuant;             }
  TComLoopFilter*         getLoopFilter         () { return  &m_cLoopFilter;          }
  TEncAdaptiveLoopFilter* getAdaptiveLoopFilter () { return  &m_cAdaptiveLoopFilter;  }
#if MTK_SAO
  TEncSampleAdaptiveOffset* getSAO                () { return  &m_cEncSAO;              }
#endif
  TEncGOP*                getGOPEncoder         () { return  &m_cGOPEncoder;          }
  TEncSlice*              getSliceEncoder       () { return  &m_cSliceEncoder;        }
  TEncCu*                 getCuEncoder          () { return  &m_cCuEncoder;           }
  TEncEntropy*            getEntropyCoder       () { return  &m_cEntropyCoder;        }
  TEncCavlc*              getCavlcCoder         () { return  &m_cCavlcCoder;          }
  TEncSbac*               getSbacCoder          () { return  &m_cSbacCoder;           }
  TEncBinCABAC*           getBinCABAC           () { return  &m_cBinCoderCABAC;       }
  
  TComBitCounter*         getBitCounter         () { return  &m_cBitCounter;          }
  TComRdCost*             getRdCost             () { return  &m_cRdCost;              }
  TEncSbac***             getRDSbacCoder        () { return  m_pppcRDSbacCoder;       }
  TEncSbac*               getRDGoOnSbacCoder    () { return  &m_cRDGoOnSbacCoder;     }
  
  TComSPS*                getSPS                () { return  &m_cSPS;                 }
  TComPPS*                getPPS                () { return  &m_cPPS;                 }
  
  // -------------------------------------------------------------------------------------------------------------------
  // encoder function
  // -------------------------------------------------------------------------------------------------------------------
  
  /// encode several number of pictures until end-of-sequence
  Void encode( bool bEos, TComPicYuv* pcPicYuvOrg, TComList<TComPicYuv*>& rcListPicYuvRecOut,
              std::list<AccessUnit>& accessUnitsOut, Int& iNumEncoded );
  
};

//! \}

#endif // __TENCTOP__
