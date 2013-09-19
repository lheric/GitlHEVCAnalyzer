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
  m_pcSbacDecoders = NULL;
  m_pcBinCABACs = NULL;
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
                   ,TComSampleAdaptiveOffset* pcSAO
                   )
{
  m_pcEntropyDecoder      = pcEntropyDecoder;
  m_pcSbacDecoder         = pcSbacDecoder;
  m_pcBinCABAC            = pcBinCABAC;
  m_pcCavlcDecoder        = pcCavlcDecoder;
  m_pcSliceDecoder        = pcSliceDecoder;
  m_pcLoopFilter          = pcLoopFilter;
  m_pcAdaptiveLoopFilter  = pcAdaptiveLoopFilter;
  m_pcSAO  = pcSAO;
}


// ====================================================================================================================
// Private member functions
// ====================================================================================================================

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TDecGop::decompressGop(TComInputBitstream* pcBitstream, TComPic*& rpcPic, Bool bExecuteDeblockAndAlf)
{
  TComSlice*  pcSlice = rpcPic->getSlice(rpcPic->getCurrSliceIdx());
  // Table of extracted substreams.
  // These must be deallocated AND their internal fifos, too.
  TComInputBitstream **ppcSubstreams = NULL;

  //-- For time output for each slice
  long iBeforeTime = clock();
  
  UInt uiStartCUAddr   = pcSlice->getEntropySliceCurStartCUAddr();
  static Bool  bFirst = true;
  static UInt  uiILSliceCount;
  static UInt* puiILSliceStartLCU;
  static std::vector<AlfCUCtrlInfo> vAlfCUCtrlSlices;

  if (!bExecuteDeblockAndAlf)
  {
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

    m_pcSbacDecoder->init( (TDecBinIf*)m_pcBinCABAC );
    m_pcEntropyDecoder->setEntropyDecoder (m_pcSbacDecoder);
    
    UInt uiNumSubstreams = pcSlice->getPPS()->getNumSubstreams();

    //init each couple {EntropyDecoder, Substream}
    UInt *puiSubstreamSizes = pcSlice->getSubstreamSizes();
    ppcSubstreams    = new TComInputBitstream*[uiNumSubstreams];
    m_pcSbacDecoders = new TDecSbac[uiNumSubstreams];
    m_pcBinCABACs    = new TDecBinCABAC[uiNumSubstreams];
    UInt uiBitsRead = pcBitstream->getByteLocation()<<3;
    for ( UInt ui = 0 ; ui < uiNumSubstreams ; ui++ )
    {
      m_pcSbacDecoders[ui].init(&m_pcBinCABACs[ui]);
      UInt uiSubstreamSizeBits = (ui+1 < uiNumSubstreams ? puiSubstreamSizes[ui] : pcBitstream->getNumBitsLeft());
      ppcSubstreams[ui] = pcBitstream->extractSubstream(ui+1 < uiNumSubstreams ? puiSubstreamSizes[ui] : pcBitstream->getNumBitsLeft());
      // update location information from where tile markers were extracted
      {
        UInt uiDestIdx       = 0;
        for (UInt uiSrcIdx = 0; uiSrcIdx<pcBitstream->getTileMarkerLocationCount(); uiSrcIdx++)
        {
          UInt uiLocation = pcBitstream->getTileMarkerLocation(uiSrcIdx);
          if ((uiBitsRead>>3)<=uiLocation  &&  uiLocation<((uiBitsRead+uiSubstreamSizeBits)>>3))
          {
            ppcSubstreams[ui]->setTileMarkerLocation( uiDestIdx, uiLocation - (uiBitsRead>>3) );
            ppcSubstreams[ui]->setTileMarkerLocationCount( uiDestIdx+1 );
            uiDestIdx++;
          }
        }
        ppcSubstreams[ui]->setTileMarkerLocationCount( uiDestIdx );
        uiBitsRead += uiSubstreamSizeBits;
      }
    }

    for ( UInt ui = 0 ; ui+1 < uiNumSubstreams; ui++ )
    {
      m_pcEntropyDecoder->setEntropyDecoder ( &m_pcSbacDecoders[uiNumSubstreams - 1 - ui] );
      m_pcEntropyDecoder->setBitstream      (  ppcSubstreams   [uiNumSubstreams - 1 - ui] );
      m_pcEntropyDecoder->resetEntropy      (pcSlice);
    }

    m_pcEntropyDecoder->setEntropyDecoder ( m_pcSbacDecoder  );
    m_pcEntropyDecoder->setBitstream      ( ppcSubstreams[0] );
    m_pcEntropyDecoder->resetEntropy      (pcSlice);

    if(uiSliceStartCuAddr == uiStartCUAddr)
    {
      if(pcSlice->getSPS()->getUseALF())
      {
        if(pcSlice->getAlfEnabledFlag())
        {
          vAlfCUCtrlSlices.push_back(m_cAlfCUCtrlOneSlice);
        }
      }
    }

    m_pcSbacDecoders[0].load(m_pcSbacDecoder);
    m_pcSliceDecoder->decompressSlice( pcBitstream, ppcSubstreams, rpcPic, m_pcSbacDecoder, m_pcSbacDecoders);
    m_pcEntropyDecoder->setBitstream(  ppcSubstreams[uiNumSubstreams-1] );
    
    if ( pcSlice->getPPS()->getEntropyCodingSynchro() )
    {
      // deallocate all created substreams, including internal buffers.
      for (UInt ui = 0; ui < uiNumSubstreams; ui++)
      {
        ppcSubstreams[ui]->deleteFifo();
        delete ppcSubstreams[ui];
      }
      delete[] ppcSubstreams;
      delete[] m_pcSbacDecoders; m_pcSbacDecoders = NULL;
      delete[] m_pcBinCABACs; m_pcBinCABACs = NULL;
    }
    m_dDecTime += (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;
  }
  else
  {
    // deblocking filter
    Bool bLFCrossTileBoundary = (pcSlice->getPPS()->getTileBehaviorControlPresentFlag() == 1)?
                                (pcSlice->getPPS()->getLFCrossTileBoundaryFlag()):(pcSlice->getPPS()->getSPS()->getLFCrossTileBoundaryFlag());
    if(pcSlice->getSPS()->getUseDF())
    {
      if(pcSlice->getInheritDblParamFromAPS())
      {
        pcSlice->setLoopFilterDisable(pcSlice->getAPS()->getLoopFilterDisable());
        if (!pcSlice->getLoopFilterDisable())
        {
          pcSlice->setLoopFilterBetaOffset(pcSlice->getAPS()->getLoopFilterBetaOffset());
          pcSlice->setLoopFilterTcOffset(pcSlice->getAPS()->getLoopFilterTcOffset());
        }
      }
    }
    m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), pcSlice->getLoopFilterBetaOffset(), pcSlice->getLoopFilterTcOffset(), bLFCrossTileBoundary);
    m_pcLoopFilter->loopFilterPic( rpcPic );

    pcSlice = rpcPic->getSlice(0);
    if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
    {
      Int sliceGranularity = pcSlice->getPPS()->getSliceGranularity();
      puiILSliceStartLCU[uiILSliceCount] = rpcPic->getNumCUsInFrame()* rpcPic->getNumPartInCU();
      rpcPic->createNonDBFilterInfo(puiILSliceStartLCU, uiILSliceCount,sliceGranularity,pcSlice->getSPS()->getLFCrossSliceBoundaryFlag(),rpcPic->getPicSym()->getNumTiles() ,bLFCrossTileBoundary);
    }

    if( pcSlice->getSPS()->getUseSAO() )
    {
      if(pcSlice->getSaoEnabledFlag())
      {
        m_pcSAO->createPicSaoInfo(rpcPic, uiILSliceCount);
        m_pcSAO->SAOProcess(rpcPic, pcSlice->getAPS()->getSaoParam());  
        m_pcAdaptiveLoopFilter->PCMLFDisableProcess(rpcPic);
        m_pcSAO->destroyPicSaoInfo();
      }
    }

    // adaptive loop filter
    if( pcSlice->getSPS()->getUseALF() )
    {
      if(pcSlice->getAlfEnabledFlag())
      {
        m_pcAdaptiveLoopFilter->createPicAlfInfo(rpcPic, uiILSliceCount);
      m_pcAdaptiveLoopFilter->ALFProcess(rpcPic, pcSlice->getAPS()->getAlfParam(), vAlfCUCtrlSlices);

      m_pcAdaptiveLoopFilter->PCMLFDisableProcess(rpcPic);
      m_pcAdaptiveLoopFilter->destroyPicAlfInfo();
      }
    }
    
    if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
    {
      rpcPic->destroyNonDBFilterInfo();
    }

    rpcPic->compressMotion(); 
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

    rpcPic->setOutputMark(true);
    rpcPic->setReconMark(true);

    rpcPic->setUsedForTMVP( true );

    uiILSliceCount = 0;
    vAlfCUCtrlSlices.clear();
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
