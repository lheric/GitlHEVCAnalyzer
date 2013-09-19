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

/** \file     TDecTop.cpp
    \brief    decoder class
*/

#include "NALread.h"
#include "TDecTop.h"
#if ENABLE_ANAYSIS_OUTPUT
#include "TLibSysuAnalyzer/TSysuAnalyzerOutput.h"
#endif
//! \ingroup TLibDecoder
//! \{

TDecTop::TDecTop()
: m_SEIs(0)
{
  m_pcPic = 0;
  m_iGopSize      = 0;
  m_bGopSizeSet   = false;
  m_iMaxRefPicNum = 0;
  m_uiValidPS = 0;
#if ENC_DEC_TRACE
  g_hTrace = fopen( "TraceDec.txt", "wb" );
  g_bJustDoIt = g_bEncDecTraceDisable;
  g_nSymbolCounter = 0;
#endif
  m_bRefreshPending = 0;
  m_uiPOCCDR = 0;
  m_uiPOCRA = MAX_UINT;          
  m_uiPrevPOC               = UInt(-1);
  m_bFirstSliceInPicture    = true;
  m_bFirstSliceInSequence   = true;
#if E045_SLICE_COMMON_INFO_SHARING
  m_pcPPSBuffer = NULL;
  m_pbHasNewPPS = NULL;
#endif
}

TDecTop::~TDecTop()
{
#if ENC_DEC_TRACE
  fclose( g_hTrace );
#endif
}

Void TDecTop::create()
{
  m_cGopDecoder.create();
  m_apcSlicePilot = new TComSlice;
  m_uiSliceIdx = m_uiLastSliceIdx = 0;
}

Void TDecTop::destroy()
{
#if E045_SLICE_COMMON_INFO_SHARING
  destroyPPSBuffer();
#endif
  m_cGopDecoder.destroy();
  
  delete m_apcSlicePilot;
  m_apcSlicePilot = NULL;
  
  m_cSliceDecoder.destroy();
}

Void TDecTop::init()
{
  // initialize ROM
  initROM();
#if MTK_SAO
  m_cGopDecoder.  init( &m_cEntropyDecoder, &m_cSbacDecoder, &m_cBinCABAC, &m_cCavlcDecoder, &m_cSliceDecoder, &m_cLoopFilter, &m_cAdaptiveLoopFilter, &m_cSAO);
#else
  m_cGopDecoder.  init( &m_cEntropyDecoder, &m_cSbacDecoder, &m_cBinCABAC, &m_cCavlcDecoder, &m_cSliceDecoder, &m_cLoopFilter, &m_cAdaptiveLoopFilter );
#endif
  m_cSliceDecoder.init( &m_cEntropyDecoder, &m_cCuDecoder );
  m_cEntropyDecoder.init(&m_cPrediction);
}

Void TDecTop::deletePicBuffer ( )
{
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  Int iSize = Int( m_cListPic.size() );
  
  for (Int i = 0; i < iSize; i++ )
  {
    TComPic* pcPic = *(iterPic++);
    pcPic->destroy();
    
    delete pcPic;
    pcPic = NULL;
  }
  
  // destroy ALF temporary buffers
  m_cAdaptiveLoopFilter.destroy();

#if MTK_SAO
  m_cSAO.destroy();
#endif
  
  m_cLoopFilter.        destroy();
  
  // destroy ROM
  destroyROM();
}

Void TDecTop::xUpdateGopSize (TComSlice* pcSlice)
{
  if ( !pcSlice->isIntra() && !m_bGopSizeSet)
  {
    m_iGopSize    = pcSlice->getPOC();
    m_bGopSizeSet = true;
    
    m_cGopDecoder.setGopSize(m_iGopSize);
  }
}

