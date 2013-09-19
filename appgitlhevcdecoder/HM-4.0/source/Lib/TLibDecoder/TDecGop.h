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

/** \file     TDecGop.h
    \brief    GOP decoder class (header)
*/

#ifndef __TDECGOP__
#define __TDECGOP__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TLibCommon/CommonDef.h"
#include "TLibCommon/TComBitStream.h"
#include "TLibCommon/TComList.h"
#include "TLibCommon/TComPicYuv.h"
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComLoopFilter.h"
#include "TLibCommon/TComAdaptiveLoopFilter.h"

#include "TDecEntropy.h"
#include "TDecSlice.h"
#include "TDecBinCoder.h"
#include "TDecBinCoderCABAC.h"

//! \ingroup TLibDecoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// GOP decoder class
class TDecGop
{
private:
  Int                   m_iGopSize;
  TComList<TComPic*>    m_cListPic;         //  Dynamic buffer
  
  //  Access channel
  TDecEntropy*          m_pcEntropyDecoder;
  TDecSbac*             m_pcSbacDecoder;
  TDecBinCABAC*         m_pcBinCABAC;
  TDecCavlc*            m_pcCavlcDecoder;
  TDecSlice*            m_pcSliceDecoder;
  TComLoopFilter*       m_pcLoopFilter;
  
  // Adaptive Loop filter
  TComAdaptiveLoopFilter*       m_pcAdaptiveLoopFilter;
#if MTK_SAO
  TComSampleAdaptiveOffset*              m_pcSAO;
  SAOParam              m_cSaoParam;
#endif
  ALFParam              m_cAlfParam;
  Double                m_dDecTime;

  bool m_pictureDigestEnabled; ///< if true, handle picture_digest SEI messages

public:
  TDecGop();
  virtual ~TDecGop();
  
  Void  init    ( TDecEntropy*            pcEntropyDecoder, 
                 TDecSbac*               pcSbacDecoder, 
                 TDecBinCABAC*           pcBinCABAC,
                 TDecCavlc*              pcCavlcDecoder, 
                 TDecSlice*              pcSliceDecoder, 
                 TComLoopFilter*         pcLoopFilter, 
                 TComAdaptiveLoopFilter* pcAdaptiveLoopFilter
#if MTK_SAO
                 ,TComSampleAdaptiveOffset*                pcSAO
#endif
                 );
  Void  create  ();
  Void  destroy ();
#if REF_SETTING_FOR_LD
  Void  decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf, TComList<TComPic*>& rcListPic );
#else
  Void  decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf );
#endif
  Void  setGopSize( Int i) { m_iGopSize = i; }

  void setPictureDigestEnabled(bool enabled) { m_pictureDigestEnabled = enabled; }

#if E045_SLICE_COMMON_INFO_SHARING
private:
  /// copy shared ALF parameters from PPS
  Void copySharedAlfParamFromPPS(ALFParam* pAlfDst, ALFParam* pAlfSrc);
#endif

};

//! \}

#endif // !defined(AFX_TDECGOP_H__29440B7A_7CC0_48C7_8DD5_1A531D3CED45__INCLUDED_)

