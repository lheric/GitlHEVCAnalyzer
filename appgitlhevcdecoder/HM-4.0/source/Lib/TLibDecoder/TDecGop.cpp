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

/** \file     TDecGop.cpp
    \brief    GOP decoder class
*/

extern bool g_md5_mismatch; ///< top level flag to signal when there is a decode problem

#include "TDecGop.h"
#include "TDecCAVLC.h"
#include "TDecSbac.h"
#include "TDecBinCoder.h"
#include "TDecBinCoderCABAC.h"
#include "libmd5/MD5.h"
#include "TLibCommon/SEI.h"

#include <time.h>

//! \ingroup TLibDecoder
//! \{

static void calcAndPrintMD5Status(TComPicYuv& pic, const SEImessages* seis);

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TDecGop::TDecGop()
{
  m_iGopSize = 0;
  m_dDecTime = 0;
}

TDecGop::~TDecGop()
{
  
}

Void TDecGop::create()
{
  
}


Void TDecGop::destroy()
{

}

Void TDecGop::init( TDecEntropy*            pcEntropyDecoder, 
                   TDecSbac*               pcSbacDecoder, 
                   TDecBinCABAC*           pcBinCABAC,
                   TDecCavlc*              pcCavlcDecoder, 
                   TDecSlice*              pcSliceDecoder, 
                   TComLoopFilter*         pcLoopFilter, 
                   TComAdaptiveLoopFilter* pcAdaptiveLoopFilter 
#if MTK_SAO
                   ,TComSampleAdaptiveOffset* pcSAO
#endif                   
                   )
{
  m_pcEntropyDecoder      = pcEntropyDecoder;
  m_pcSbacDecoder         = pcSbacDecoder;
  m_pcBinCABAC            = pcBinCABAC;
  m_pcCavlcDecoder        = pcCavlcDecoder;
  m_pcSliceDecoder        = pcSliceDecoder;
  m_pcLoopFilter          = pcLoopFilter;
  m_pcAdaptiveLoopFilter  = pcAdaptiveLoopFilter;
#if MTK_SAO
  m_pcSAO  = pcSAO;
#endif

}


// ====================================================================================================================
// Private member functions
// ====================================================================================================================
#if E045_SLICE_COMMON_INFO_SHARING
Void TDecGop::copySharedAlfParamFromPPS(ALFParam* pAlfDst, ALFParam* pAlfSrc)
{
  pAlfDst->alf_flag = pAlfSrc->alf_flag;
  if (!pAlfDst->alf_flag)
  {
    return;
  }

#if MQT_BA_RA
  pAlfDst->alf_pcr_region_flag= pAlfSrc->alf_pcr_region_flag;  
#endif  
  pAlfDst->filters_per_group  = pAlfSrc->filters_per_group;
  pAlfDst->filtNo             = pAlfSrc->filtNo;
  pAlfDst->realfiltNo         = pAlfSrc->realfiltNo;
#if !STAR_CROSS_SHAPES_LUMA
  pAlfDst->tap                = pAlfSrc->tap;
#if TI_ALF_MAX_VSIZE_7
  pAlfDst->tapV               = pAlfSrc->tapV;
#endif
#endif
  pAlfDst->num_coeff          = pAlfSrc->num_coeff;
  pAlfDst->noFilters          = pAlfSrc->noFilters;
  pAlfDst->startSecondFilter  = pAlfSrc->startSecondFilter;
  ::memcpy(pAlfDst->filterPattern, pAlfSrc->filterPattern, sizeof(Int)*NO_VAR_BINS);
  ::memcpy(pAlfDst->varIndTab,     pAlfSrc->varIndTab,     sizeof(Int)*NO_VAR_BINS);

  pAlfDst->filters_per_group_diff = pAlfSrc->filters_per_group_diff;
  pAlfDst->forceCoeff0            = pAlfSrc->forceCoeff0;
  pAlfDst->predMethod             = pAlfSrc->predMethod;
  ::memcpy(pAlfDst->codedVarBins, pAlfSrc->codedVarBins, sizeof(Int)*NO_VAR_BINS);


  pAlfDst->minKStart              = pAlfSrc->minKStart;
  ::memcpy(pAlfDst->kMinTab,       pAlfSrc->kMinTab, sizeof(Int)*42);

  for (Int i=0; i<NO_VAR_BINS; i++)
  {
    ::memcpy(pAlfDst->coeffmulti[i], pAlfSrc->coeffmulti[i], sizeof(Int)*ALF_MAX_NUM_COEF);
  }

  pAlfDst->chroma_idc         = pAlfSrc->chroma_idc;
  if(pAlfDst->chroma_idc)
  {
#if ALF_CHROMA_NEW_SHAPES
    pAlfDst->realfiltNo_chroma = pAlfSrc->realfiltNo_chroma;
#else
    pAlfDst->tap_chroma       = pAlfSrc->tap_chroma;
#endif
    pAlfDst->num_coeff_chroma = pAlfSrc->num_coeff_chroma;
    ::memcpy(pAlfDst->coeff_chroma, pAlfSrc->coeff_chroma, sizeof(Int)*pAlfDst->num_coeff_chroma);
  }
}
#endif