Void TDecTop::xGetNewPicBuffer ( TComSlice* pcSlice, TComPic*& rpcPic )
{
  xUpdateGopSize(pcSlice);
  
  m_iMaxRefPicNum = max(m_iMaxRefPicNum, max(max(2, pcSlice->getNumRefIdx(REF_PIC_LIST_0)+1), m_iGopSize/2 + 2 + pcSlice->getNumRefIdx(REF_PIC_LIST_0)));
  
  if (m_cListPic.size() < (UInt)m_iMaxRefPicNum)
  {
    rpcPic = new TComPic();
    
    rpcPic->create ( pcSlice->getSPS()->getWidth(), pcSlice->getSPS()->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth, true);
    m_cListPic.pushBack( rpcPic );
    
    return;
  }
  
  Bool bBufferIsAvailable = false;
  TComList<TComPic*>::iterator  iterPic   = m_cListPic.begin();
  while (iterPic != m_cListPic.end())
  {
    rpcPic = *(iterPic++);
    if ( rpcPic->getReconMark() == false )
    {
      bBufferIsAvailable = true;
      break;
    }

    if ( rpcPic->getSlice( 0 )->isReferenced() == false )
    {
      rpcPic->setReconMark( false );
      rpcPic->getPicYuvRec()->setBorderExtension( false );
      bBufferIsAvailable = true;
      break;
    }
  }
  
  if ( !bBufferIsAvailable )
  {
    pcSlice->sortPicList(m_cListPic);
    iterPic = m_cListPic.begin();
    rpcPic = *(iterPic);
    rpcPic->setReconMark(false);
    
    // mark it should be extended
    rpcPic->getPicYuvRec()->setBorderExtension(false);
  }
}

Void TDecTop::executeDeblockAndAlf(UInt& ruiPOC, TComList<TComPic*>*& rpcListPic, Int& iSkipFrame, Int& iPOCLastDisplay)
{
  if (!m_pcPic)
    /* nothing to deblock */
    return;

  TComPic*&   pcPic         = m_pcPic;

  // Execute Deblock and ALF only + Cleanup
#if REF_SETTING_FOR_LD
  m_cGopDecoder.decompressGop(NULL, pcPic, true, m_cListPic );
#else
  m_cGopDecoder.decompressGop(NULL, pcPic, true);
#endif

  // Apply decoder picture marking at the end of coding
  pcPic->getSlice( 0 )->decodingTLayerSwitchingMarking( m_cListPic );

  TComSlice::sortPicList( m_cListPic ); // sorting for application output
  ruiPOC              = pcPic->getSlice(m_uiSliceIdx-1)->getPOC();
  rpcListPic          = &m_cListPic;  
  m_cCuDecoder.destroy();        
  m_bFirstSliceInPicture  = true;

  return;
}

