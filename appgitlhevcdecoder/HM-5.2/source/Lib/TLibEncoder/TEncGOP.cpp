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

/** \file     TEncGOP.cpp
    \brief    GOP encoder class
*/

#include <list>
#include <algorithm>

#include "TEncTop.h"
#include "TEncGOP.h"
#include "TEncAnalyze.h"
#include "libmd5/MD5.h"
#include "TLibCommon/SEI.h"
#include "TLibCommon/NAL.h"
#include "NALwrite.h"

#include <time.h>
#include <math.h>
#include <functional>

using namespace std;

//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TEncGOP::TEncGOP()
{
  m_iLastIDR            = 0;
  m_iGopSize            = 0;
  m_iNumPicCoded        = 0; //Niko
  m_bFirst              = true;
  
  m_pcCfg               = NULL;
  m_pcSliceEncoder      = NULL;
  m_pcListPic           = NULL;
  
  m_pcEntropyCoder      = NULL;
  m_pcCavlcCoder        = NULL;
  m_pcSbacCoder         = NULL;
  m_pcBinCABAC          = NULL;
  
  m_bSeqFirst           = true;
  
  m_bRefreshPending     = 0;
  m_uiPOCCDR            = 0;

  return;
}

TEncGOP::~TEncGOP()
{
}

/** Create list to contain pointers to LCU start addresses of slice.
 * \param iWidth, iHeight are picture width, height. iMaxCUWidth, iMaxCUHeight are LCU width, height.
 */
Void  TEncGOP::create( Int iWidth, Int iHeight, UInt iMaxCUWidth, UInt iMaxCUHeight )
{
  UInt uiWidthInCU       = ( iWidth %iMaxCUWidth  ) ? iWidth /iMaxCUWidth  + 1 : iWidth /iMaxCUWidth;
  UInt uiHeightInCU      = ( iHeight%iMaxCUHeight ) ? iHeight/iMaxCUHeight + 1 : iHeight/iMaxCUHeight;
  UInt uiNumCUsInFrame   = uiWidthInCU * uiHeightInCU;
  m_uiStoredStartCUAddrForEncodingSlice = new UInt [uiNumCUsInFrame*(1<<(g_uiMaxCUDepth<<1))+1];
  m_uiStoredStartCUAddrForEncodingEntropySlice = new UInt [uiNumCUsInFrame*(1<<(g_uiMaxCUDepth<<1))+1];
  m_bLongtermTestPictureHasBeenCoded = 0;
  m_bLongtermTestPictureHasBeenCoded2 = 0;
}

Void  TEncGOP::destroy()
{
  delete [] m_uiStoredStartCUAddrForEncodingSlice; m_uiStoredStartCUAddrForEncodingSlice = NULL;
  delete [] m_uiStoredStartCUAddrForEncodingEntropySlice; m_uiStoredStartCUAddrForEncodingEntropySlice = NULL;
}

