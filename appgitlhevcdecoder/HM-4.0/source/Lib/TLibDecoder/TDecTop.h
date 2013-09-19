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

/** \file     TDecTop.h
    \brief    decoder class (header)
*/

#ifndef __TDECTOP__
#define __TDECTOP__

#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPicYuv.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/SEI.h"

#include "TDecGop.h"
#include "TDecEntropy.h"
#include "TDecSbac.h"
#include "TDecCAVLC.h"

struct InputNALUnit;

//! \ingroup TLibDecoder
//! \{

#if E045_SLICE_COMMON_INFO_SHARING
#define MAX_NUM_PPS 1
#define MAX_NUM_PPS_BUFFER (MAX_NUM_PPS +1)
#endif


// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// decoder class
class TDecTop
{
private:
  Int                     m_iGopSize;
  Bool                    m_bGopSizeSet;
  int                     m_iMaxRefPicNum;
  
  Bool                    m_bRefreshPending;    ///< refresh pending flag
  UInt                    m_uiPOCCDR;           ///< temporal reference of the CDR picture
  UInt                    m_uiPOCRA;            ///< temporal reference of the random access point

  UInt                    m_uiValidPS;
  TComList<TComPic*>      m_cListPic;         //  Dynamic buffer
  TComSPS                 m_cSPS;
#if E045_SLICE_COMMON_INFO_SHARING
  TComPPS*                m_pcPPS;       //!< PPS
  TComPPS*                m_pcPPSBuffer; //!< PPS buffer
  Bool*                   m_pbHasNewPPS; //!< Availability for each PPS in PPS buffer
  Int                     m_iPPSCounter; //!< PPS counter
#else
  TComPPS                 m_cPPS;
#endif
  TComSlice*              m_apcSlicePilot;
  
  SEImessages *m_SEIs; ///< "all" SEI messages.  If not NULL, we own the object.

  // functional classes
  TComPrediction          m_cPrediction;
  TComTrQuant             m_cTrQuant;
  TDecGop                 m_cGopDecoder;
  TDecSlice               m_cSliceDecoder;
  TDecCu                  m_cCuDecoder;
  TDecEntropy             m_cEntropyDecoder;
  TDecCavlc               m_cCavlcDecoder;
  TDecSbac                m_cSbacDecoder;
  TDecBinCABAC            m_cBinCABAC;
  TComLoopFilter          m_cLoopFilter;
  TComAdaptiveLoopFilter  m_cAdaptiveLoopFilter;
#if MTK_SAO
  TComSampleAdaptiveOffset m_cSAO;
#endif

  Bool isRandomAccessSkipPicture(Int& iSkipFrame,  Int& iPOCLastDisplay);
  TComPic*                m_pcPic;
  UInt                    m_uiSliceIdx;
  UInt                    m_uiLastSliceIdx;
  UInt                    m_uiPrevPOC;
  Bool                    m_bFirstSliceInPicture;
  Bool                    m_bFirstSliceInSequence;

public:
  TDecTop();
  virtual ~TDecTop();
  
  Void  create  ();
  Void  destroy ();

  void setPictureDigestEnabled(bool enabled) { m_cGopDecoder.setPictureDigestEnabled(enabled); }
  
  Void  init();
  Bool  decode(InputNALUnit& nalu, Int& iSkipFrame, Int& iPOCLastDisplay);
  
  TComSPS *getSPS() { return (m_uiValidPS & 1) ? &m_cSPS : NULL; }
  
  Void  deletePicBuffer();

  Void executeDeblockAndAlf(UInt& ruiPOC, TComList<TComPic*>*& rpcListPic, Int& iSkipFrame,  Int& iPOCLastDisplay);

protected:
  Void  xGetNewPicBuffer  (TComSlice* pcSlice, TComPic*& rpcPic);
  Void  xUpdateGopSize    (TComSlice* pcSlice);

#if E045_SLICE_COMMON_INFO_SHARING
  /// create PPS buffer
  Void     createPPSBuffer      ();
  /// destroy PPS buffer
  Void     destroyPPSBuffer     ();
  /// signal the PPS availability
  Void     signalNewPPSAvailable();
  /// update PPS buffer counter
  Void     updatePPSBuffer      ();
  /// get new PPS buffer for the coming PPS
  TComPPS* getNewPPSBuffer      ();
  /// get PPS pointer
  TComPPS* getPPS               ()   {return m_pcPPS;}
  /// signal if the PPS is available
  Bool     hasNewPPS            ()   {return m_pbHasNewPPS[m_iPPSCounter];}
#endif
  
};// END CLASS DEFINITION TDecTop


//! \}

#endif // __TDECTOP__