Bool TDecTop::decode(InputNALUnit& nalu, Int& iSkipFrame, Int& iPOCLastDisplay)
{
  TComPic*&   pcPic         = m_pcPic;
#if E045_SLICE_COMMON_INFO_SHARING
  static TComPPS*    pcNewPPS = NULL;
#endif

  // Initialize entropy decoder
  m_cEntropyDecoder.setEntropyDecoder (&m_cCavlcDecoder);
  m_cEntropyDecoder.setBitstream      (nalu.m_Bitstream);

  switch (nalu.m_UnitType)
  {
    case NAL_UNIT_SPS:
      m_cEntropyDecoder.decodeSPS( &m_cSPS );

#if ENABLE_ANAYSIS_OUTPUT
      TSysuAnalyzerOutput::getInstance()->writeOutSps( &m_cSPS );
#endif

#if AMP    
      for (Int i = 0; i < m_cSPS.getMaxCUDepth() - 1; i++)
      {
        // m_cSPS.setAMPAcc( i, m_cSPS.getUseAMP() );
        m_cSPS.setAMPAcc( i, 1 );
      }

      for (Int i = m_cSPS.getMaxCUDepth() - 1; i < m_cSPS.getMaxCUDepth(); i++)
      {
        m_cSPS.setAMPAcc( i, 0 );
      }
#endif

      // create ALF temporary buffer
      m_cAdaptiveLoopFilter.create( m_cSPS.getWidth(), m_cSPS.getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
#if MTK_SAO
      m_cSAO.create( m_cSPS.getWidth(), m_cSPS.getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
#endif
#if PARALLEL_MERGED_DEBLK
      m_cLoopFilter.create( m_cSPS.getWidth(), m_cSPS.getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
#else
      m_cLoopFilter.        create( g_uiMaxCUDepth );
#endif
#if E045_SLICE_COMMON_INFO_SHARING
      createPPSBuffer();
#endif
      m_uiValidPS |= 1;
      
      return false;

    case NAL_UNIT_PPS:

#if E045_SLICE_COMMON_INFO_SHARING
      pcNewPPS = getNewPPSBuffer();
#if SUB_LCU_DQP
      pcNewPPS->setSPS(&m_cSPS);
#endif
      m_cEntropyDecoder.decodePPS( pcNewPPS );
      if(pcNewPPS->getSharedPPSInfoEnabled())
      {
        if(m_cSPS.getUseALF())
        {
          m_cEntropyDecoder.decodeAlfParam( pcNewPPS->getSharedAlfParam());
        }
      }
      signalNewPPSAvailable();
#else
#if SUB_LCU_DQP
      m_cPPS.setSPS(&m_cSPS);
#endif
      m_cEntropyDecoder.decodePPS( &m_cPPS );
#endif
      m_uiValidPS |= 2;
      return false;

    case NAL_UNIT_SEI:
      m_SEIs = new SEImessages;
      m_cEntropyDecoder.decodeSEI(*m_SEIs);
      return false;

    case NAL_UNIT_CODED_SLICE:
    case NAL_UNIT_CODED_SLICE_IDR:
    case NAL_UNIT_CODED_SLICE_CDR:
    {
      // make sure we already received both parameter sets
      assert( 3 == m_uiValidPS );
      if (m_bFirstSliceInPicture)
      {
        m_apcSlicePilot->initSlice();
        m_uiSliceIdx     = 0;
        m_uiLastSliceIdx = 0;
#if E045_SLICE_COMMON_INFO_SHARING
        if(hasNewPPS())
        {
          m_pcPPS = pcNewPPS;
          updatePPSBuffer();
        }
#endif
      }
      m_apcSlicePilot->setSliceIdx(m_uiSliceIdx);

      //  Read slice header
      m_apcSlicePilot->setSPS( &m_cSPS );
#if E045_SLICE_COMMON_INFO_SHARING
      m_apcSlicePilot->setPPS( m_pcPPS );
#else
      m_apcSlicePilot->setPPS( &m_cPPS );
#endif
      m_apcSlicePilot->setSliceIdx(m_uiSliceIdx);
      if (!m_bFirstSliceInPicture)
      {
        memcpy(m_apcSlicePilot, pcPic->getPicSym()->getSlice(m_uiSliceIdx-1), sizeof(TComSlice));
      }

      m_apcSlicePilot->setNalUnitType(nalu.m_UnitType);
      m_apcSlicePilot->setReferenced(nalu.m_RefIDC != NAL_REF_IDC_PRIORITY_LOWEST);
      m_cEntropyDecoder.decodeSliceHeader (m_apcSlicePilot);

      if ( m_apcSlicePilot->getSymbolMode() )
      {
        Int numBitsForByteAlignment = nalu.m_Bitstream->getNumBitsUntilByteAligned();
        if ( numBitsForByteAlignment > 0 )
        {
          UInt bitsForByteAlignment;
          nalu.m_Bitstream->read( numBitsForByteAlignment, bitsForByteAlignment );
          assert( bitsForByteAlignment == ( ( 1 << numBitsForByteAlignment ) - 1 ) );
        }
      }
      m_apcSlicePilot->setTLayerInfo(nalu.m_TemporalID);

      if (m_apcSlicePilot->isNextSlice() && m_apcSlicePilot->getPOC()!=m_uiPrevPOC && !m_bFirstSliceInSequence)
      {
        m_uiPrevPOC = m_apcSlicePilot->getPOC();
        return true;
      }
      if (m_apcSlicePilot->isNextSlice()) 
        m_uiPrevPOC = m_apcSlicePilot->getPOC();
      m_bFirstSliceInSequence = false;
      if (m_apcSlicePilot->isNextSlice())
      {
        // Skip pictures due to random access
        if (isRandomAccessSkipPicture(iSkipFrame, iPOCLastDisplay))
        {
          return false;
        }
      }
      
      if (m_bFirstSliceInPicture)
      {
        // Buffer initialize for prediction.
        m_cPrediction.initTempBuff();
        //  Get a new picture buffer
        xGetNewPicBuffer (m_apcSlicePilot, pcPic);
        
        /* transfer any SEI messages that have been received to the picture */
        pcPic->setSEIs(m_SEIs);
        m_SEIs = NULL;

        // Recursive structure
        m_cCuDecoder.create ( g_uiMaxCUDepth, g_uiMaxCUWidth, g_uiMaxCUHeight );
        m_cCuDecoder.init   ( &m_cEntropyDecoder, &m_cTrQuant, &m_cPrediction );
        m_cTrQuant.init     ( g_uiMaxCUWidth, g_uiMaxCUHeight, m_apcSlicePilot->getSPS()->getMaxTrSize());
        
        m_cSliceDecoder.create( m_apcSlicePilot, m_apcSlicePilot->getSPS()->getWidth(), m_apcSlicePilot->getSPS()->getHeight(), g_uiMaxCUWidth, g_uiMaxCUHeight, g_uiMaxCUDepth );
      }

      //  Set picture slice pointer
      TComSlice*  pcSlice = m_apcSlicePilot;
      Bool bNextSlice     = pcSlice->isNextSlice();
      if (m_bFirstSliceInPicture) 
      {
        if(pcPic->getNumAllocatedSlice() != 1)
        {
          pcPic->clearSliceBuffer();
        }
      }
      else
      {
        pcPic->allocateNewSlice();
      }
      assert(pcPic->getNumAllocatedSlice() == (m_uiSliceIdx + 1));
      m_apcSlicePilot = pcPic->getPicSym()->getSlice(m_uiSliceIdx); 
      pcPic->getPicSym()->setSlice(pcSlice, m_uiSliceIdx);

      pcPic->setTLayer(nalu.m_TemporalID);

      if (bNextSlice)
      {
        // Do decoding refresh marking if any
        pcSlice->decodingRefreshMarking(m_uiPOCCDR, m_bRefreshPending, m_cListPic);
        
        // Set reference list
        pcSlice->setRefPicList( m_cListPic );
        
        // HierP + GPB case
        if ( m_cSPS.getUseLDC() && pcSlice->isInterB() )
        {
          if(pcSlice->getRefPicListCombinationFlag() && (pcSlice->getNumRefIdx(REF_PIC_LIST_0) > pcSlice->getNumRefIdx(REF_PIC_LIST_1)))
          {
            for (Int iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_1); iRefIdx++)
            {
              pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
            }
          }
          else
          {
            Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
            pcSlice->setNumRefIdx( REF_PIC_LIST_1, iNumRefIdx );
            
            for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
            {
              pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
            }
          }
        }
        
        // For generalized B
        // note: maybe not existed case (always L0 is copied to L1 if L1 is empty)
        if (pcSlice->isInterB() && pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0)
        {
          Int iNumRefIdx = pcSlice->getNumRefIdx(REF_PIC_LIST_0);
          pcSlice->setNumRefIdx        ( REF_PIC_LIST_1, iNumRefIdx );
          
          for (Int iRefIdx = 0; iRefIdx < iNumRefIdx; iRefIdx++)
          {
            pcSlice->setRefPic(pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx), REF_PIC_LIST_1, iRefIdx);
          }
        }
#if TMVP_ONE_LIST_CHECK
        if (pcSlice->isInterB())
        {
          Bool bLowDelay = true;
          Int  iCurrPOC  = pcSlice->getPOC();
          Int iRefIdx = 0;

          for (iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_0) && bLowDelay; iRefIdx++)
          {
            if ( pcSlice->getRefPic(REF_PIC_LIST_0, iRefIdx)->getPOC() > iCurrPOC )
            {
              bLowDelay = false;
            }
          }
          for (iRefIdx = 0; iRefIdx < pcSlice->getNumRefIdx(REF_PIC_LIST_1) && bLowDelay; iRefIdx++)
          {
            if ( pcSlice->getRefPic(REF_PIC_LIST_1, iRefIdx)->getPOC() > iCurrPOC )
            {
              bLowDelay = false;
            }
          }

          pcSlice->setCheckLDC(bLowDelay);            
        }
#endif
        
        //---------------
        pcSlice->setRefPOCList();
        
        if(!pcSlice->getRefPicListModificationFlagLC())
        {
          pcSlice->generateCombinedList();
        }
        
        pcSlice->setNoBackPredFlag( false );
        if ( pcSlice->getSliceType() == B_SLICE && !pcSlice->getRefPicListCombinationFlag())
        {
          if ( pcSlice->getNumRefIdx(RefPicList( 0 ) ) == pcSlice->getNumRefIdx(RefPicList( 1 ) ) )
          {
            pcSlice->setNoBackPredFlag( true );
            int i;
            for ( i=0; i < pcSlice->getNumRefIdx(RefPicList( 1 ) ); i++ )
            {
              if ( pcSlice->getRefPOC(RefPicList(1), i) != pcSlice->getRefPOC(RefPicList(0), i) ) 
              {
                pcSlice->setNoBackPredFlag( false );
                break;
              }
            }
          }
        }
      }
      
      pcPic->setCurrSliceIdx(m_uiSliceIdx);

      //  Decode a picture
#if REF_SETTING_FOR_LD
      m_cGopDecoder.decompressGop(nalu.m_Bitstream, pcPic, false, m_cListPic );
#else
      m_cGopDecoder.decompressGop(nalu.m_Bitstream, pcPic, false);
#endif

      m_bFirstSliceInPicture = false;
      m_uiSliceIdx++;
    }
      break;
    default:
      assert (1);
  }

  return false;
}