Void TEncGOP::init ( TEncTop* pcTEncTop )
{
  m_pcEncTop     = pcTEncTop;
  m_pcCfg                = pcTEncTop;
  m_pcSliceEncoder       = pcTEncTop->getSliceEncoder();
  m_pcListPic            = pcTEncTop->getListPic();
  
  m_pcEntropyCoder       = pcTEncTop->getEntropyCoder();
  m_pcCavlcCoder         = pcTEncTop->getCavlcCoder();
  m_pcSbacCoder          = pcTEncTop->getSbacCoder();
  m_pcBinCABAC           = pcTEncTop->getBinCABAC();
  m_pcLoopFilter         = pcTEncTop->getLoopFilter();
  m_pcBitCounter         = pcTEncTop->getBitCounter();
  
  // Adaptive Loop filter
  m_pcAdaptiveLoopFilter = pcTEncTop->getAdaptiveLoopFilter();
  //--Adaptive Loop filter
  m_pcSAO                = pcTEncTop->getSAO();
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================
Void TEncGOP::compressGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut, std::list<AccessUnit>& accessUnitsInGOP)
{
  TComPic*        pcPic;
  TComPicYuv*     pcPicYuvRecOut;
  TComSlice*      pcSlice;
  TComOutputBitstream  *pcBitstreamRedirect;
  pcBitstreamRedirect = new TComOutputBitstream;
  OutputNALUnit        *naluBuffered             = NULL;
  Bool                  bIteratorAtListStart     = false;
  AccessUnit::iterator  itLocationToPushSliceHeaderNALU; // used to store location where NALU containing slice header is to be inserted
  UInt                  uiOneBitstreamPerSliceLength = 0;
  TEncSbac* pcSbacCoders = NULL;
  TComOutputBitstream* pcSubstreamsOut = NULL;

  xInitGOP( iPOCLast, iNumPicRcvd, rcListPic, rcListPicYuvRecOut );
  
  m_iNumPicCoded = 0;

  for ( Int iGOPid=0; iGOPid < m_iGopSize; iGOPid++ )
    {
      UInt uiColDir = 1;
      //-- For time output for each slice
      long iBeforeTime = clock();
      
      //select uiColDir
      Int iCloseLeft=1, iCloseRight=-1;
      for(Int i = 0; i<m_pcCfg->getGOPEntry(iGOPid).m_iNumRefPics; i++) 
      {
        Int iRef = m_pcCfg->getGOPEntry(iGOPid).m_aiReferencePics[i];
        if(iRef>0&&(iRef<iCloseRight||iCloseRight==-1))
        {
          iCloseRight=iRef;
        }
        else if(iRef<0&&(iRef>iCloseLeft||iCloseLeft==1))
        {
          iCloseLeft=iRef;
        }
      }
      if(iCloseRight>-1)
      {
        iCloseRight=iCloseRight+m_pcCfg->getGOPEntry(iGOPid).m_iPOC-1;
      }
      if(iCloseLeft<1) 
      {
        iCloseLeft=iCloseLeft+m_pcCfg->getGOPEntry(iGOPid).m_iPOC-1;
        while(iCloseLeft<0)
        {
          iCloseLeft+=m_iGopSize;
        }
      }
      int iLeftQP=0, iRightQP=0;
      for(Int i=0; i<m_iGopSize; i++)
      {
        if(m_pcCfg->getGOPEntry(i).m_iPOC==(iCloseLeft%m_iGopSize)+1)
          iLeftQP= m_pcCfg->getGOPEntry(i).m_iQPOffset;
        if (m_pcCfg->getGOPEntry(i).m_iPOC==(iCloseRight%m_iGopSize)+1)
          iRightQP=m_pcCfg->getGOPEntry(i).m_iQPOffset;
      }
      if(iCloseRight>-1&&iRightQP<iLeftQP)
      {
        uiColDir=0;
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////// Initial to start encoding
      UInt uiPOCCurr = iPOCLast -iNumPicRcvd+ m_pcCfg->getGOPEntry(iGOPid).m_iPOC;
      Int iTimeOffset = m_pcCfg->getGOPEntry(iGOPid).m_iPOC;
      if(uiPOCCurr>=m_pcCfg->getFrameToBeEncoded())
      {
        continue;
      }
      if(iPOCLast == 0)
      {
        uiPOCCurr=0;
        iTimeOffset = 1;
      }
        
      if(getNalUnitType(uiPOCCurr) == NAL_UNIT_CODED_SLICE_IDR)
      {
        m_iLastIDR = uiPOCCurr;
      }        
      /* start a new access unit: create an entry in the list of output
       * access units */
      accessUnitsInGOP.push_back(AccessUnit());
      AccessUnit& accessUnit = accessUnitsInGOP.back();
      xGetBuffer( rcListPic, rcListPicYuvRecOut, iNumPicRcvd, iTimeOffset, pcPic, pcPicYuvRecOut, uiPOCCurr );
      
      //  Slice data initialization
      pcPic->clearSliceBuffer();
      assert(pcPic->getNumAllocatedSlice() == 1);
      m_pcSliceEncoder->setSliceIdx(0);
      pcPic->setCurrSliceIdx(0);

      std::vector<TComAPS>& vAPS = m_pcEncTop->getAPS();
      m_pcSliceEncoder->initEncSlice ( pcPic, iPOCLast, uiPOCCurr, iNumPicRcvd, iGOPid, pcSlice, m_pcEncTop->getSPS(), m_pcEncTop->getPPS() );
      pcSlice->setLastIDR(m_iLastIDR);
      pcSlice->setSliceIdx(0);

      m_pcEncTop->getSPS()->setDisInter4x4(m_pcEncTop->getDisInter4x4());
      pcSlice->setScalingList ( m_pcEncTop->getScalingList()  );
      if(m_pcEncTop->getUseScalingListId() == SCALING_LIST_OFF)
      {
        m_pcEncTop->getTrQuant()->setFlatScalingList();
        m_pcEncTop->getTrQuant()->setUseScalingList(false);
      }
      else if(m_pcEncTop->getUseScalingListId() == SCALING_LIST_DEFAULT)
      {
        pcSlice->setDefaultScalingList ();
        pcSlice->getScalingList()->setUseDefaultOnlyFlag(true);
        m_pcEncTop->getTrQuant()->setScalingList(pcSlice->getScalingList());
        m_pcEncTop->getTrQuant()->setUseScalingList(true);
      }
      else if(m_pcEncTop->getUseScalingListId() == SCALING_LIST_FILE_READ)
      {
        if(pcSlice->getScalingList()->xParseScalingList(m_pcCfg->getScalingListFile()))
        {
          pcSlice->setDefaultScalingList ();
        }
        pcSlice->getScalingList()->xScalingListMatrixModeDecision();
        pcSlice->getScalingList()->setUseDefaultOnlyFlag(pcSlice->checkDefaultScalingList());
        m_pcEncTop->getTrQuant()->setScalingList(pcSlice->getScalingList());
        m_pcEncTop->getTrQuant()->setUseScalingList(true);
      }
      else
      {
        printf("error : ScalingList == %d no support\n",m_pcEncTop->getUseScalingListId());
        assert(0);
      }

      if(pcSlice->getSliceType()==B_SLICE&&m_pcCfg->getGOPEntry(iGOPid).m_iSliceType=='P')
      {
        pcSlice->setSliceType(P_SLICE);
      }
      // Set the nal unit type
      pcSlice->setNalUnitType(getNalUnitType(uiPOCCurr));
      // Do decoding refresh marking if any 
      pcSlice->decodingRefreshMarking(m_uiPOCCDR, m_bRefreshPending, rcListPic);

      if ( !pcSlice->getPPS()->getEnableTMVPFlag() && pcPic->getTLayer() == 0 )
      {
        pcSlice->decodingMarkingForNoTMVP( rcListPic, pcSlice->getPOC() );
      }

      m_pcEncTop->selectReferencePictureSet(pcSlice, uiPOCCurr, iGOPid,rcListPic);
      pcSlice->getRPS()->setNumberOfLongtermPictures(0);

      if(pcSlice->checkThatAllRefPicsAreAvailable(rcListPic, pcSlice->getRPS(), false) != 0)
      {
         pcSlice->createExplicitReferencePictureSetFromReference(rcListPic, pcSlice->getRPS());
      }
      pcSlice->applyReferencePictureSet(rcListPic, pcSlice->getRPS());

      TComRefPicListModification* refPicListModification = pcSlice->getRefPicListModification();
      refPicListModification->setRefPicListModificationFlagL0(0);
      refPicListModification->setNumberOfRefPicListModificationsL0(0);
      refPicListModification->setRefPicListModificationFlagL1(0);
      refPicListModification->setNumberOfRefPicListModificationsL1(0);

      pcSlice->setNumRefIdx(REF_PIC_LIST_0,min((UInt)m_pcCfg->getGOPEntry(iGOPid).m_iRefBufSize,pcSlice->getRPS()->getNumberOfPictures()));
      pcSlice->setNumRefIdx(REF_PIC_LIST_1,min((UInt)m_pcCfg->getGOPEntry(iGOPid).m_iRefBufSize,pcSlice->getRPS()->getNumberOfPictures()));

#if ADAPTIVE_QP_SELECTION
      pcSlice->setTrQuant( m_pcEncTop->getTrQuant() );
#endif      
      //  Set reference list
      pcSlice->setRefPicList ( rcListPic );
      
      //  Slice info. refinement
      if ( (pcSlice->getSliceType() == B_SLICE) && (pcSlice->getNumRefIdx(REF_PIC_LIST_1) == 0) )
      {
        pcSlice->setSliceType ( P_SLICE );
      }
      
      if (pcSlice->getSliceType() != B_SLICE || !pcSlice->getSPS()->getUseLComb())
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, 0);
        pcSlice->setRefPicListCombinationFlag(false);
        pcSlice->setRefPicListModificationFlagLC(false);
      }
      else
      {
        pcSlice->setRefPicListCombinationFlag(pcSlice->getSPS()->getUseLComb());
        pcSlice->setRefPicListModificationFlagLC(pcSlice->getSPS()->getLCMod());
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, pcSlice->getNumRefIdx(REF_PIC_LIST_0));
      }
      
      if (pcSlice->getSliceType() == B_SLICE)
      {
        pcSlice->setColDir(uiColDir);
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
      
      uiColDir = 1-uiColDir;
      
      //-------------------------------------------------------------
      pcSlice->setRefPOCList();
      
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

      if(pcSlice->getNoBackPredFlag())
      {
        pcSlice->setNumRefIdx(REF_PIC_LIST_C, 0);
      }
      pcSlice->generateCombinedList();
      
      /////////////////////////////////////////////////////////////////////////////////////////////////// Compress a slice
      //  Slice compression
      if (m_pcCfg->getUseASR())
      {
        m_pcSliceEncoder->setSearchRange(pcSlice);
      }

      UInt uiNumSlices = 1;

      UInt uiInternalAddress = pcPic->getNumPartInCU()-4;
      UInt uiExternalAddress = pcPic->getPicSym()->getNumberOfCUsInFrame()-1;
      UInt uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
      UInt uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
      UInt uiWidth = pcSlice->getSPS()->getWidth();
      UInt uiHeight = pcSlice->getSPS()->getHeight();
      while(uiPosX>=uiWidth||uiPosY>=uiHeight) 
      {
        uiInternalAddress--;
        uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
        uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
      }
      uiInternalAddress++;
      if(uiInternalAddress==pcPic->getNumPartInCU()) 
      {
        uiInternalAddress = 0;
        uiExternalAddress++;
      }
      UInt uiRealEndAddress = uiExternalAddress*pcPic->getNumPartInCU()+uiInternalAddress;

    UInt uiCummulativeTileWidth;
    UInt uiCummulativeTileHeight;
    Int  p, j;
    UInt uiEncCUAddr;
    
    if(pcSlice->getPPS()->getTileBehaviorControlPresentFlag() == 1)
    {
      pcPic->getPicSym()->setTileBoundaryIndependenceIdr( pcSlice->getPPS()->getTileBoundaryIndependenceIdr() );
    }
    else
    {
      pcPic->getPicSym()->setTileBoundaryIndependenceIdr( pcSlice->getPPS()->getSPS()->getTileBoundaryIndependenceIdr() );

    }

    if( pcSlice->getPPS()->getColumnRowInfoPresent() == 1 )    //derive the tile parameters from PPS
    {
      //set NumColumnsMinus1 and NumRowsMinus1
      pcPic->getPicSym()->setNumColumnsMinus1( pcSlice->getPPS()->getNumColumnsMinus1() );
      pcPic->getPicSym()->setNumRowsMinus1( pcSlice->getPPS()->getNumRowsMinus1() );

      //create the TComTileArray
      pcPic->getPicSym()->xCreateTComTileArray();

      if( pcSlice->getPPS()->getUniformSpacingIdr() == 1 )
      {
        //set the width for each tile
        for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
        {
          for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1()+1; p++)
          {
            pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->
              setTileWidth( (p+1)*pcPic->getPicSym()->getFrameWidthInCU()/(pcPic->getPicSym()->getNumColumnsMinus1()+1) 
              - (p*pcPic->getPicSym()->getFrameWidthInCU())/(pcPic->getPicSym()->getNumColumnsMinus1()+1) );
          }
        }

        //set the height for each tile
        for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
        {
          for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1()+1; p++)
          {
            pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->
              setTileHeight( (p+1)*pcPic->getPicSym()->getFrameHeightInCU()/(pcPic->getPicSym()->getNumRowsMinus1()+1) 
              - (p*pcPic->getPicSym()->getFrameHeightInCU())/(pcPic->getPicSym()->getNumRowsMinus1()+1) );   
          }
        }
      }
      else
      {
        //set the width for each tile
        for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
        {
          uiCummulativeTileWidth = 0;
          for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1(); p++)
          {
            pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->setTileWidth( pcSlice->getPPS()->getColumnWidth(p) );
            uiCummulativeTileWidth += pcSlice->getPPS()->getColumnWidth(p);
          }
          pcPic->getPicSym()->getTComTile(j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p)->setTileWidth( pcPic->getPicSym()->getFrameWidthInCU()-uiCummulativeTileWidth );
        }

        //set the height for each tile
        for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
        {
          uiCummulativeTileHeight = 0;
          for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1(); p++)
          {
            pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->setTileHeight( pcSlice->getPPS()->getRowHeight(p) );
            uiCummulativeTileHeight += pcSlice->getPPS()->getRowHeight(p);
          }
          pcPic->getPicSym()->getTComTile(p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcPic->getPicSym()->getFrameHeightInCU()-uiCummulativeTileHeight );
        }
      }
    }
    else //derive the tile parameters from SPS
    {
      //set NumColumnsMins1 and NumRowsMinus1
      pcPic->getPicSym()->setNumColumnsMinus1( pcSlice->getSPS()->getNumColumnsMinus1() );
      pcPic->getPicSym()->setNumRowsMinus1( pcSlice->getSPS()->getNumRowsMinus1() );

      //create the TComTileArray
      pcPic->getPicSym()->xCreateTComTileArray();

      if( pcSlice->getSPS()->getUniformSpacingIdr() == 1 )
      {
        //set the width for each tile
        for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
        {
          for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1()+1; p++)
          {
            pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->
              setTileWidth( (p+1)*pcPic->getPicSym()->getFrameWidthInCU()/(pcPic->getPicSym()->getNumColumnsMinus1()+1) 
              - (p*pcPic->getPicSym()->getFrameWidthInCU())/(pcPic->getPicSym()->getNumColumnsMinus1()+1) );
          }
        }

        //set the height for each tile
        for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
        {
          for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1()+1; p++)
          {
            pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->
              setTileHeight( (p+1)*pcPic->getPicSym()->getFrameHeightInCU()/(pcPic->getPicSym()->getNumRowsMinus1()+1) 
              - (p*pcPic->getPicSym()->getFrameHeightInCU())/(pcPic->getPicSym()->getNumRowsMinus1()+1) );   
          }
        }
      }

      else
      {
        //set the width for each tile
        for(j=0; j < pcPic->getPicSym()->getNumRowsMinus1()+1; j++)
        {
          uiCummulativeTileWidth = 0;
          for(p=0; p < pcPic->getPicSym()->getNumColumnsMinus1(); p++)
          {
            pcPic->getPicSym()->getTComTile( j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p )->setTileWidth( pcSlice->getSPS()->getColumnWidth(p) );
            uiCummulativeTileWidth += pcSlice->getSPS()->getColumnWidth(p);
          }
          pcPic->getPicSym()->getTComTile(j * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + p)->setTileWidth( pcPic->getPicSym()->getFrameWidthInCU()-uiCummulativeTileWidth );
        }

        //set the height for each tile
        for(j=0; j < pcPic->getPicSym()->getNumColumnsMinus1()+1; j++)
        {
          uiCummulativeTileHeight = 0;
          for(p=0; p < pcPic->getPicSym()->getNumRowsMinus1(); p++)
          {
            pcPic->getPicSym()->getTComTile( p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j )->setTileHeight( pcSlice->getSPS()->getRowHeight(p) );
            uiCummulativeTileHeight += pcSlice->getSPS()->getRowHeight(p);
          }
          pcPic->getPicSym()->getTComTile(p * (pcPic->getPicSym()->getNumColumnsMinus1()+1) + j)->setTileHeight( pcPic->getPicSym()->getFrameHeightInCU()-uiCummulativeTileHeight );
        }
      }
    }

    //intialize each tile of the current picture
    pcPic->getPicSym()->xInitTiles();

    // Allocate some coders, now we know how many tiles there are.
    Int iNumSubstreams = pcSlice->getPPS()->getNumSubstreams();
    
    //generate the Coding Order Map and Inverse Coding Order Map
    for(p=0, uiEncCUAddr=0; p<pcPic->getPicSym()->getNumberOfCUsInFrame(); p++, uiEncCUAddr = pcPic->getPicSym()->xCalculateNxtCUAddr(uiEncCUAddr))
    {
      pcPic->getPicSym()->setCUOrderMap(p, uiEncCUAddr);
      pcPic->getPicSym()->setInverseCUOrderMap(uiEncCUAddr, p);
    }
    pcPic->getPicSym()->setCUOrderMap(pcPic->getPicSym()->getNumberOfCUsInFrame(), pcPic->getPicSym()->getNumberOfCUsInFrame());    
    pcPic->getPicSym()->setInverseCUOrderMap(pcPic->getPicSym()->getNumberOfCUsInFrame(), pcPic->getPicSym()->getNumberOfCUsInFrame());
    if (pcSlice->getPPS()->getEntropyCodingMode())
    {
      // Allocate some coders, now we know how many tiles there are.
      m_pcEncTop->createWPPCoders(iNumSubstreams);
      pcSbacCoders = m_pcEncTop->getSbacCoders();
      pcSubstreamsOut = new TComOutputBitstream[iNumSubstreams];
    }

      UInt uiStartCUAddrSliceIdx = 0; // used to index "m_uiStoredStartCUAddrForEncodingSlice" containing locations of slice boundaries
      UInt uiStartCUAddrSlice    = 0; // used to keep track of current slice's starting CU addr.
      pcSlice->setSliceCurStartCUAddr( uiStartCUAddrSlice ); // Setting "start CU addr" for current slice
      memset(m_uiStoredStartCUAddrForEncodingSlice, 0, sizeof(UInt) * (pcPic->getPicSym()->getNumberOfCUsInFrame()*pcPic->getNumPartInCU()+1));

      UInt uiStartCUAddrEntropySliceIdx = 0; // used to index "m_uiStoredStartCUAddrForEntropyEncodingSlice" containing locations of slice boundaries
      UInt uiStartCUAddrEntropySlice    = 0; // used to keep track of current Entropy slice's starting CU addr.
      pcSlice->setEntropySliceCurStartCUAddr( uiStartCUAddrEntropySlice ); // Setting "start CU addr" for current Entropy slice
      
      memset(m_uiStoredStartCUAddrForEncodingEntropySlice, 0, sizeof(UInt) * (pcPic->getPicSym()->getNumberOfCUsInFrame()*pcPic->getNumPartInCU()+1));
      UInt uiNextCUAddr = 0;
      m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]                = uiNextCUAddr;
      m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = uiNextCUAddr;

      while(uiNextCUAddr<uiRealEndAddress) // determine slice boundaries
      {
        pcSlice->setNextSlice       ( false );
        pcSlice->setNextEntropySlice( false );
        assert(pcPic->getNumAllocatedSlice() == uiStartCUAddrSliceIdx);
        m_pcSliceEncoder->precompressSlice( pcPic );
        m_pcSliceEncoder->compressSlice   ( pcPic );

        Bool bNoBinBitConstraintViolated = (!pcSlice->isNextSlice() && !pcSlice->isNextEntropySlice());
        if (pcSlice->isNextSlice() || (bNoBinBitConstraintViolated && m_pcCfg->getSliceMode()==AD_HOC_SLICES_FIXED_NUMBER_OF_LCU_IN_SLICE))
        {
          uiStartCUAddrSlice                                              = pcSlice->getSliceCurEndCUAddr();
          // Reconstruction slice
          m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]  = uiStartCUAddrSlice;
          // Entropy slice
          if (uiStartCUAddrEntropySliceIdx>0 && m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx-1] != uiStartCUAddrSlice)
          {
            m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = uiStartCUAddrSlice;
          }
          
          if (uiStartCUAddrSlice < uiRealEndAddress)
          {
            pcPic->allocateNewSlice();          
            pcPic->setCurrSliceIdx                  ( uiStartCUAddrSliceIdx-1 );
            m_pcSliceEncoder->setSliceIdx           ( uiStartCUAddrSliceIdx-1 );
            pcSlice = pcPic->getSlice               ( uiStartCUAddrSliceIdx-1 );
            pcSlice->copySliceInfo                  ( pcPic->getSlice(0)      );
            pcSlice->setSliceIdx                    ( uiStartCUAddrSliceIdx-1 );
            pcSlice->setSliceCurStartCUAddr         ( uiStartCUAddrSlice      );
            pcSlice->setEntropySliceCurStartCUAddr  ( uiStartCUAddrSlice      );
            pcSlice->setSliceBits(0);
            uiNumSlices ++;
          }
        }
        else if (pcSlice->isNextEntropySlice() || (bNoBinBitConstraintViolated && m_pcCfg->getEntropySliceMode()==SHARP_FIXED_NUMBER_OF_LCU_IN_ENTROPY_SLICE))
        {
          uiStartCUAddrEntropySlice                                                     = pcSlice->getEntropySliceCurEndCUAddr();
          m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = uiStartCUAddrEntropySlice;
          pcSlice->setEntropySliceCurStartCUAddr( uiStartCUAddrEntropySlice );
        }
        else
        {
          uiStartCUAddrSlice                                                            = pcSlice->getSliceCurEndCUAddr();
          uiStartCUAddrEntropySlice                                                     = pcSlice->getEntropySliceCurEndCUAddr();
        }        

        uiNextCUAddr = (uiStartCUAddrSlice > uiStartCUAddrEntropySlice) ? uiStartCUAddrSlice : uiStartCUAddrEntropySlice;
      }
      m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx++]                = pcSlice->getSliceCurEndCUAddr();
      m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx++]  = pcSlice->getSliceCurEndCUAddr();
      
      pcSlice = pcPic->getSlice(0);
      //-- Loop filter
      Bool bLFCrossTileBoundary = (pcSlice->getPPS()->getTileBehaviorControlPresentFlag() == 1)?
                                  (pcSlice->getPPS()->getLFCrossTileBoundaryFlag()):(pcSlice->getPPS()->getSPS()->getLFCrossTileBoundaryFlag());
      m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), pcSlice->getLoopFilterBetaOffset(), pcSlice->getLoopFilterTcOffset(), bLFCrossTileBoundary);
      m_pcLoopFilter->loopFilterPic( pcPic );

      pcSlice = pcPic->getSlice(0);
      if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
      {
        Int sliceGranularity = pcSlice->getPPS()->getSliceGranularity();
        pcPic->createNonDBFilterInfo(m_uiStoredStartCUAddrForEncodingSlice, uiNumSlices, sliceGranularity, pcSlice->getSPS()->getLFCrossSliceBoundaryFlag(),pcPic->getPicSym()->getNumTiles() ,bLFCrossTileBoundary);
      }


      pcSlice = pcPic->getSlice(0);

      if(pcSlice->getSPS()->getUseSAO())
      {
        m_pcSAO->createPicSaoInfo(pcPic, uiNumSlices);
      }

      std::vector<AlfCUCtrlInfo> vAlfCUCtrlParam;


      pcSlice = pcPic->getSlice(0);

      if(pcSlice->getSPS()->getUseALF())
      {
        vAlfCUCtrlParam.resize(uiNumSlices);
        m_pcAdaptiveLoopFilter->createPicAlfInfo(pcPic, uiNumSlices);
      }

      /////////////////////////////////////////////////////////////////////////////////////////////////// File writing
      // Set entropy coder
      m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );

      /* write various header sets. */
      if ( m_bSeqFirst )
      {
        OutputNALUnit nalu(NAL_UNIT_SPS, NAL_REF_IDC_PRIORITY_HIGHEST);
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        m_pcEntropyCoder->encodeSPS(pcSlice->getSPS());
        writeRBSPTrailingBits(nalu.m_Bitstream);
        accessUnit.push_back(new NALUnitEBSP(nalu));

        nalu = NALUnit(NAL_UNIT_PPS, NAL_REF_IDC_PRIORITY_HIGHEST);
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        m_pcEntropyCoder->encodePPS(pcSlice->getPPS());
        writeRBSPTrailingBits(nalu.m_Bitstream);
        accessUnit.push_back(new NALUnitEBSP(nalu));

        m_bSeqFirst = false;
      }

      /* use the main bitstream buffer for storing the marshalled picture */
      m_pcEntropyCoder->setBitstream(NULL);

      uiStartCUAddrSliceIdx = 0;
      uiStartCUAddrSlice    = 0; 

      uiStartCUAddrEntropySliceIdx = 0;
      uiStartCUAddrEntropySlice    = 0; 
      uiNextCUAddr                 = 0;
      pcSlice = pcPic->getSlice(uiStartCUAddrSliceIdx);

      Int processingState = (pcSlice->getSPS()->getUseALF() || pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getScalingListFlag() || pcSlice->getSPS()->getUseDF())?(EXECUTE_INLOOPFILTER):(ENCODE_SLICE);

      static Int iCurrAPSIdx = 0;
      Int iCodedAPSIdx = 0;
      TComSlice* pcSliceForAPS = NULL;

      bool skippedSlice=false;
      while (uiNextCUAddr < uiRealEndAddress) // Iterate over all slices
      {
        switch(processingState)
        {
        case ENCODE_SLICE:
          {
        pcSlice->setNextSlice       ( false );
        pcSlice->setNextEntropySlice( false );
        if (uiNextCUAddr == m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx])
        {
          pcSlice = pcPic->getSlice(uiStartCUAddrSliceIdx);
          pcPic->setCurrSliceIdx(uiStartCUAddrSliceIdx);
          m_pcSliceEncoder->setSliceIdx(uiStartCUAddrSliceIdx);
          assert(uiStartCUAddrSliceIdx == pcSlice->getSliceIdx());
          // Reconstruction slice
          pcSlice->setSliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setSliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx+1 ] );
          // Entropy slice
          pcSlice->setEntropySliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setEntropySliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx+1 ] );

          pcSlice->setNextSlice       ( true );

          uiStartCUAddrSliceIdx++;
          uiStartCUAddrEntropySliceIdx++;
        } 
        else if (uiNextCUAddr == m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx])
        {
          // Entropy slice
          pcSlice->setEntropySliceCurStartCUAddr( uiNextCUAddr );  // to be used in encodeSlice() + context restriction
          pcSlice->setEntropySliceCurEndCUAddr  ( m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx+1 ] );

          pcSlice->setNextEntropySlice( true );

          uiStartCUAddrEntropySliceIdx++;
        }

      pcSlice->setRPS(pcPic->getSlice(0)->getRPS());
      pcSlice->setRPSidx(pcPic->getSlice(0)->getRPSidx());
        UInt uiDummyStartCUAddr;
        UInt uiDummyBoundingCUAddr;
        m_pcSliceEncoder->xDetermineStartAndBoundingCUAddr(uiDummyStartCUAddr,uiDummyBoundingCUAddr,pcPic,true);

        uiInternalAddress = pcPic->getPicSym()->getPicSCUAddr(pcSlice->getEntropySliceCurEndCUAddr()-1) % pcPic->getNumPartInCU();
        uiExternalAddress = pcPic->getPicSym()->getPicSCUAddr(pcSlice->getEntropySliceCurEndCUAddr()-1) / pcPic->getNumPartInCU();
        uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
        uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
        uiWidth = pcSlice->getSPS()->getWidth();
        uiHeight = pcSlice->getSPS()->getHeight();
        while(uiPosX>=uiWidth||uiPosY>=uiHeight)
        {
          uiInternalAddress--;
          uiPosX = ( uiExternalAddress % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth+ g_auiRasterToPelX[ g_auiZscanToRaster[uiInternalAddress] ];
          uiPosY = ( uiExternalAddress / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight+ g_auiRasterToPelY[ g_auiZscanToRaster[uiInternalAddress] ];
        }
        uiInternalAddress++;
        if(uiInternalAddress==pcPic->getNumPartInCU())
        {
          uiInternalAddress = 0;
          uiExternalAddress = pcPic->getPicSym()->getCUOrderMap(pcPic->getPicSym()->getInverseCUOrderMap(uiExternalAddress)+1);
        }
        UInt uiEndAddress = pcPic->getPicSym()->getPicSCUEncOrder(uiExternalAddress*pcPic->getNumPartInCU()+uiInternalAddress);
        if(uiEndAddress<=pcSlice->getEntropySliceCurStartCUAddr()) {
          UInt uiBoundingAddrSlice, uiBoundingAddrEntropySlice;
          uiBoundingAddrSlice        = m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx];          
          uiBoundingAddrEntropySlice = m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx];          
          uiNextCUAddr               = min(uiBoundingAddrSlice, uiBoundingAddrEntropySlice);
          if(pcSlice->isNextSlice())
          {
            skippedSlice=true;
          }
          continue;
        }
        if(skippedSlice) 
        {
          pcSlice->setNextSlice       ( true );
          pcSlice->setNextEntropySlice( false );
        }
        skippedSlice=false;
        if (pcSlice->getPPS()->getEntropyCodingMode())
        {
          pcSlice->allocSubstreamSizes( iNumSubstreams );
          for ( UInt ui = 0 ; ui < iNumSubstreams; ui++ )
            pcSubstreamsOut[ui].clear();
        }

          m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );
          m_pcEntropyCoder->resetEntropy      ();
        /* start slice NALunit */
        OutputNALUnit nalu(pcSlice->getNalUnitType(), pcSlice->isReferenced() ? NAL_REF_IDC_PRIORITY_HIGHEST: NAL_REF_IDC_PRIORITY_LOWEST, pcSlice->getTLayer(), true);
        Bool bEntropySlice = (!pcSlice->isNextSlice());
        if (!bEntropySlice)
        {
          uiOneBitstreamPerSliceLength = 0; // start of a new slice
        }

        // used while writing slice header
        Int iTransmitLWHeader = (m_pcCfg->getTileMarkerFlag()==0) ? 0 : 1;
        pcSlice->setTileMarkerFlag ( iTransmitLWHeader );
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        pcSlice->setCABACinitIDC(pcSlice->getSliceType());
        m_pcEntropyCoder->encodeSliceHeader(pcSlice);
        if(pcSlice->isNextSlice())
        {
          if (pcSlice->getSPS()->getUseALF())
          {
            if(pcSlice->getAPS()->getAlfEnabled())
            {
              AlfCUCtrlInfo& cAlfCUCtrlParam = vAlfCUCtrlParam[pcSlice->getSliceIdx()];
              if(cAlfCUCtrlParam.cu_control_flag)
              {
                m_pcEntropyCoder->setAlfCtrl( true );
                m_pcEntropyCoder->setMaxAlfCtrlDepth(cAlfCUCtrlParam.alf_max_depth);
                m_pcCavlcCoder->setAlfCtrl(true);
                m_pcCavlcCoder->setMaxAlfCtrlDepth(cAlfCUCtrlParam.alf_max_depth); 
              }
              else
              {
                m_pcEntropyCoder->setAlfCtrl(false);
              }
              m_pcEntropyCoder->encodeAlfCtrlParam(cAlfCUCtrlParam, m_pcAdaptiveLoopFilter->getNumCUsInPic());
            }
          }
        }
        m_pcEntropyCoder->encodeTileMarkerFlag(pcSlice);


        // is it needed?
        {
          if (!bEntropySlice)
          {
            pcBitstreamRedirect->writeAlignOne();
          }
          else
          {
          // We've not completed our slice header info yet, do the alignment later.
          }
          m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
          m_pcEntropyCoder->setEntropyCoder ( m_pcSbacCoder, pcSlice );
          m_pcEntropyCoder->resetEntropy    ();
          for ( UInt ui = 0 ; ui < pcSlice->getPPS()->getNumSubstreams() ; ui++ )
          {
            m_pcEntropyCoder->setEntropyCoder ( &pcSbacCoders[ui], pcSlice );
            m_pcEntropyCoder->resetEntropy    ();
          }
        }

        if(pcSlice->isNextSlice())
        {
          // set entropy coder for writing
          m_pcSbacCoder->init( (TEncBinIf*)m_pcBinCABAC );
          {
            for ( UInt ui = 0 ; ui < pcSlice->getPPS()->getNumSubstreams() ; ui++ )
            {
              m_pcEntropyCoder->setEntropyCoder ( &pcSbacCoders[ui], pcSlice );
              m_pcEntropyCoder->resetEntropy    ();
            }
            pcSbacCoders[0].load(m_pcSbacCoder);
            m_pcEntropyCoder->setEntropyCoder ( &pcSbacCoders[0], pcSlice );  //ALF is written in substream #0 with CABAC coder #0 (see ALF param encoding below)
          }
          m_pcEntropyCoder->resetEntropy    ();
          // File writing
          if (!bEntropySlice)
          {
            m_pcEntropyCoder->setBitstream(pcBitstreamRedirect);
          }
          else
          {
            m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
          }
          // for now, override the TILES_DECODER setting in order to write substreams.
            m_pcEntropyCoder->setBitstream    ( &pcSubstreamsOut[0] );

        }
        pcSlice->setFinalized(true);

          m_pcSbacCoder->load( &pcSbacCoders[0] );

        pcSlice->setTileOffstForMultES( uiOneBitstreamPerSliceLength );
        if (!bEntropySlice)
        {
          pcSlice->setTileLocationCount ( 0 );
          m_pcSliceEncoder->encodeSlice(pcPic, pcBitstreamRedirect, pcSubstreamsOut); // redirect is only used for CAVLC tile position info.
        }
        else
        {
          m_pcSliceEncoder->encodeSlice(pcPic, &nalu.m_Bitstream, pcSubstreamsOut); // nalu.m_Bitstream is only used for CAVLC tile position info.
        }

        {
          // Construct the final bitstream by flushing and concatenating substreams.
          // The final bitstream is either nalu.m_Bitstream or pcBitstreamRedirect;
          UInt* puiSubstreamSizes = pcSlice->getSubstreamSizes();
          UInt uiTotalCodedSize = 0; // for padding calcs.
          UInt uiNumSubstreamsPerTile = iNumSubstreams;
          if (pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && pcSlice->getPPS()->getEntropyCodingSynchro())
            uiNumSubstreamsPerTile /= pcPic->getPicSym()->getNumTiles();
          for ( UInt ui = 0 ; ui < iNumSubstreams; ui++ )
          {
            // Flush all substreams -- this includes empty ones.
            // Terminating bit and flush.
            m_pcEntropyCoder->setEntropyCoder   ( &pcSbacCoders[ui], pcSlice );
            m_pcEntropyCoder->setBitstream      (  &pcSubstreamsOut[ui] );
            m_pcEntropyCoder->encodeTerminatingBit( 1 );
            m_pcEntropyCoder->encodeSliceFinish();
            pcSubstreamsOut[ui].write( 1, 1 ); // stop bit.
            // Byte alignment is necessary between tiles when tiles are independent.
            uiTotalCodedSize += pcSubstreamsOut[ui].getNumberOfWrittenBits();

            {
              Bool bNextSubstreamInNewTile = ((ui+1) < iNumSubstreams)
                                             && ((ui+1)%uiNumSubstreamsPerTile == 0);
              if (bNextSubstreamInNewTile)
              {
                // byte align.
                while (uiTotalCodedSize&0x7)
                {
                  pcSubstreamsOut[ui].write(0, 1);
                  uiTotalCodedSize++;
                }
              }
              Bool bRecordOffsetNext = m_pcCfg->getTileLocationInSliceHeaderFlag()
                                            && bNextSubstreamInNewTile;
              if (bRecordOffsetNext)
                pcSlice->setTileLocation(ui/uiNumSubstreamsPerTile, pcSlice->getTileOffstForMultES()+(uiTotalCodedSize>>3));
            }
            if (ui+1 < pcSlice->getPPS()->getNumSubstreams())
              puiSubstreamSizes[ui] = pcSubstreamsOut[ui].getNumberOfWrittenBits();
          }

          // Complete the slice header info.
          m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice );
          m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
          m_pcEntropyCoder->encodeSliceHeaderSubstreamTable(pcSlice);

          // Substreams...
          TComOutputBitstream *pcOut = pcBitstreamRedirect;
          // xWriteTileLocation will perform byte-alignment...
          {
            if (bEntropySlice)
            {
              // In these cases, padding is necessary here.
              pcOut = &nalu.m_Bitstream;
              pcOut->writeAlignOne();
            }
          }
          UInt uiAccumulatedLength = 0;
          for ( UInt ui = 0 ; ui < pcSlice->getPPS()->getNumSubstreams(); ui++ )
          {
            pcOut->addSubstream(&pcSubstreamsOut[ui]);

            // Update tile marker location information
            for (Int uiMrkIdx = 0; uiMrkIdx < pcSubstreamsOut[ui].getTileMarkerLocationCount(); uiMrkIdx++)
            {
              UInt uiBottom = pcOut->getTileMarkerLocationCount();
              pcOut->setTileMarkerLocation      ( uiBottom, uiAccumulatedLength + pcSubstreamsOut[ui].getTileMarkerLocation( uiMrkIdx ) );
              pcOut->setTileMarkerLocationCount ( uiBottom + 1 );
            }
            uiAccumulatedLength = (pcOut->getNumberOfWrittenBits() >> 3);
          }
        }

        UInt uiBoundingAddrSlice, uiBoundingAddrEntropySlice;
        uiBoundingAddrSlice        = m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx];          
        uiBoundingAddrEntropySlice = m_uiStoredStartCUAddrForEncodingEntropySlice[uiStartCUAddrEntropySliceIdx];          
        uiNextCUAddr               = min(uiBoundingAddrSlice, uiBoundingAddrEntropySlice);
        Bool bNextCUInNewSlice     = (uiNextCUAddr >= uiRealEndAddress) || (uiNextCUAddr == m_uiStoredStartCUAddrForEncodingSlice[uiStartCUAddrSliceIdx]);

        // If current NALU is the first NALU of slice (containing slice header) and more NALUs exist (due to multiple entropy slices) then buffer it.
        // If current NALU is the last NALU of slice and a NALU was buffered, then (a) Write current NALU (b) Update an write buffered NALU at approproate location in NALU list.
        Bool bNALUAlignedWrittenToList    = false; // used to ensure current NALU is not written more than once to the NALU list.
        if (pcSlice->getSPS()->getTileBoundaryIndependenceIdr() && !pcSlice->getSPS()->getTileBoundaryIndependenceIdr())
        {
          if (bNextCUInNewSlice)
          {
            if (!bEntropySlice) // there were no entropy slices
            {
              xWriteTileLocationToSliceHeader(nalu, pcBitstreamRedirect, pcSlice);
            }
            // (a) writing current NALU
            writeRBSPTrailingBits(nalu.m_Bitstream);
            accessUnit.push_back(new NALUnitEBSP(nalu));
            bNALUAlignedWrittenToList = true;

            // (b) update and write buffered NALU
            if (bEntropySlice) // if entropy slices existed in the slice then perform concatenation for the buffered nalu-bitstream and buffered payload bitstream
            {
              // Perform bitstream concatenation of slice header and partial slice payload
              xWriteTileLocationToSliceHeader((*naluBuffered), pcBitstreamRedirect, pcSlice);
              if (bIteratorAtListStart)
              {
                itLocationToPushSliceHeaderNALU = accessUnit.begin();
              }
              else
              {
                itLocationToPushSliceHeaderNALU++;
              }
              accessUnit.insert(itLocationToPushSliceHeaderNALU, (new NALUnitEBSP((*naluBuffered))) );

              // free buffered nalu
              delete naluBuffered;
              naluBuffered     = NULL;
            }
          }
          else // another entropy slice exists
          {
            // Is this start-of-slice NALU? i.e. the one containing slice header. If Yes, then buffer it.
            if (!bEntropySlice)
            {
              // store a pointer to where NALU for slice header is to be written in NALU list
              itLocationToPushSliceHeaderNALU = accessUnit.end();
              if (accessUnit.begin() == accessUnit.end())
              {
                bIteratorAtListStart = true;
              }
              else
              {
                bIteratorAtListStart = false;
                itLocationToPushSliceHeaderNALU--;
              }

              // buffer nalu for later writing
              naluBuffered = new OutputNALUnit(pcSlice->getNalUnitType(), pcSlice->isReferenced() ? NAL_REF_IDC_PRIORITY_HIGHEST: NAL_REF_IDC_PRIORITY_LOWEST, pcSlice->getTLayer(), true);
              copyNaluData( (*naluBuffered), nalu );

              // perform byte-alignment to get appropriate bitstream length (used for explicit tile location signaling in slice header)
              writeRBSPTrailingBits((*pcBitstreamRedirect));
              bNALUAlignedWrittenToList = true; // This is not really a write to bitsream but buffered for later. The flag is set to prevent writing of current NALU to list.
              uiOneBitstreamPerSliceLength += pcBitstreamRedirect->getNumberOfWrittenBits(); // length of bitstream after byte-alignment
            }
            else // write out entropy slice
            {
              writeRBSPTrailingBits(nalu.m_Bitstream);
              accessUnit.push_back(new NALUnitEBSP(nalu));
              bNALUAlignedWrittenToList = true; 
              uiOneBitstreamPerSliceLength += nalu.m_Bitstream.getNumberOfWrittenBits(); // length of bitstream after byte-alignment
            }
          }
        }
        else
        {
          xWriteTileLocationToSliceHeader(nalu, pcBitstreamRedirect, pcSlice);
          writeRBSPTrailingBits(nalu.m_Bitstream);
          accessUnit.push_back(new NALUnitEBSP(nalu));
          bNALUAlignedWrittenToList = true; 
          uiOneBitstreamPerSliceLength += nalu.m_Bitstream.getNumberOfWrittenBits(); // length of bitstream after byte-alignment
        }

        if (!bNALUAlignedWrittenToList)
        {
        {
          nalu.m_Bitstream.writeAlignZero();
        }
        accessUnit.push_back(new NALUnitEBSP(nalu));
        uiOneBitstreamPerSliceLength += nalu.m_Bitstream.getNumberOfWrittenBits() + 24; // length of bitstream after byte-alignment + 3 byte startcode 0x000001
        }


        processingState = ENCODE_SLICE;
          }
          break;
        case EXECUTE_INLOOPFILTER:
          {
            TComAPS cAPS;
            allocAPS(&cAPS, pcSlice->getSPS());
            // set entropy coder for RD
            m_pcEntropyCoder->setEntropyCoder ( m_pcCavlcCoder, pcSlice );

            if ( pcSlice->getSPS()->getUseSAO() )
            {
              m_pcEntropyCoder->resetEntropy();
              m_pcEntropyCoder->setBitstream( m_pcBitCounter );
              m_pcSAO->startSaoEnc(pcPic, m_pcEntropyCoder, m_pcEncTop->getRDSbacCoder(), NULL);
              SAOParam& cSaoParam = *(cAPS.getSaoParam());

#if SAO_CHROMA_LAMBDA 
              m_pcSAO->SAOProcess(&cSaoParam, pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma());
#else
#if ALF_CHROMA_LAMBDA
              m_pcSAO->SAOProcess(&cSaoParam, pcPic->getSlice(0)->getLambdaLuma());
#else
              m_pcSAO->SAOProcess(&cSaoParam, pcPic->getSlice(0)->getLambda());
#endif
#endif
              m_pcSAO->endSaoEnc();

              m_pcAdaptiveLoopFilter->PCMLFDisableProcess(pcPic);
            }

            // adaptive loop filter
            UInt64 uiDist, uiBits;
            if ( pcSlice->getSPS()->getUseALF())
            {
              m_pcEntropyCoder->resetEntropy    ();
              m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
              m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder );

              ALFParam& cAlfParam = *( cAPS.getAlfParam());

#if ALF_CHROMA_LAMBDA 
              m_pcAdaptiveLoopFilter->ALFProcess( &cAlfParam, &vAlfCUCtrlParam, pcPic->getSlice(0)->getLambdaLuma(), pcPic->getSlice(0)->getLambdaChroma(), uiDist, uiBits);
#else
#if SAO_CHROMA_LAMBDA 
              m_pcAdaptiveLoopFilter->ALFProcess( &cAlfParam, &vAlfCUCtrlParam, pcPic->getSlice(0)->getLambdaLuma(), uiDist, uiBits);
#else
              m_pcAdaptiveLoopFilter->ALFProcess( &cAlfParam, &vAlfCUCtrlParam, pcPic->getSlice(0)->getLambda(), uiDist, uiBits);
#endif
#endif
              m_pcAdaptiveLoopFilter->endALFEnc();

              m_pcAdaptiveLoopFilter->PCMLFDisableProcess(pcPic);
            }
            iCodedAPSIdx = iCurrAPSIdx;  
            pcSliceForAPS = pcSlice;

            assignNewAPS(cAPS, iCodedAPSIdx, vAPS, pcSliceForAPS);
            iCurrAPSIdx = (iCurrAPSIdx +1)%MAX_NUM_SUPPORTED_APS;
            processingState = ENCODE_APS;

            //set APS link to the slices
            for(Int s=0; s< uiNumSlices; s++)
            {
              if (pcSlice->getSPS()->getUseALF())
              {
                pcPic->getSlice(s)->setAlfEnabledFlag((cAPS.getAlfParam()->alf_flag==1)?true:false);
              }
              if (pcSlice->getSPS()->getUseSAO())
              {
                pcPic->getSlice(s)->setSaoEnabledFlag((cAPS.getSaoParam()->bSaoFlag[0]==1)?true:false);
              }
              pcPic->getSlice(s)->setAPS(&(vAPS[iCodedAPSIdx]));
              pcPic->getSlice(s)->setAPSId(iCodedAPSIdx);
            }
          }
          break;
        case ENCODE_APS:
          {
            OutputNALUnit nalu(NAL_UNIT_APS, NAL_REF_IDC_PRIORITY_HIGHEST);
            encodeAPS(&(vAPS[iCodedAPSIdx]), nalu.m_Bitstream, pcSliceForAPS);
            accessUnit.push_back(new NALUnitEBSP(nalu));

            processingState = ENCODE_SLICE;
          }
          break;
        default:
          {
            printf("Not a supported encoding state\n");
            assert(0);
            exit(-1);
          }
        }
      } // end iteration over slices


      if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
      {
        if(pcSlice->getSPS()->getUseSAO())
        {
          m_pcSAO->destroyPicSaoInfo();
        }

        if(pcSlice->getSPS()->getUseALF())
        {
          m_pcAdaptiveLoopFilter->destroyPicAlfInfo();
        }

        pcPic->destroyNonDBFilterInfo();
      }

      pcPic->compressMotion(); 
      
      //-- For time output for each slice
      Double dEncTime = (double)(clock()-iBeforeTime) / CLOCKS_PER_SEC;

      const char* digestStr = NULL;
      if (m_pcCfg->getPictureDigestEnabled())
      {
        /* calculate MD5sum for entire reconstructed picture */
        SEIpictureDigest sei_recon_picture_digest;
        sei_recon_picture_digest.method = SEIpictureDigest::MD5;
        calcMD5(*pcPic->getPicYuvRec(), sei_recon_picture_digest.digest);
        digestStr = digestToString(sei_recon_picture_digest.digest);

        OutputNALUnit nalu(NAL_UNIT_SEI, NAL_REF_IDC_PRIORITY_LOWEST);

        /* write the SEI messages */
        m_pcEntropyCoder->setEntropyCoder(m_pcCavlcCoder, pcSlice);
        m_pcEntropyCoder->setBitstream(&nalu.m_Bitstream);
        m_pcEntropyCoder->encodeSEI(sei_recon_picture_digest);
        writeRBSPTrailingBits(nalu.m_Bitstream);

        /* insert the SEI message NALUnit before any Slice NALUnits */
        AccessUnit::iterator it = find_if(accessUnit.begin(), accessUnit.end(), mem_fun(&NALUnit::isSlice));
        accessUnit.insert(it, new NALUnitEBSP(nalu));
      }

      xCalculateAddPSNR( pcPic, pcPic->getPicYuvRec(), accessUnit, dEncTime );
      if (digestStr)
        printf(" [MD5:%s]", digestStr);