// ====================================================================================================================
// Public member functions
// ====================================================================================================================
#if REF_SETTING_FOR_LD
Void TDecGop::decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf, TComList<TComPic*>& rcListPic )
#else
Void TDecGop::decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf)
#endif
{
  TComSlice*  pcSlice = rpcPic->getSlice(rpcPic->getCurrSliceIdx());

  //-- For time output for each slice
  long iBeforeTime = clock();
  
  UInt uiStartCUAddr   = pcSlice->getEntropySliceCurStartCUAddr();
#if MTK_NONCROSS_INLOOP_FILTER
  static Bool  bFirst = true;
  static UInt  uiILSliceCount;
  static UInt* puiILSliceStartLCU;
#endif

  if (!bExecuteDeblockAndAlf)
  {
#if MTK_NONCROSS_INLOOP_FILTER
    if(bFirst)
    {
      uiILSliceCount = 0;
      puiILSliceStartLCU = new UInt[(rpcPic->getNumCUsInFrame()* rpcPic->getNumPartInCU()) +1];
      bFirst = false;
    }

    UInt uiSliceStartCuAddr = pcSlice->getSliceCurStartCUAddr();
    if(uiSliceStartCuAddr == uiStartCUAddr)
    {
      puiILSliceStartLCU[uiILSliceCount] = uiSliceStartCuAddr;
      uiILSliceCount++;
    }
#endif //MTK_NONCROSS_INLOOP_FILTER

    UInt iSymbolMode = pcSlice->getSymbolMode();
    if (iSymbolMode)
    {
      m_pcSbacDecoder->init( (TDecBinIf*)m_pcBinCABAC );
      m_pcEntropyDecoder->setEntropyDecoder (m_pcSbacDecoder);
    }
    else
    {
      m_pcEntropyDecoder->setEntropyDecoder (m_pcCavlcDecoder);
    }
    
    m_pcEntropyDecoder->setBitstream      (pcBitstream);
    m_pcEntropyDecoder->resetEntropy      (pcSlice);
    
    if (uiStartCUAddr==0)  // decode SAO params only from first slice header
    {
#if MTK_SAO
      if( rpcPic->getSlice(0)->getSPS()->getUseSAO() )
      {  
        m_pcSAO->InitSao(&m_cSaoParam);
        m_pcEntropyDecoder->decodeSaoParam(&m_cSaoParam);
      }
#endif

      if ( rpcPic->getSlice(0)->getSPS()->getUseALF() )
      {
#if (!E045_SLICE_COMMON_INFO_SHARING)
        m_pcAdaptiveLoopFilter->setNumCUsInFrame(rpcPic);
#endif
        m_pcAdaptiveLoopFilter->allocALFParam(&m_cAlfParam);
#if !E045_SLICE_COMMON_INFO_SHARING
#if FINE_GRANULARITY_SLICES && MTK_NONCROSS_INLOOP_FILTER
        m_pcEntropyDecoder->setSliceGranularity(pcSlice->getPPS()->getSliceGranularity());
#endif
        m_pcEntropyDecoder->decodeAlfParam( &m_cAlfParam );
#endif
      }
    }
   
#if E045_SLICE_COMMON_INFO_SHARING
    if(uiSliceStartCuAddr == uiStartCUAddr)
    {
      if( pcSlice->getSPS()->getUseALF())
      {
        if(!pcSlice->getPPS()->getSharedPPSInfoEnabled())
        {
          m_pcEntropyDecoder->decodeAlfParam( &m_cAlfParam );
        }
        else
        {
          copySharedAlfParamFromPPS(&m_cAlfParam, pcSlice->getPPS()->getSharedAlfParam());
        }
        m_pcEntropyDecoder->decodeAlfCtrlParam( &m_cAlfParam, (uiStartCUAddr==0));
      }
    }
#endif

    m_pcSliceDecoder->decompressSlice(pcBitstream, rpcPic);
    
    m_dDecTime += (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
  }
  else
  {
    // deblocking filter
    m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), 0, 0);
    m_pcLoopFilter->loopFilterPic( rpcPic );
#if MTK_SAO
    {

#if MTK_SAO && MTK_NONCROSS_INLOOP_FILTER && FINE_GRANULARITY_SLICES 
      if( pcSlice->getSPS()->getUseSAO() )
      {
        m_pcSAO->setNumSlicesInPic( uiILSliceCount );
        m_pcSAO->setSliceGranularityDepth(pcSlice->getPPS()->getSliceGranularity());
        if(uiILSliceCount == 1)
        {
          m_pcSAO->setUseNonCrossAlf(false);
        }
        else
        {
          if(m_cSaoParam.bSaoFlag)
          {
            m_pcSAO->setPic(rpcPic);
            puiILSliceStartLCU[uiILSliceCount] = rpcPic->getNumCUsInFrame()* rpcPic->getNumPartInCU();
            m_pcSAO->setUseNonCrossAlf(!pcSlice->getSPS()->getLFCrossSliceBoundaryFlag());
            if (m_pcSAO->getUseNonCrossAlf())
            {
              m_pcSAO->InitIsFineSliceCu();

              for(UInt i=0; i< uiILSliceCount ; i++)
              {
                UInt uiStartAddr = puiILSliceStartLCU[i];
                UInt uiEndAddr   = puiILSliceStartLCU[i+1]-1;
                m_pcSAO->createSliceMap(i, uiStartAddr, uiEndAddr);
              }
            }
          }
        }
      }
#endif
      if( rpcPic->getSlice(0)->getSPS()->getUseSAO())
      {

        m_pcSAO->SAOProcess(rpcPic, &m_cSaoParam);  

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX
        m_pcAdaptiveLoopFilter->PCMLFDisableProcess(rpcPic);
#endif
      }
    }
#endif


    // adaptive loop filter
    if( pcSlice->getSPS()->getUseALF() )
    {
#if MTK_NONCROSS_INLOOP_FILTER  
      m_pcAdaptiveLoopFilter->setNumSlicesInPic( uiILSliceCount );
#if FINE_GRANULARITY_SLICES
      m_pcAdaptiveLoopFilter->setSliceGranularityDepth(pcSlice->getPPS()->getSliceGranularity());
#endif
      if(uiILSliceCount == 1)
      {
        m_pcAdaptiveLoopFilter->setUseNonCrossAlf(false);
      }
      else
      {
#if FINE_GRANULARITY_SLICES
        puiILSliceStartLCU[uiILSliceCount] = rpcPic->getNumCUsInFrame()* rpcPic->getNumPartInCU();
#else
        puiILSliceStartLCU[uiILSliceCount] = rpcPic->getNumCUsInFrame();
#endif
        m_pcAdaptiveLoopFilter->setUseNonCrossAlf(!pcSlice->getSPS()->getLFCrossSliceBoundaryFlag());
        m_pcAdaptiveLoopFilter->createSlice(rpcPic);

        for(UInt i=0; i< uiILSliceCount ; i++)
        {
#if FINE_GRANULARITY_SLICES
          UInt uiStartAddr = puiILSliceStartLCU[i];
          UInt uiEndAddr   = puiILSliceStartLCU[i+1]-1;
#else
          UInt uiStartAddr = (puiILSliceStartLCU[i]*rpcPic->getNumPartInCU());
          UInt uiEndAddr   = (puiILSliceStartLCU[i+1]*rpcPic->getNumPartInCU())-1;

#endif
          (*m_pcAdaptiveLoopFilter)[i].create(i, uiStartAddr, uiEndAddr);
        }
      }
#endif
      m_pcAdaptiveLoopFilter->ALFProcess(rpcPic, &m_cAlfParam);

#if E057_INTRA_PCM && E192_SPS_PCM_FILTER_DISABLE_SYNTAX
      m_pcAdaptiveLoopFilter->PCMLFDisableProcess(rpcPic);
#endif

#if MTK_NONCROSS_INLOOP_FILTER
      if(uiILSliceCount > 1)
      {
        m_pcAdaptiveLoopFilter->destroySlice();
      }
#endif
      m_pcAdaptiveLoopFilter->freeALFParam(&m_cAlfParam);
    }
    
#if AMVP_BUFFERCOMPRESS
    rpcPic->compressMotion(); 
#endif 
    Char c = (pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B');
    if (!pcSlice->isReferenced()) c += 32;
    
    //-- For time output for each slice
    printf("\nPOC %4d TId: %1d ( %c-SLICE, QP%3d ) ",
          pcSlice->getPOC(),
          pcSlice->getTLayer(),
          c,
          pcSlice->getSliceQp() );

    m_dDecTime += (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
    printf ("[DT %6.3f] ", m_dDecTime );
    m_dDecTime  = 0;
    
    for (Int iRefList = 0; iRefList < 2; iRefList++)
    {
      printf ("[L%d ", iRefList);
      for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
      {
        printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex));
      }
      printf ("] ");
    }
    if(pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0 && !pcSlice->getNoBackPredFlag())
    {
      printf ("[LC ");
      for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(REF_PIC_LIST_C); iRefIndex++)
      {
        printf ("%d ", pcSlice->getRefPOC((RefPicList)pcSlice->getListIdFromIdxOfLC(iRefIndex), pcSlice->getRefIdxFromIdxOfLC(iRefIndex)));
      }
      printf ("] ");
    }

    if (m_pictureDigestEnabled)
    {
      calcAndPrintMD5Status(*rpcPic->getPicYuvRec(), rpcPic->getSEIs());
    }