/** Function for checking if picture should be skipped because of random access
 * \param iSkipFrame skip frame counter
 * \param iPOCLastDisplay POC of last picture displayed
 * \returns true if the picture shold be skipped in the random access.
 * This function checks the skipping of pictures in the case of -s option random access.
 * All pictures prior to the random access point indicated by the counter iSkipFrame are skipped.
 * It also checks the type of Nal unit type at the random access point.
 * If the random access point is CDR, pictures with POC equal to or greater than the CDR POC are decoded.
 * If the random access point is IDR all pictures after the random access point are decoded.
 * If the random access point is not IDR or CDR, a warning is issues, and decoding of pictures with POC 
 * equal to or greater than the random access point POC is attempted. For non IDR/CDR random 
 * access point there is no guarantee that the decoder will not crash.
 */
Bool TDecTop::isRandomAccessSkipPicture(Int& iSkipFrame,  Int& iPOCLastDisplay)
{
  if (iSkipFrame) 
  {
    iSkipFrame--;   // decrement the counter
    return true;
  }
  else if (m_uiPOCRA == MAX_UINT) // start of random access point, m_uiPOCRA has not been set yet.
  {
    if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_CDR)
    {
      m_uiPOCRA = m_apcSlicePilot->getPOC(); // set the POC random access since we need to skip the reordered pictures in CDR.
    }
    else if (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR)
    {
      m_uiPOCRA = 0; // no need to skip the reordered pictures in IDR, they are decodable.
    }
    else 
    {
      printf("\nUnsafe random access point. Decoder may crash.");
      m_uiPOCRA = m_apcSlicePilot->getPOC(); // set the POC random access skip the reordered pictures and try to decode if possible.  This increases the chances of avoiding a decoder crash.
      //m_uiPOCRA = 0;
    }
  }
  else if (m_apcSlicePilot->getPOC() < m_uiPOCRA)  // skip the reordered pictures if necessary
  {
    iPOCLastDisplay++;
    return true;
  }
  // if we reach here, then the picture is not skipped.
  return false; 
}
#if E045_SLICE_COMMON_INFO_SHARING
Void TDecTop::createPPSBuffer()
{
  m_pcPPSBuffer = new TComPPS[MAX_NUM_PPS_BUFFER];
  m_pbHasNewPPS = new Bool[MAX_NUM_PPS_BUFFER];

  m_iPPSCounter = 0;

  for(Int i=0; i< MAX_NUM_PPS_BUFFER; i++)
  {
    m_pbHasNewPPS[i] = false;
  }

  if(m_cSPS.getUseALF())
  {
    for(Int i=0; i< MAX_NUM_PPS_BUFFER; i++)
    {
      m_cAdaptiveLoopFilter.allocALFParam(m_pcPPSBuffer[i].getSharedAlfParam());
    }
  }

  m_pcPPS = &(m_pcPPSBuffer[0]);
}

Void TDecTop::destroyPPSBuffer()
{
  if(m_pbHasNewPPS != NULL)
  {
    delete[] m_pbHasNewPPS;
    m_pbHasNewPPS = NULL;
  }

  if(m_pcPPSBuffer != NULL)
  {
    if(m_cSPS.getUseALF())
    {
      for (Int i=0; i< MAX_NUM_PPS_BUFFER; i++)
      {
        m_cAdaptiveLoopFilter.freeALFParam(m_pcPPSBuffer[i].getSharedAlfParam());
      }
    }
    delete[] m_pcPPSBuffer;
    m_pcPPSBuffer = NULL;
  }
}

TComPPS* TDecTop::getNewPPSBuffer()
{
  return &(m_pcPPSBuffer[m_iPPSCounter]);
}

Void TDecTop::signalNewPPSAvailable()
{
  m_pbHasNewPPS[m_iPPSCounter] = true;
}

Void TDecTop::updatePPSBuffer()
{
  m_pbHasNewPPS[m_iPPSCounter] = false;
  m_iPPSCounter = (m_iPPSCounter +1)%MAX_NUM_PPS_BUFFER;
}


#endif


//! \}