#if FIXED_ROUNDING_FRAME_MEMORY
      /* TODO: this should happen after copyToPic(pcPicYuvRecOut) */
      pcPic->getPicYuvRec()->xFixedRoundingPic();
#endif
      pcPic->getPicYuvRec()->copyToPic(pcPicYuvRecOut);
      
      pcPic->setReconMark   ( true );

      pcPic->setUsedForTMVP ( true );

      m_bFirst = false;
      m_iNumPicCoded++;

      /* logging: insert a newline at end of picture period */
      printf("\n");
      fflush(stdout);
  }
  
  delete[] pcSubstreamsOut;
  delete pcBitstreamRedirect;

  assert ( m_iNumPicCoded == iNumPicRcvd );
}

/** Memory allocation for APS
  * \param [out] pAPS APS pointer
  * \param [in] pSPS SPS pointer
  */
Void TEncGOP::allocAPS (TComAPS* pAPS, TComSPS* pSPS)
{
  if(pSPS->getUseSAO())
  {
    pAPS->createSaoParam();
    m_pcSAO->allocSaoParam(pAPS->getSaoParam());
  }
  if(pSPS->getUseALF())
  {
    pAPS->createAlfParam();
    m_pcAdaptiveLoopFilter->allocALFParam(pAPS->getAlfParam());
  }
}