#if FIXED_ROUNDING_FRAME_MEMORY
    rpcPic->getPicYuvRec()->xFixedRoundingPic();
#endif

    rpcPic->setReconMark(true);

#if REF_SETTING_FOR_LD
      if ( rpcPic->getSlice(0)->getSPS()->getUseNewRefSetting() )
      {
        if ( rpcPic->getSlice(0)->isReferenced() )
        {
          rpcPic->getSlice(0)->decodingRefMarkingForLD( rcListPic, rpcPic->getSlice(0)->getSPS()->getMaxNumRefFrames(), rpcPic->getSlice(0)->getPOC() );
        }
      }
#endif

#if MTK_NONCROSS_INLOOP_FILTER
    uiILSliceCount = 0;
#endif
  }
}

/**
 * Calculate and print MD5 for pic, compare to picture_digest SEI if
 * present in seis.  seis may be NULL.  MD5 is printed to stdout, in
 * a manner suitable for the status line. Theformat is:
 *  [MD5:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx,(yyy)]
 * Where, x..x is the md5
 *        yyy has the following meanings:
 *            OK          - calculated MD5 matches the SEI message
 *            ***ERROR*** - calculated MD5 does not match the SEI message
 *            unk         - no SEI message was available for comparison
 */
static void calcAndPrintMD5Status(TComPicYuv& pic, const SEImessages* seis)
{
  /* calculate MD5sum for entire reconstructed picture */
  unsigned char recon_digest[16];
  calcMD5(pic, recon_digest);

  /* compare digest against received version */
  const char* md5_ok = "(unk)";
  bool md5_mismatch = false;

  if (seis && seis->picture_digest)
  {
    md5_ok = "(OK)";
    for (unsigned i = 0; i < 16; i++)
    {
      if (recon_digest[i] != seis->picture_digest->digest[i])
      {
        md5_ok = "(***ERROR***)";
        md5_mismatch = true;
      }
    }
  }

  printf("[MD5:%s,%s] ", digestToString(recon_digest), md5_ok);
  if (md5_mismatch)
  {
    g_md5_mismatch = true;
    printf("[rxMD5:%s] ", digestToString(seis->picture_digest->digest));
  }
}
//! \}