/** Memory deallocation for APS
  * \param [out] pAPS APS pointer
  * \param [in] pSPS SPS pointer
  */
Void TEncGOP::freeAPS (TComAPS* pAPS, TComSPS* pSPS)
{
  if(pSPS->getUseSAO())
  {
    if(pAPS->getSaoParam() != NULL)
    {
      m_pcSAO->freeSaoParam(pAPS->getSaoParam());
      pAPS->destroySaoParam();

    }
  }
  if(pSPS->getUseALF())
  {
    if(pAPS->getAlfParam() != NULL)
    {
      m_pcAdaptiveLoopFilter->freeALFParam(pAPS->getAlfParam());
      pAPS->destroyAlfParam();
    }
  }
}

/** Assign APS object into APS container according to APS ID
  * \param [in] cAPS APS object
  * \param [in] apsID APS ID
  * \param [in,out] vAPS APS container
  * \param [in] pcSlice pointer to slice
  */
Void TEncGOP::assignNewAPS(TComAPS& cAPS, Int apsID, std::vector<TComAPS>& vAPS, TComSlice* pcSlice)
{

  cAPS.setAPSID(apsID);
  if(pcSlice->getPOC() == 0)
  {
    cAPS.setScalingListEnabled(pcSlice->getSPS()->getScalingListFlag());
  }
  else
  {
    cAPS.setScalingListEnabled(false);
  }

  cAPS.setSaoEnabled(pcSlice->getSPS()->getUseSAO() ? (cAPS.getSaoParam()->bSaoFlag[0] ):(false));
  cAPS.setAlfEnabled(pcSlice->getSPS()->getUseALF() ? (cAPS.getAlfParam()->alf_flag ==1):(false));

  cAPS.setLoopFilterOffsetInAPS(m_pcCfg->getLoopFilterOffsetInAPS());
  cAPS.setLoopFilterDisable(m_pcCfg->getLoopFilterDisable());
  cAPS.setLoopFilterBetaOffset(m_pcCfg->getLoopFilterBetaOffset());
  cAPS.setLoopFilterTcOffset(m_pcCfg->getLoopFilterTcOffset());

#if !PARAMSET_VLC_CLEANUP
  if(cAPS.getSaoEnabled() || cAPS.getAlfEnabled())
  {
    cAPS.setCABACForAPS( true );
    if(cAPS.getCABACForAPS())
    {
      cAPS.setCABACinitIDC(pcSlice->getSliceType());
      cAPS.setCABACinitQP(pcSlice->getSliceQp());
    }
  }
#endif

  //assign new APS into APS container
  Int apsBufSize= (Int)vAPS.size();

  if(apsID >= apsBufSize)
  {
    vAPS.resize(apsID +1);
  }

  freeAPS(&(vAPS[apsID]), pcSlice->getSPS());
  vAPS[apsID] = cAPS;
}


/** encode APS syntax elements
  * \param [in] pcAPS APS pointer
  * \param [in, out] APSbs bitstream
  * \param [in] pointer to slice (just used for entropy coder initialization)
  */
Void TEncGOP::encodeAPS(TComAPS* pcAPS, TComOutputBitstream& APSbs, TComSlice* pcSlice)
{
  m_pcEntropyCoder->setEntropyCoder   ( m_pcCavlcCoder, pcSlice);
  m_pcEntropyCoder->resetEntropy      ();
  m_pcEntropyCoder->setBitstream(&APSbs);

  m_pcEntropyCoder->encodeAPSInitInfo(pcAPS);
  if(pcAPS->getScalingListEnabled())
  {
    m_pcEntropyCoder->encodeScalingList( pcSlice->getScalingList() );
  }
  if(pcAPS->getLoopFilterOffsetInAPS())
  {
    m_pcEntropyCoder->encodeDFParams(pcAPS);
  }

  if(pcAPS->getSaoEnabled())
  {
    m_pcEntropyCoder->encodeSaoParam(pcAPS->getSaoParam());
  }

  if(pcAPS->getAlfEnabled())
  {
    m_pcEntropyCoder->encodeAlfParam(pcAPS->getAlfParam());
  }

  //neither SAO and ALF is enabled
  writeRBSPTrailingBits(APSbs);
}

Void TEncGOP::printOutSummary(UInt uiNumAllPicCoded)
{
  assert (uiNumAllPicCoded == m_gcAnalyzeAll.getNumPic());
  
    
  //--CFG_KDY
  m_gcAnalyzeAll.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeI.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeP.setFrmRate( m_pcCfg->getFrameRate() );
  m_gcAnalyzeB.setFrmRate( m_pcCfg->getFrameRate() );
  
  //-- all
  printf( "\n\nSUMMARY --------------------------------------------------------\n" );
  m_gcAnalyzeAll.printOut('a');
  
  printf( "\n\nI Slices--------------------------------------------------------\n" );
  m_gcAnalyzeI.printOut('i');
  
  printf( "\n\nP Slices--------------------------------------------------------\n" );
  m_gcAnalyzeP.printOut('p');
  
  printf( "\n\nB Slices--------------------------------------------------------\n" );
  m_gcAnalyzeB.printOut('b');
  
#if _SUMMARY_OUT_
  m_gcAnalyzeAll.printSummaryOut();
#endif
#if _SUMMARY_PIC_
  m_gcAnalyzeI.printSummary('I');
  m_gcAnalyzeP.printSummary('P');
  m_gcAnalyzeB.printSummary('B');
#endif

  printf("\nRVM: %.3lf\n" , xCalculateRVM());
}

Void TEncGOP::preLoopFilterPicAll( TComPic* pcPic, UInt64& ruiDist, UInt64& ruiBits )
{
  TComSlice* pcSlice = pcPic->getSlice(pcPic->getCurrSliceIdx());
  Bool bCalcDist = false;
  m_pcLoopFilter->setCfg(pcSlice->getLoopFilterDisable(), m_pcCfg->getLoopFilterBetaOffset(), m_pcCfg->getLoopFilterTcOffset(), m_pcCfg->getLFCrossTileBoundaryFlag());
  m_pcLoopFilter->loopFilterPic( pcPic );
  
  m_pcEntropyCoder->setEntropyCoder ( m_pcEncTop->getRDGoOnSbacCoder(), pcSlice );
  m_pcEntropyCoder->resetEntropy    ();
  m_pcEntropyCoder->setBitstream    ( m_pcBitCounter );
  pcSlice = pcPic->getSlice(0);
  if(pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
  {
    pcPic->createNonDBFilterInfo();
  }
  
  // Adaptive Loop filter
  if( pcSlice->getSPS()->getUseALF() )
  {
    m_pcAdaptiveLoopFilter->createPicAlfInfo(pcPic);
    ALFParam cAlfParam;
    m_pcAdaptiveLoopFilter->allocALFParam(&cAlfParam);
    
    m_pcAdaptiveLoopFilter->startALFEnc(pcPic, m_pcEntropyCoder);
    
#if ALF_CHROMA_LAMBDA  
    m_pcAdaptiveLoopFilter->ALFProcess(&cAlfParam, NULL, pcSlice->getLambdaLuma(), pcSlice->getLambdaChroma(), ruiDist, ruiBits);
#else
#if SAO_CHROMA_LAMBDA 
    m_pcAdaptiveLoopFilter->ALFProcess(&cAlfParam, NULL, pcSlice->getLambdaLuma(), ruiDist, ruiBits);
#else
    m_pcAdaptiveLoopFilter->ALFProcess(&cAlfParam, NULL, pcSlice->getLambda(), ruiDist, ruiBits);
#endif

#endif
    m_pcAdaptiveLoopFilter->endALFEnc();
    m_pcAdaptiveLoopFilter->freeALFParam(&cAlfParam);

    m_pcAdaptiveLoopFilter->PCMLFDisableProcess(pcPic);
    m_pcAdaptiveLoopFilter->destroyPicAlfInfo();
  }
  if( pcSlice->getSPS()->getUseSAO() || pcSlice->getSPS()->getUseALF())
  {
    pcPic->destroyNonDBFilterInfo();
  }
  
  m_pcEntropyCoder->resetEntropy    ();
  ruiBits += m_pcEntropyCoder->getNumberOfWrittenBits();
  
  if (!bCalcDist)
    ruiDist = xFindDistortionFrame(pcPic->getPicYuvOrg(), pcPic->getPicYuvRec());
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TEncGOP::xInitGOP( Int iPOCLast, Int iNumPicRcvd, TComList<TComPic*>& rcListPic, TComList<TComPicYuv*>& rcListPicYuvRecOut )
{
  assert( iNumPicRcvd > 0 );
  //  Exception for the first frame
  if ( iPOCLast == 0 )
  {
    m_iGopSize    = 1;
  }
  else
    m_iGopSize    = m_pcCfg->getGOPSize();
  
  assert (m_iGopSize > 0); 

  return;
}

Void TEncGOP::xGetBuffer( TComList<TComPic*>&       rcListPic,
                         TComList<TComPicYuv*>&    rcListPicYuvRecOut,
                         Int                       iNumPicRcvd,
                         Int                       iTimeOffset,
                         TComPic*&                 rpcPic,
                         TComPicYuv*&              rpcPicYuvRecOut,
                         UInt                      uiPOCCurr )
{
  Int i;
  //  Rec. output
  TComList<TComPicYuv*>::iterator     iterPicYuvRec = rcListPicYuvRecOut.end();
  for ( i = 0; i < iNumPicRcvd - iTimeOffset + 1; i++ )
  {
    iterPicYuvRec--;
  }
  
  rpcPicYuvRecOut = *(iterPicYuvRec);
  
  //  Current pic.
  TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
  while (iterPic != rcListPic.end())
  {
    rpcPic = *(iterPic);
    rpcPic->setCurrSliceIdx(0);
    if (rpcPic->getPOC() == (Int)uiPOCCurr)
    {
      break;
    }
    iterPic++;
  }
  
  assert (rpcPic->getPOC() == (Int)uiPOCCurr);
  
  return;
}

UInt64 TEncGOP::xFindDistortionFrame (TComPicYuv* pcPic0, TComPicYuv* pcPic1)
{
  Int     x, y;
  Pel*  pSrc0   = pcPic0 ->getLumaAddr();
  Pel*  pSrc1   = pcPic1 ->getLumaAddr();
#if IBDI_DISTORTION
  Int  iShift = g_uiBitIncrement;
  Int  iOffset = 1<<(g_uiBitIncrement-1);
#else
  UInt  uiShift = g_uiBitIncrement<<1;
#endif
  Int   iTemp;
  
  Int   iStride = pcPic0->getStride();
  Int   iWidth  = pcPic0->getWidth();
  Int   iHeight = pcPic0->getHeight();
  
  UInt64  uiTotalDiff = 0;
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
#if IBDI_DISTORTION
      iTemp = ((pSrc0[x]+iOffset)>>iShift) - ((pSrc1[x]+iOffset)>>iShift); uiTotalDiff += iTemp * iTemp;
#else
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
#endif
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }
  
  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  
  pSrc0  = pcPic0->getCbAddr();
  pSrc1  = pcPic1->getCbAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
#if IBDI_DISTORTION
      iTemp = ((pSrc0[x]+iOffset)>>iShift) - ((pSrc1[x]+iOffset)>>iShift); uiTotalDiff += iTemp * iTemp;
#else
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
#endif
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }
  
  pSrc0  = pcPic0->getCrAddr();
  pSrc1  = pcPic1->getCrAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
#if IBDI_DISTORTION
      iTemp = ((pSrc0[x]+iOffset)>>iShift) - ((pSrc1[x]+iOffset)>>iShift); uiTotalDiff += iTemp * iTemp;
#else
      iTemp = pSrc0[x] - pSrc1[x]; uiTotalDiff += (iTemp*iTemp) >> uiShift;
#endif
    }
    pSrc0 += iStride;
    pSrc1 += iStride;
  }
  
  return uiTotalDiff;
}

#if VERBOSE_RATE
static const char* nalUnitTypeToString(NalUnitType type)
{
  switch (type)
  {
  case NAL_UNIT_CODED_SLICE: return "SLICE";
  case NAL_UNIT_CODED_SLICE_CDR: return "CDR";
  case NAL_UNIT_CODED_SLICE_IDR: return "IDR";
  case NAL_UNIT_SEI: return "SEI";
  case NAL_UNIT_SPS: return "SPS";
  case NAL_UNIT_PPS: return "PPS";
  case NAL_UNIT_FILLER_DATA: return "FILLER";
  default: return "UNK";
  }
}
#endif

Void TEncGOP::xCalculateAddPSNR( TComPic* pcPic, TComPicYuv* pcPicD, const AccessUnit& accessUnit, Double dEncTime )
{
  Int     x, y;
  UInt64 uiSSDY  = 0;
  UInt64 uiSSDU  = 0;
  UInt64 uiSSDV  = 0;
  
  Double  dYPSNR  = 0.0;
  Double  dUPSNR  = 0.0;
  Double  dVPSNR  = 0.0;
  
  //===== calculate PSNR =====
  Pel*  pOrg    = pcPic ->getPicYuvOrg()->getLumaAddr();
  Pel*  pRec    = pcPicD->getLumaAddr();
  Int   iStride = pcPicD->getStride();
  
  Int   iWidth;
  Int   iHeight;
  
  iWidth  = pcPicD->getWidth () - m_pcEncTop->getPad(0);
  iHeight = pcPicD->getHeight() - m_pcEncTop->getPad(1);
  
  Int   iSize   = iWidth*iHeight;
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDY   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }
  
  iHeight >>= 1;
  iWidth  >>= 1;
  iStride >>= 1;
  pOrg  = pcPic ->getPicYuvOrg()->getCbAddr();
  pRec  = pcPicD->getCbAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDU   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }
  
  pOrg  = pcPic ->getPicYuvOrg()->getCrAddr();
  pRec  = pcPicD->getCrAddr();
  
  for( y = 0; y < iHeight; y++ )
  {
    for( x = 0; x < iWidth; x++ )
    {
      Int iDiff = (Int)( pOrg[x] - pRec[x] );
      uiSSDV   += iDiff * iDiff;
    }
    pOrg += iStride;
    pRec += iStride;
  }
  
  unsigned int maxval = 255 * (1<<(g_uiBitDepth + g_uiBitIncrement -8));
  Double fRefValueY = (double) maxval * maxval * iSize;
  Double fRefValueC = fRefValueY / 4.0;
  dYPSNR            = ( uiSSDY ? 10.0 * log10( fRefValueY / (Double)uiSSDY ) : 99.99 );
  dUPSNR            = ( uiSSDU ? 10.0 * log10( fRefValueC / (Double)uiSSDU ) : 99.99 );
  dVPSNR            = ( uiSSDV ? 10.0 * log10( fRefValueC / (Double)uiSSDV ) : 99.99 );

  /* calculate the size of the access unit, excluding:
   *  - any AnnexB contributions (start_code_prefix, zero_byte, etc.,)
   *  - SEI NAL units
   */
  unsigned numRBSPBytes = 0;
  for (AccessUnit::const_iterator it = accessUnit.begin(); it != accessUnit.end(); it++)
  {
    unsigned numRBSPBytes_nal = unsigned((*it)->m_nalUnitData.str().size());
#if VERBOSE_RATE
    printf("*** %6s numBytesInNALunit: %u\n", nalUnitTypeToString((*it)->m_UnitType), numRBSPBytes_nal);
#endif
    if ((*it)->m_UnitType != NAL_UNIT_SEI)
      numRBSPBytes += numRBSPBytes_nal;
  }

  unsigned uibits = numRBSPBytes * 8;
  m_vRVM_RP.push_back( uibits );

  //===== add PSNR =====
  m_gcAnalyzeAll.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  TComSlice*  pcSlice = pcPic->getSlice(0);
  if (pcSlice->isIntra())
  {
    m_gcAnalyzeI.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  if (pcSlice->isInterP())
  {
    m_gcAnalyzeP.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }
  if (pcSlice->isInterB())
  {
    m_gcAnalyzeB.addResult (dYPSNR, dUPSNR, dVPSNR, (Double)uibits);
  }

  Char c = (pcSlice->isIntra() ? 'I' : pcSlice->isInterP() ? 'P' : 'B');
  if (!pcSlice->isReferenced()) c += 32;

#if ADAPTIVE_QP_SELECTION
  printf("POC %4d TId: %1d ( %c-SLICE, nQP %d QP %d ) %10d bits",
         pcSlice->getPOC(),
         pcSlice->getTLayer(),
         c,
         pcSlice->getSliceQpBase(),
         pcSlice->getSliceQp(),
         uibits );
#else
  printf("POC %4d TId: %1d ( %c-SLICE, QP %d ) %10d bits",
         pcSlice->getPOC()-pcSlice->getLastIDR(),
         pcSlice->getTLayer(),
         c,
         pcSlice->getSliceQp(),
         uibits );
#endif

  printf(" [Y %6.4lf dB    U %6.4lf dB    V %6.4lf dB]", dYPSNR, dUPSNR, dVPSNR );
  printf(" [ET %5.0f ]", dEncTime );
  
  for (Int iRefList = 0; iRefList < 2; iRefList++)
  {
    printf(" [L%d ", iRefList);
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(RefPicList(iRefList)); iRefIndex++)
    {
      printf ("%d ", pcSlice->getRefPOC(RefPicList(iRefList), iRefIndex)-pcSlice->getLastIDR());
    }
    printf("]");
  }
  if(pcSlice->getNumRefIdx(REF_PIC_LIST_C)>0 && !pcSlice->getNoBackPredFlag())
  {
    printf(" [LC ");
    for (Int iRefIndex = 0; iRefIndex < pcSlice->getNumRefIdx(REF_PIC_LIST_C); iRefIndex++)
    {
      printf ("%d ", pcSlice->getRefPOC((RefPicList)pcSlice->getListIdFromIdxOfLC(iRefIndex), pcSlice->getRefIdxFromIdxOfLC(iRefIndex))-pcSlice->getLastIDR());
    }
    printf("]");
  }
}

/** Function for deciding the nal_unit_type.
 * \param uiPOCCurr POC of the current picture
 * \returns the nal_unit type of the picture
 * This function checks the configuration and returns the appropriate nal_unit_type for the picture.
 */
NalUnitType TEncGOP::getNalUnitType(UInt uiPOCCurr)
{
  if (uiPOCCurr == 0)
  {
    return NAL_UNIT_CODED_SLICE_IDR;
  }
  if (uiPOCCurr % m_pcCfg->getIntraPeriod() == 0)
  {
    if (m_pcCfg->getDecodingRefreshType() == 1)
    {
      return NAL_UNIT_CODED_SLICE_CDR;
    }
    else if (m_pcCfg->getDecodingRefreshType() == 2)
    {
      return NAL_UNIT_CODED_SLICE_IDR;
    }
  }
  return NAL_UNIT_CODED_SLICE;
}

Double TEncGOP::xCalculateRVM()
{
  Double dRVM = 0;
  
  if( m_pcCfg->getGOPSize() == 1 && m_pcCfg->getIntraPeriod() != 1 && m_pcCfg->getFrameToBeEncoded() > RVM_VCEGAM10_M * 2 )
  {
    // calculate RVM only for lowdelay configurations
    std::vector<Double> vRL , vB;
    size_t N = m_vRVM_RP.size();
    vRL.resize( N );
    vB.resize( N );
    
    Int i;
    Double dRavg = 0 , dBavg = 0;
    vB[RVM_VCEGAM10_M] = 0;
    for( i = RVM_VCEGAM10_M + 1 ; i < N - RVM_VCEGAM10_M + 1 ; i++ )
    {
      vRL[i] = 0;
      for( Int j = i - RVM_VCEGAM10_M ; j <= i + RVM_VCEGAM10_M - 1 ; j++ )
        vRL[i] += m_vRVM_RP[j];
      vRL[i] /= ( 2 * RVM_VCEGAM10_M );
      vB[i] = vB[i-1] + m_vRVM_RP[i] - vRL[i];
      dRavg += m_vRVM_RP[i];
      dBavg += vB[i];
    }
    
    dRavg /= ( N - 2 * RVM_VCEGAM10_M );
    dBavg /= ( N - 2 * RVM_VCEGAM10_M );
    
    double dSigamB = 0;
    for( i = RVM_VCEGAM10_M + 1 ; i < N - RVM_VCEGAM10_M + 1 ; i++ )
    {
      Double tmp = vB[i] - dBavg;
      dSigamB += tmp * tmp;
    }
    dSigamB = sqrt( dSigamB / ( N - 2 * RVM_VCEGAM10_M ) );
    
    double f = sqrt( 12.0 * ( RVM_VCEGAM10_M - 1 ) / ( RVM_VCEGAM10_M + 1 ) );
    
    dRVM = dSigamB / dRavg * f;
  }
  
  return( dRVM );
}

/** Determine the difference between consecutive tile sizes (in bytes) and writes it to  bistream rNalu [slice header]
 * \param rpcBitstreamRedirect contains the bitstream to be concatenated to rNalu. rpcBitstreamRedirect contains slice payload. rpcSlice contains tile location information.
 * \returns Updates rNalu to contain concatenated bitstream. rpcBitstreamRedirect is cleared at the end of this function call.
 */
Void TEncGOP::xWriteTileLocationToSliceHeader (OutputNALUnit& rNalu, TComOutputBitstream*& rpcBitstreamRedirect, TComSlice*& rpcSlice)
{
  {
    Int iTransmitTileLocationInSliceHeader = (rpcSlice->getTileLocationCount()==0 || m_pcCfg->getTileLocationInSliceHeaderFlag()==0) ? 0 : 1;
    rNalu.m_Bitstream.write(iTransmitTileLocationInSliceHeader, 1);   // write flag indicating whether tile location information communicated in slice header

    if (iTransmitTileLocationInSliceHeader)
    {
      rNalu.m_Bitstream.write(rpcSlice->getTileLocationCount()-1, 5);   // write number of tiles

      Int *aiDiff;
      aiDiff = new Int [rpcSlice->getTileLocationCount()];

      // Find largest number of bits required by Diff
      Int iLastSize = 0, iDiffMax = 0, iDiffMin = 0;
      for (UInt uiIdx=0; uiIdx<rpcSlice->getTileLocationCount(); uiIdx++)
      {
        Int iCurDiff, iCurSize;
        if (uiIdx==0)
        {
          iCurDiff  = rpcSlice->getTileLocation( uiIdx );
          iLastSize = rpcSlice->getTileLocation( uiIdx );
        }
        else
        {
          iCurSize  = rpcSlice->getTileLocation( uiIdx )  - rpcSlice->getTileLocation( uiIdx-1 );
          iCurDiff  = iCurSize - iLastSize;
          iLastSize = iCurSize;
        }
        // Store Diff so it may be written to slice header later without re-calculating.
        aiDiff[uiIdx] = iCurDiff;

        if (iCurDiff>iDiffMax)
        {
          iDiffMax = iCurDiff;
        }
        if (iCurDiff<iDiffMin)
        {
          iDiffMin = iCurDiff;
        }
      }

      Int iDiffMinAbs, iDiffMaxAbs;
      iDiffMinAbs = (iDiffMin<0) ? (-iDiffMin) : iDiffMin;
      iDiffMaxAbs = (iDiffMax<0) ? (-iDiffMax) : iDiffMax;

      Int iBitsUsedByDiff = 0, iDiffAbsLargest;
      iDiffAbsLargest = (iDiffMinAbs < iDiffMaxAbs) ? iDiffMaxAbs : iDiffMinAbs;        
      while (1)
      {
        if (iDiffAbsLargest >= (1 << iBitsUsedByDiff) )
        {
          iBitsUsedByDiff++;
        }
        else
        {
          break;
        }
      }
      iBitsUsedByDiff++;

      if (iBitsUsedByDiff > 32)
      {
        printf("\nDiff magnitude uses more than 32-bits");
        assert ( 0 );
        exit ( 0 ); // trying to catch any problems with using fixed bits for Diff information
      }

      rNalu.m_Bitstream.write( iBitsUsedByDiff-1, 5 ); // write number of bits used by Diff

      // Write diff to slice header (rNalu)
      for (UInt uiIdx=0; uiIdx<rpcSlice->getTileLocationCount(); uiIdx++)
      {
        Int iCurDiff = aiDiff[uiIdx];

        // write sign of diff
        if (uiIdx!=0)
        {
          if (iCurDiff<0)          
          {
            rNalu.m_Bitstream.write(1, 1);
          }
          else
          {
            rNalu.m_Bitstream.write(0, 1);
          }
        }

        // write abs value of diff
        Int iAbsDiff = (iCurDiff<0) ? (-iCurDiff) : iCurDiff;
        if (iAbsDiff > ((((UInt64)1)<<32)-1))
        {
          printf("\niAbsDiff exceeds 32-bit limit");
          exit(0);
        }
        rNalu.m_Bitstream.write( iAbsDiff, iBitsUsedByDiff-1 ); 
      }

      delete [] aiDiff;
    }
  }

  // Byte-align
  rNalu.m_Bitstream.writeAlignOne();

  // Update tile marker locations
  TComOutputBitstream *pcOut = &rNalu.m_Bitstream;
  UInt uiAccumulatedLength   = pcOut->getNumberOfWrittenBits() >> 3;
  for (Int uiMrkIdx = 0; uiMrkIdx < rpcBitstreamRedirect->getTileMarkerLocationCount(); uiMrkIdx++)
  {
    UInt uiBottom = pcOut->getTileMarkerLocationCount();
    pcOut->setTileMarkerLocation      ( uiBottom, uiAccumulatedLength + rpcBitstreamRedirect->getTileMarkerLocation( uiMrkIdx ) );
    pcOut->setTileMarkerLocationCount ( uiBottom + 1 );
  }

  // Perform bitstream concatenation
  if (rpcBitstreamRedirect->getNumberOfWrittenBits() > 0)
  {
    UInt uiBitCount  = rpcBitstreamRedirect->getNumberOfWrittenBits();
    if (rpcBitstreamRedirect->getByteStreamLength()>0)
    {
      UChar *pucStart  =  reinterpret_cast<UChar*>(rpcBitstreamRedirect->getByteStream());
      UInt uiWriteByteCount = 0;
      while (uiWriteByteCount < (uiBitCount >> 3) )
      {
        UInt uiBits = (*pucStart);
        rNalu.m_Bitstream.write(uiBits, 8);
        pucStart++;
        uiWriteByteCount++;
      }
    }
    UInt uiBitsHeld = (uiBitCount & 0x07);
    for (UInt uiIdx=0; uiIdx < uiBitsHeld; uiIdx++)
    {
      rNalu.m_Bitstream.write((rpcBitstreamRedirect->getHeldBits() & (1 << (7-uiIdx))) >> (7-uiIdx), 1);
    }          
  }

  m_pcEntropyCoder->setBitstream(&rNalu.m_Bitstream);

  delete rpcBitstreamRedirect;
  rpcBitstreamRedirect = new TComOutputBitstream;
}
//! \}
