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

/** \file     TComDataCU.cpp
    \brief    CU data structure
    \todo     not all entities are documented
*/

#include "TComDataCU.h"
#include "TComPic.h"

//! \ingroup TLibCommon
//! \{

#if ADAPTIVE_QP_SELECTION
Int * TComDataCU::m_pcGlbArlCoeffY  = NULL;
Int * TComDataCU::m_pcGlbArlCoeffCb = NULL;
Int * TComDataCU::m_pcGlbArlCoeffCr = NULL;
#endif

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComDataCU::TComDataCU()
{
  m_pcPic              = NULL;
  m_pcSlice            = NULL;
  m_puhDepth           = NULL;
  
  m_pePartSize         = NULL;
  m_pePredMode         = NULL;
  m_puiAlfCtrlFlag     = NULL;
  m_puiTmpAlfCtrlFlag  = NULL;
  m_puhWidth           = NULL;
  m_puhHeight          = NULL;
  m_phQP               = NULL;
  m_pbMergeFlag        = NULL;
  m_puhMergeIndex      = NULL;
  m_puhLumaIntraDir    = NULL;
  m_puhChromaIntraDir  = NULL;
  m_puhInterDir        = NULL;
  m_puhTrIdx           = NULL;
  m_puhCbf[0]          = NULL;
  m_puhCbf[1]          = NULL;
  m_puhCbf[2]          = NULL;
  m_pcTrCoeffY         = NULL;
  m_pcTrCoeffCb        = NULL;
  m_pcTrCoeffCr        = NULL;
#if ADAPTIVE_QP_SELECTION  
  m_pcArlCoeffY        = NULL;
  m_pcArlCoeffCb       = NULL;
  m_pcArlCoeffCr       = NULL;
#endif
  
  m_pbIPCMFlag         = NULL;
  m_pcIPCMSampleY      = NULL;
  m_pcIPCMSampleCb     = NULL;
  m_pcIPCMSampleCr     = NULL;

  m_pcPattern          = NULL;
  
  m_pcCUAboveLeft      = NULL;
  m_pcCUAboveRight     = NULL;
  m_pcCUAbove          = NULL;
  m_pcCULeft           = NULL;
  
  m_apcCUColocated[0]  = NULL;
  m_apcCUColocated[1]  = NULL;
  
  m_apiMVPIdx[0]       = NULL;
  m_apiMVPIdx[1]       = NULL;
  m_apiMVPNum[0]       = NULL;
  m_apiMVPNum[1]       = NULL;
  
  m_bDecSubCu          = false;
  m_uiSliceStartCU        = 0;
  m_uiEntropySliceStartCU = 0;
}

TComDataCU::~TComDataCU()
{
}

Void TComDataCU::create(UInt uiNumPartition, UInt uiWidth, UInt uiHeight, Bool bDecSubCu, Int unitSize
#if ADAPTIVE_QP_SELECTION
                        , Bool bGlobalRMARLBuffer
#endif                                              
                        )
{
  m_bDecSubCu = bDecSubCu;
  
  m_pcPic              = NULL;
  m_pcSlice            = NULL;
  m_uiNumPartition     = uiNumPartition;
  m_unitSize = unitSize;
  
  if ( !bDecSubCu )
  {
    m_phQP               = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhDepth           = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhWidth           = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_puhHeight          = (UChar*    )xMalloc(UChar,    uiNumPartition);
    m_pePartSize         = new Char[ uiNumPartition ];
    m_pePredMode         = new Char[ uiNumPartition ];
    
    m_puiAlfCtrlFlag     = new Bool[ uiNumPartition ];
    
    m_pbMergeFlag        = (Bool*  )xMalloc(Bool,   uiNumPartition);
    m_puhMergeIndex      = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhLumaIntraDir    = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhChromaIntraDir  = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhInterDir        = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_puhTrIdx           = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_puhCbf[0]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[1]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    m_puhCbf[2]          = (UChar* )xMalloc(UChar,  uiNumPartition);
    
    m_apiMVPIdx[0]       = new Char[ uiNumPartition ];
    m_apiMVPIdx[1]       = new Char[ uiNumPartition ];
    m_apiMVPNum[0]       = new Char[ uiNumPartition ];
    m_apiMVPNum[1]       = new Char[ uiNumPartition ];
    
    m_pcTrCoeffY         = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight);
    m_pcTrCoeffCb        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);
    m_pcTrCoeffCr        = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight/4);
#if ADAPTIVE_QP_SELECTION    
    if( bGlobalRMARLBuffer )
    {
      if( m_pcGlbArlCoeffY == NULL )
      {
        m_pcGlbArlCoeffY   = (Int*)xMalloc(Int, uiWidth*uiHeight);
        m_pcGlbArlCoeffCb  = (Int*)xMalloc(Int, uiWidth*uiHeight/4);
        m_pcGlbArlCoeffCr  = (Int*)xMalloc(Int, uiWidth*uiHeight/4);
      }
      m_pcArlCoeffY        = m_pcGlbArlCoeffY;
      m_pcArlCoeffCb       = m_pcGlbArlCoeffCb;
      m_pcArlCoeffCr       = m_pcGlbArlCoeffCr;
    }
    else
    {
      m_pcArlCoeffY        = (Int*)xMalloc(Int, uiWidth*uiHeight);
      m_pcArlCoeffCb       = (Int*)xMalloc(Int, uiWidth*uiHeight/4);
      m_pcArlCoeffCr       = (Int*)xMalloc(Int, uiWidth*uiHeight/4);
    }
#endif
    
    m_pbIPCMFlag         = (Bool*  )xMalloc(Bool, uiNumPartition);
    m_pcIPCMSampleY      = (Pel*   )xMalloc(Pel , uiWidth*uiHeight);
    m_pcIPCMSampleCb     = (Pel*   )xMalloc(Pel , uiWidth*uiHeight/4);
    m_pcIPCMSampleCr     = (Pel*   )xMalloc(Pel , uiWidth*uiHeight/4);

    m_acCUMvField[0].create( uiNumPartition );
    m_acCUMvField[1].create( uiNumPartition );
    
  }
  else
  {
    m_acCUMvField[0].setNumPartition(uiNumPartition );
    m_acCUMvField[1].setNumPartition(uiNumPartition );
  }
  
  m_uiSliceStartCU        = (UInt*  )xMalloc(UInt, uiNumPartition);
  m_uiEntropySliceStartCU = (UInt*  )xMalloc(UInt, uiNumPartition);
  
  // create pattern memory
  m_pcPattern            = (TComPattern*)xMalloc(TComPattern, 1);
  
  // create motion vector fields
  
  m_pcCUAboveLeft      = NULL;
  m_pcCUAboveRight     = NULL;
  m_pcCUAbove          = NULL;
  m_pcCULeft           = NULL;
  
  m_apcCUColocated[0]  = NULL;
  m_apcCUColocated[1]  = NULL;
}

Void TComDataCU::destroy()
{
  m_pcPic              = NULL;
  m_pcSlice            = NULL;
  
  if ( m_pcPattern )
  { 
    xFree(m_pcPattern);
    m_pcPattern = NULL;
  }
  
  // encoder-side buffer free
  if ( !m_bDecSubCu )
  {
    if ( m_phQP               ) { xFree(m_phQP);                m_phQP              = NULL; }
    if ( m_puhDepth           ) { xFree(m_puhDepth);            m_puhDepth          = NULL; }
    if ( m_puhWidth           ) { xFree(m_puhWidth);            m_puhWidth          = NULL; }
    if ( m_puhHeight          ) { xFree(m_puhHeight);           m_puhHeight         = NULL; }
    if ( m_pePartSize         ) { delete[] m_pePartSize;        m_pePartSize        = NULL; }
    if ( m_pePredMode         ) { delete[] m_pePredMode;        m_pePredMode        = NULL; }
    if ( m_puhCbf[0]          ) { xFree(m_puhCbf[0]);           m_puhCbf[0]         = NULL; }
    if ( m_puhCbf[1]          ) { xFree(m_puhCbf[1]);           m_puhCbf[1]         = NULL; }
    if ( m_puhCbf[2]          ) { xFree(m_puhCbf[2]);           m_puhCbf[2]         = NULL; }
    if ( m_puiAlfCtrlFlag     ) { delete[] m_puiAlfCtrlFlag;    m_puiAlfCtrlFlag    = NULL; }
    if ( m_puhInterDir        ) { xFree(m_puhInterDir);         m_puhInterDir       = NULL; }
    if ( m_pbMergeFlag        ) { xFree(m_pbMergeFlag);         m_pbMergeFlag       = NULL; }
    if ( m_puhMergeIndex      ) { xFree(m_puhMergeIndex);       m_puhMergeIndex     = NULL; }
    if ( m_puhLumaIntraDir    ) { xFree(m_puhLumaIntraDir);     m_puhLumaIntraDir   = NULL; }
    if ( m_puhChromaIntraDir  ) { xFree(m_puhChromaIntraDir);   m_puhChromaIntraDir = NULL; }
    if ( m_puhTrIdx           ) { xFree(m_puhTrIdx);            m_puhTrIdx          = NULL; }
    if ( m_pcTrCoeffY         ) { xFree(m_pcTrCoeffY);          m_pcTrCoeffY        = NULL; }
    if ( m_pcTrCoeffCb        ) { xFree(m_pcTrCoeffCb);         m_pcTrCoeffCb       = NULL; }
    if ( m_pcTrCoeffCr        ) { xFree(m_pcTrCoeffCr);         m_pcTrCoeffCr       = NULL; }
#if ADAPTIVE_QP_SELECTION
    if ( m_pcGlbArlCoeffY     ) { xFree(m_pcGlbArlCoeffY);      m_pcGlbArlCoeffY    = NULL; }
    if ( m_pcGlbArlCoeffCb    ) { xFree(m_pcGlbArlCoeffCb);     m_pcGlbArlCoeffCb   = NULL; }
    if ( m_pcGlbArlCoeffCr    ) { xFree(m_pcGlbArlCoeffCr);     m_pcGlbArlCoeffCr   = NULL; }
#endif
    if ( m_pbIPCMFlag         ) { xFree(m_pbIPCMFlag   );       m_pbIPCMFlag        = NULL; }
    if ( m_pcIPCMSampleY      ) { xFree(m_pcIPCMSampleY);       m_pcIPCMSampleY     = NULL; }
    if ( m_pcIPCMSampleCb     ) { xFree(m_pcIPCMSampleCb);      m_pcIPCMSampleCb    = NULL; }
    if ( m_pcIPCMSampleCr     ) { xFree(m_pcIPCMSampleCr);      m_pcIPCMSampleCr    = NULL; }
    if ( m_apiMVPIdx[0]       ) { delete[] m_apiMVPIdx[0];      m_apiMVPIdx[0]      = NULL; }
    if ( m_apiMVPIdx[1]       ) { delete[] m_apiMVPIdx[1];      m_apiMVPIdx[1]      = NULL; }
    if ( m_apiMVPNum[0]       ) { delete[] m_apiMVPNum[0];      m_apiMVPNum[0]      = NULL; }
    if ( m_apiMVPNum[1]       ) { delete[] m_apiMVPNum[1];      m_apiMVPNum[1]      = NULL; }
    
    m_acCUMvField[0].destroy();
    m_acCUMvField[1].destroy();
    
  }
  
  m_pcCUAboveLeft       = NULL;
  m_pcCUAboveRight      = NULL;
  m_pcCUAbove           = NULL;
  m_pcCULeft            = NULL;
  
  m_apcCUColocated[0]   = NULL;
  m_apcCUColocated[1]   = NULL;

  if( m_uiSliceStartCU )
  {
    xFree(m_uiSliceStartCU);
    m_uiSliceStartCU=NULL;
  }
  if(m_uiEntropySliceStartCU )
  {
    xFree(m_uiEntropySliceStartCU);
    m_uiEntropySliceStartCU=NULL;
  }
}

const NDBFBlockInfo& NDBFBlockInfo::operator= (const NDBFBlockInfo& src)
{
  this->tileID = src.tileID;
  this->sliceID= src.sliceID;
  this->startSU= src.startSU;
  this->endSU  = src.endSU;
  this->widthSU= src.widthSU;
  this->heightSU=src.heightSU;
  this->posX   = src.posX;
  this->posY   = src.posY;
  this->width  = src.width;
  this->height = src.height;
  ::memcpy(this->isBorderAvailable, src.isBorderAvailable, sizeof(Bool)*((Int)NUM_SGU_BORDER));

  return *this;
}


// ====================================================================================================================
// Public member functions
// ====================================================================================================================

// --------------------------------------------------------------------------------------------------------------------
// Initialization
// --------------------------------------------------------------------------------------------------------------------

/**
 - initialize top-level CU
 - internal buffers are already created
 - set values before encoding a CU
 .
 \param  pcPic     picture (TComPic) class pointer
 \param  iCUAddr   CU address
 */
Void TComDataCU::initCU( TComPic* pcPic, UInt iCUAddr )
{

  m_pcPic              = pcPic;
  m_pcSlice            = pcPic->getSlice(pcPic->getCurrSliceIdx());
  m_uiCUAddr           = iCUAddr;
  m_uiCUPelX           = ( iCUAddr % pcPic->getFrameWidthInCU() ) * g_uiMaxCUWidth;
  m_uiCUPelY           = ( iCUAddr / pcPic->getFrameWidthInCU() ) * g_uiMaxCUHeight;
  m_uiAbsIdxInLCU      = 0;
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiTotalBins        = 0;
  m_uiNumPartition     = pcPic->getNumPartInCU();
  
  for(int i=0; i<pcPic->getNumPartInCU(); i++) 
  {
    if(pcPic->getPicSym()->getInverseCUOrderMap(iCUAddr)*pcPic->getNumPartInCU()+i>=getSlice()->getSliceCurStartCUAddr())
    {
      m_uiSliceStartCU[i]=getSlice()->getSliceCurStartCUAddr();
    }
    else
    {
      m_uiSliceStartCU[i]=pcPic->getCU(getAddr())->m_uiSliceStartCU[i];
    }
  }
  for(int i=0; i<pcPic->getNumPartInCU(); i++) 
  {
    if(pcPic->getPicSym()->getInverseCUOrderMap(iCUAddr)*pcPic->getNumPartInCU()+i>=getSlice()->getEntropySliceCurStartCUAddr())
    {
      m_uiEntropySliceStartCU[i]=getSlice()->getEntropySliceCurStartCUAddr();
    }
    else
    {
      m_uiEntropySliceStartCU[i]=pcPic->getCU(getAddr())->m_uiEntropySliceStartCU[i];
    }
  }

  Int partStartIdx = getSlice()->getEntropySliceCurStartCUAddr() - pcPic->getPicSym()->getInverseCUOrderMap(iCUAddr) * pcPic->getNumPartInCU();

  Int numElements = min<Int>( partStartIdx, m_uiNumPartition );
  for ( Int ui = 0; ui < numElements; ui++ )
  {
    TComDataCU * pcFrom = pcPic->getCU(getAddr());
    m_pePartSize[ui] = pcFrom->getPartitionSize(ui);
    m_pePredMode[ui] = pcFrom->getPredictionMode(ui);
    m_puhDepth[ui] = pcFrom->getDepth(ui);
    m_puhWidth  [ui] = pcFrom->getWidth(ui);
    m_puhHeight [ui] = pcFrom->getHeight(ui);
    m_puhTrIdx  [ui] = pcFrom->getTransformIdx(ui);
    m_apiMVPIdx[0][ui] = pcFrom->m_apiMVPIdx[0][ui];;
    m_apiMVPIdx[1][ui] = pcFrom->m_apiMVPIdx[1][ui];
    m_apiMVPNum[0][ui] = pcFrom->m_apiMVPNum[0][ui];
    m_apiMVPNum[1][ui] = pcFrom->m_apiMVPNum[1][ui];
    m_phQP[ui]=pcFrom->m_phQP[ui];
    m_puiAlfCtrlFlag[ui]=pcFrom->m_puiAlfCtrlFlag[ui];
    m_pbMergeFlag[ui]=pcFrom->m_pbMergeFlag[ui];
    m_puhMergeIndex[ui]=pcFrom->m_puhMergeIndex[ui];
    m_puhLumaIntraDir[ui]=pcFrom->m_puhLumaIntraDir[ui];
    m_puhChromaIntraDir[ui]=pcFrom->m_puhChromaIntraDir[ui];
    m_puhInterDir[ui]=pcFrom->m_puhInterDir[ui];
    m_puhCbf[0][ui]=pcFrom->m_puhCbf[0][ui];
    m_puhCbf[1][ui]=pcFrom->m_puhCbf[1][ui];
    m_puhCbf[2][ui]=pcFrom->m_puhCbf[2][ui];
    m_pbIPCMFlag[ui] = pcFrom->m_pbIPCMFlag[ui];
  }
  
  Int firstElement = max<Int>( partStartIdx, 0 );
  numElements = m_uiNumPartition - firstElement;
  
  if ( numElements > 0 )
  {
    memset( m_pePartSize        + firstElement, SIZE_NONE,                numElements * sizeof( *m_pePartSize ) );
    memset( m_pePredMode        + firstElement, MODE_NONE,                numElements * sizeof( *m_pePredMode ) );
    memset( m_puhDepth          + firstElement, 0,                        numElements * sizeof( *m_puhDepth ) );
    memset( m_puhTrIdx          + firstElement, 0,                        numElements * sizeof( *m_puhTrIdx ) );
    memset( m_puhWidth          + firstElement, g_uiMaxCUWidth,           numElements * sizeof( *m_puhWidth ) );
    memset( m_puhHeight         + firstElement, g_uiMaxCUHeight,          numElements * sizeof( *m_puhHeight ) );
    memset( m_apiMVPIdx[0]      + firstElement, -1,                       numElements * sizeof( *m_apiMVPIdx[0] ) );
    memset( m_apiMVPIdx[1]      + firstElement, -1,                       numElements * sizeof( *m_apiMVPIdx[1] ) );
    memset( m_apiMVPNum[0]      + firstElement, -1,                       numElements * sizeof( *m_apiMVPNum[0] ) );
    memset( m_apiMVPNum[1]      + firstElement, -1,                       numElements * sizeof( *m_apiMVPNum[1] ) );
    memset( m_phQP              + firstElement, getSlice()->getSliceQp(), numElements * sizeof( *m_phQP ) );
    memset( m_puiAlfCtrlFlag    + firstElement, false,                    numElements * sizeof( *m_puiAlfCtrlFlag ) );
    memset( m_pbMergeFlag       + firstElement, false,                    numElements * sizeof( *m_pbMergeFlag ) );
    memset( m_puhMergeIndex     + firstElement, 0,                        numElements * sizeof( *m_puhMergeIndex ) );
    memset( m_puhLumaIntraDir   + firstElement, 2,                        numElements * sizeof( *m_puhLumaIntraDir ) );
    memset( m_puhChromaIntraDir + firstElement, 0,                        numElements * sizeof( *m_puhChromaIntraDir ) );
    memset( m_puhInterDir       + firstElement, 0,                        numElements * sizeof( *m_puhInterDir ) );
    memset( m_puhCbf[0]         + firstElement, 0,                        numElements * sizeof( *m_puhCbf[0] ) );
    memset( m_puhCbf[1]         + firstElement, 0,                        numElements * sizeof( *m_puhCbf[1] ) );
    memset( m_puhCbf[2]         + firstElement, 0,                        numElements * sizeof( *m_puhCbf[2] ) );
    memset( m_pbIPCMFlag        + firstElement, false,                    numElements * sizeof( *m_pbIPCMFlag ) );
  }
  
  UInt uiTmp = g_uiMaxCUWidth*g_uiMaxCUHeight;
  if ( 0 >= partStartIdx ) 
  {
    m_acCUMvField[0].clearMvField();
    m_acCUMvField[1].clearMvField();
    memset( m_pcTrCoeffY , 0, sizeof( TCoeff ) * uiTmp );
#if ADAPTIVE_QP_SELECTION
    memset( m_pcArlCoeffY , 0, sizeof( Int ) * uiTmp );  
#endif
    memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
    uiTmp  >>= 2;
    memset( m_pcTrCoeffCb, 0, sizeof( TCoeff ) * uiTmp );
    memset( m_pcTrCoeffCr, 0, sizeof( TCoeff ) * uiTmp );
#if ADAPTIVE_QP_SELECTION  
    memset( m_pcArlCoeffCb, 0, sizeof( Int ) * uiTmp );
    memset( m_pcArlCoeffCr, 0, sizeof( Int ) * uiTmp );
#endif
    memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
    memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
  }
  else 
  {
    TComDataCU * pcFrom = pcPic->getCU(getAddr());
    m_acCUMvField[0].copyFrom(&pcFrom->m_acCUMvField[0],m_uiNumPartition,0);
    m_acCUMvField[1].copyFrom(&pcFrom->m_acCUMvField[1],m_uiNumPartition,0);
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffY[i]=pcFrom->m_pcTrCoeffY[i];
#if ADAPTIVE_QP_SELECTION
      m_pcArlCoeffY[i]=pcFrom->m_pcArlCoeffY[i];
#endif
      m_pcIPCMSampleY[i]=pcFrom->m_pcIPCMSampleY[i];
    }
    for(int i=0; i<(uiTmp>>2); i++) 
    {
      m_pcTrCoeffCb[i]=pcFrom->m_pcTrCoeffCb[i];
      m_pcTrCoeffCr[i]=pcFrom->m_pcTrCoeffCr[i];
#if ADAPTIVE_QP_SELECTION
      m_pcArlCoeffCb[i]=pcFrom->m_pcArlCoeffCb[i];
      m_pcArlCoeffCr[i]=pcFrom->m_pcArlCoeffCr[i];
#endif
      m_pcIPCMSampleCb[i]=pcFrom->m_pcIPCMSampleCb[i];
      m_pcIPCMSampleCr[i]=pcFrom->m_pcIPCMSampleCr[i];
    }
  }

  // Setting neighbor CU
  m_pcCULeft        = NULL;
  m_pcCUAbove       = NULL;
  m_pcCUAboveLeft   = NULL;
  m_pcCUAboveRight  = NULL;

  m_apcCUColocated[0] = NULL;
  m_apcCUColocated[1] = NULL;

  UInt uiWidthInCU = pcPic->getFrameWidthInCU();
  if ( m_uiCUAddr % uiWidthInCU )
  {
    m_pcCULeft = pcPic->getCU( m_uiCUAddr - 1 );
  }

  if ( m_uiCUAddr / uiWidthInCU )
  {
    m_pcCUAbove = pcPic->getCU( m_uiCUAddr - uiWidthInCU );
  }

  if ( m_pcCULeft && m_pcCUAbove )
  {
    m_pcCUAboveLeft = pcPic->getCU( m_uiCUAddr - uiWidthInCU - 1 );
  }

  if ( m_pcCUAbove && ( (m_uiCUAddr%uiWidthInCU) < (uiWidthInCU-1) )  )
  {
    m_pcCUAboveRight = pcPic->getCU( m_uiCUAddr - uiWidthInCU + 1 );
  }

  if ( getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0 )
  {
    m_apcCUColocated[0] = getSlice()->getRefPic( REF_PIC_LIST_0, 0)->getCU( m_uiCUAddr );
  }

  if ( getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 )
  {
    m_apcCUColocated[1] = getSlice()->getRefPic( REF_PIC_LIST_1, 0)->getCU( m_uiCUAddr );
  }
}

/** initialize prediction data with enabling sub-LCU-level delta QP
*\param  uiDepth  depth of the current CU 
*\param  uiQP     QP for the current CU
*- set CU width and CU height according to depth
*- set QP value according to input QP 
*- set last-coded QP value according to input last-coded QP 
*/
Void TComDataCU::initEstData( UInt uiDepth, UInt uiQP )
{
  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiTotalBins        = 0;

  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;

  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    if(getPic()->getPicSym()->getInverseCUOrderMap(getAddr())*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU+ui >= getSlice()->getEntropySliceCurStartCUAddr())
    {
      m_apiMVPIdx[0][ui] = -1;
      m_apiMVPIdx[1][ui] = -1;
      m_apiMVPNum[0][ui] = -1;
      m_apiMVPNum[1][ui] = -1;
      m_puhDepth  [ui] = uiDepth;
      m_puhWidth  [ui] = uhWidth;
      m_puhHeight [ui] = uhHeight;
      m_puhTrIdx  [ui] = 0;
      m_pePartSize[ui] = SIZE_NONE;
      m_pePredMode[ui] = MODE_NONE;
      m_pbIPCMFlag[ui] = 0;
      m_phQP[ui] = uiQP;
      m_puiAlfCtrlFlag[ui]= false;
      m_pbMergeFlag[ui] = 0;
      m_puhMergeIndex[ui] = 0;
      m_puhLumaIntraDir[ui] = 2;
      m_puhChromaIntraDir[ui] = 0;
      m_puhInterDir[ui] = 0;
      m_puhCbf[0][ui] = 0;
      m_puhCbf[1][ui] = 0;
      m_puhCbf[2][ui] = 0;
    }
  }

  UInt uiTmp = uhWidth*uhHeight;

  if(getPic()->getPicSym()->getInverseCUOrderMap(getAddr())*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU >= getSlice()->getEntropySliceCurStartCUAddr())
  {
    m_acCUMvField[0].clearMvField();
    m_acCUMvField[1].clearMvField();
    uiTmp = uhWidth*uhHeight;
    
    memset( m_pcTrCoeffY,    0, uiTmp * sizeof( *m_pcTrCoeffY    ) );
#if ADAPTIVE_QP_SELECTION
    memset( m_pcArlCoeffY ,  0, uiTmp * sizeof( *m_pcArlCoeffY   ) );
#endif
    memset( m_pcIPCMSampleY, 0, uiTmp * sizeof( *m_pcIPCMSampleY ) );

    uiTmp>>=2;
    memset( m_pcTrCoeffCb,    0, uiTmp * sizeof( *m_pcTrCoeffCb    ) );
    memset( m_pcTrCoeffCr,    0, uiTmp * sizeof( *m_pcTrCoeffCr    ) );
#if ADAPTIVE_QP_SELECTION  
    memset( m_pcArlCoeffCb,   0, uiTmp * sizeof( *m_pcArlCoeffCb   ) );
    memset( m_pcArlCoeffCr,   0, uiTmp * sizeof( *m_pcArlCoeffCr   ) );
#endif
    memset( m_pcIPCMSampleCb, 0, uiTmp * sizeof( *m_pcIPCMSampleCb ) );
    memset( m_pcIPCMSampleCr, 0, uiTmp * sizeof( *m_pcIPCMSampleCr ) );
  }
}


// initialize Sub partition
Void TComDataCU::initSubCU( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth, UInt uiQP )
{
  assert( uiPartUnitIdx<4 );

  UInt uiPartOffset = ( pcCU->getTotalNumPart()>>2 )*uiPartUnitIdx;

  m_pcPic              = pcCU->getPic();
  m_pcSlice            = m_pcPic->getSlice(m_pcPic->getCurrSliceIdx());
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = pcCU->getZorderIdxInCU() + uiPartOffset;

  m_uiCUPelX           = pcCU->getCUPelX() + ( g_uiMaxCUWidth>>uiDepth  )*( uiPartUnitIdx &  1 );
  m_uiCUPelY           = pcCU->getCUPelY() + ( g_uiMaxCUHeight>>uiDepth  )*( uiPartUnitIdx >> 1 );

  m_dTotalCost         = MAX_DOUBLE;
  m_uiTotalDistortion  = 0;
  m_uiTotalBits        = 0;
  m_uiTotalBins        = 0;
  m_uiNumPartition     = pcCU->getTotalNumPart() >> 2;

  Int iSizeInUchar = sizeof( UChar  ) * m_uiNumPartition;
  Int iSizeInBool  = sizeof( Bool   ) * m_uiNumPartition;

  memset( m_phQP,              uiQP, iSizeInUchar );

  memset( m_puiAlfCtrlFlag,     0, iSizeInBool );
  memset( m_pbMergeFlag,        0, iSizeInBool  );
  memset( m_puhMergeIndex,      0, iSizeInUchar );
  memset( m_puhLumaIntraDir,    2, iSizeInUchar );
  memset( m_puhChromaIntraDir,  0, iSizeInUchar );
  memset( m_puhInterDir,        0, iSizeInUchar );
  memset( m_puhTrIdx,           0, iSizeInUchar );
  memset( m_puhCbf[0],          0, iSizeInUchar );
  memset( m_puhCbf[1],          0, iSizeInUchar );
  memset( m_puhCbf[2],          0, iSizeInUchar );
  memset( m_puhDepth,     uiDepth, iSizeInUchar );

  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;
  memset( m_puhWidth,          uhWidth,  iSizeInUchar );
  memset( m_puhHeight,         uhHeight, iSizeInUchar );
  memset( m_pbIPCMFlag,        0, iSizeInBool  );
  for (UInt ui = 0; ui < m_uiNumPartition; ui++)
  {
    m_pePartSize[ui] = SIZE_NONE;
    m_pePredMode[ui] = MODE_NONE;
    m_apiMVPIdx[0][ui] = -1;
    m_apiMVPIdx[1][ui] = -1;
    m_apiMVPNum[0][ui] = -1;
    m_apiMVPNum[1][ui] = -1;
    if(m_pcPic->getPicSym()->getInverseCUOrderMap(getAddr())*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU+ui<getSlice()->getEntropySliceCurStartCUAddr())
    {
      m_apiMVPIdx[0][ui] = pcCU->m_apiMVPIdx[0][uiPartOffset+ui];
      m_apiMVPIdx[1][ui] = pcCU->m_apiMVPIdx[1][uiPartOffset+ui];;
      m_apiMVPNum[0][ui] = pcCU->m_apiMVPNum[0][uiPartOffset+ui];;
      m_apiMVPNum[1][ui] = pcCU->m_apiMVPNum[1][uiPartOffset+ui];;
      m_puhDepth  [ui] = pcCU->getDepth(uiPartOffset+ui);
      m_puhWidth  [ui] = pcCU->getWidth(uiPartOffset+ui);
      m_puhHeight  [ui] = pcCU->getHeight(uiPartOffset+ui);
      m_puhTrIdx  [ui] = pcCU->getTransformIdx(uiPartOffset+ui);
      m_pePartSize[ui] = pcCU->getPartitionSize(uiPartOffset+ui);
      m_pePredMode[ui] = pcCU->getPredictionMode(uiPartOffset+ui);
      m_pbIPCMFlag[ui]=pcCU->m_pbIPCMFlag[uiPartOffset+ui];
      m_phQP[ui] = pcCU->m_phQP[uiPartOffset+ui];
      m_puiAlfCtrlFlag[ui]=pcCU->m_puiAlfCtrlFlag[uiPartOffset+ui];
      m_pbMergeFlag[ui]=pcCU->m_pbMergeFlag[uiPartOffset+ui];
      m_puhMergeIndex[ui]=pcCU->m_puhMergeIndex[uiPartOffset+ui];
      m_puhLumaIntraDir[ui]=pcCU->m_puhLumaIntraDir[uiPartOffset+ui];
      m_puhChromaIntraDir[ui]=pcCU->m_puhChromaIntraDir[uiPartOffset+ui];
      m_puhInterDir[ui]=pcCU->m_puhInterDir[uiPartOffset+ui];
      m_puhCbf[0][ui]=pcCU->m_puhCbf[0][uiPartOffset+ui];
      m_puhCbf[1][ui]=pcCU->m_puhCbf[1][uiPartOffset+ui];
      m_puhCbf[2][ui]=pcCU->m_puhCbf[2][uiPartOffset+ui];

    }
  }
  UInt uiTmp = uhWidth*uhHeight;
  memset( m_pcTrCoeffY , 0, sizeof(TCoeff)*uiTmp );
#if ADAPTIVE_QP_SELECTION  
  memset( m_pcArlCoeffY , 0, sizeof(Int)*uiTmp );
#endif
  memset( m_pcIPCMSampleY , 0, sizeof( Pel ) * uiTmp );
  uiTmp >>= 2;
  memset( m_pcTrCoeffCb, 0, sizeof(TCoeff)*uiTmp );
  memset( m_pcTrCoeffCr, 0, sizeof(TCoeff)*uiTmp );
#if ADAPTIVE_QP_SELECTION
  memset( m_pcArlCoeffCb, 0, sizeof(Int)*uiTmp );
  memset( m_pcArlCoeffCr, 0, sizeof(Int)*uiTmp );
#endif
  memset( m_pcIPCMSampleCb , 0, sizeof( Pel ) * uiTmp );
  memset( m_pcIPCMSampleCr , 0, sizeof( Pel ) * uiTmp );
  m_acCUMvField[0].clearMvField();
  m_acCUMvField[1].clearMvField();

  if(m_pcPic->getPicSym()->getInverseCUOrderMap(getAddr())*m_pcPic->getNumPartInCU()+m_uiAbsIdxInLCU<getSlice()->getEntropySliceCurStartCUAddr())
  {
    // Part of this CU contains data from an older slice. Now copy in that data.
    UInt uiMaxCuWidth=pcCU->getSlice()->getSPS()->getMaxCUWidth();
    UInt uiMaxCuHeight=pcCU->getSlice()->getSPS()->getMaxCUHeight();
    TComDataCU * bigCU = getPic()->getCU(getAddr());
    Int minui = uiPartOffset;
    minui = -minui;
    pcCU->m_acCUMvField[0].copyTo(&m_acCUMvField[0],minui,uiPartOffset,m_uiNumPartition);
    pcCU->m_acCUMvField[1].copyTo(&m_acCUMvField[1],minui,uiPartOffset,m_uiNumPartition);
    UInt uiCoffOffset = uiMaxCuWidth*uiMaxCuHeight*m_uiAbsIdxInLCU/pcCU->getPic()->getNumPartInCU();
    uiTmp = uhWidth*uhHeight;
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffY[i]=bigCU->m_pcTrCoeffY[uiCoffOffset+i];
#if ADAPTIVE_QP_SELECTION
      m_pcArlCoeffY[i]=bigCU->m_pcArlCoeffY[uiCoffOffset+i];
#endif
      m_pcIPCMSampleY[i]=bigCU->m_pcIPCMSampleY[uiCoffOffset+i];
    }
    uiTmp>>=2;
    uiCoffOffset>>=2;
    for(int i=0; i<uiTmp; i++) 
    {
      m_pcTrCoeffCr[i]=bigCU->m_pcTrCoeffCr[uiCoffOffset+i];
      m_pcTrCoeffCb[i]=bigCU->m_pcTrCoeffCb[uiCoffOffset+i];
#if ADAPTIVE_QP_SELECTION
      m_pcArlCoeffCr[i]=bigCU->m_pcArlCoeffCr[uiCoffOffset+i];
      m_pcArlCoeffCb[i]=bigCU->m_pcArlCoeffCb[uiCoffOffset+i];
#endif
      m_pcIPCMSampleCb[i]=bigCU->m_pcIPCMSampleCb[uiCoffOffset+i];
      m_pcIPCMSampleCr[i]=bigCU->m_pcIPCMSampleCr[uiCoffOffset+i];
    }
  }

  m_pcCULeft        = pcCU->getCULeft();
  m_pcCUAbove       = pcCU->getCUAbove();
  m_pcCUAboveLeft   = pcCU->getCUAboveLeft();
  m_pcCUAboveRight  = pcCU->getCUAboveRight();

  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  memcpy(m_uiSliceStartCU,pcCU->m_uiSliceStartCU+uiPartOffset,sizeof(UInt)*m_uiNumPartition);
  memcpy(m_uiEntropySliceStartCU,pcCU->m_uiEntropySliceStartCU+uiPartOffset,sizeof(UInt)*m_uiNumPartition);
}

Void TComDataCU::setOutsideCUPart( UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiNumPartition = m_uiNumPartition >> (uiDepth << 1);
  UInt uiSizeInUchar = sizeof( UChar  ) * uiNumPartition;

  UChar uhWidth  = g_uiMaxCUWidth  >> uiDepth;
  UChar uhHeight = g_uiMaxCUHeight >> uiDepth;
  memset( m_puhDepth    + uiAbsPartIdx,     uiDepth,  uiSizeInUchar );
  memset( m_puhWidth    + uiAbsPartIdx,     uhWidth,  uiSizeInUchar );
  memset( m_puhHeight   + uiAbsPartIdx,     uhHeight, uiSizeInUchar );
}

// --------------------------------------------------------------------------------------------------------------------
// Copy
// --------------------------------------------------------------------------------------------------------------------

Void TComDataCU::copySubCU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiPart = uiAbsPartIdx;
  
  m_pcPic              = pcCU->getPic();
  m_pcSlice            = pcCU->getSlice();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = uiAbsPartIdx;
  
  m_uiCUPelX           = pcCU->getCUPelX() + g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ];
  m_uiCUPelY           = pcCU->getCUPelY() + g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ];
  
  UInt uiWidth         = g_uiMaxCUWidth  >> uiDepth;
  UInt uiHeight        = g_uiMaxCUHeight >> uiDepth;
  
  m_phQP=pcCU->getQP()                    + uiPart;
  m_pePartSize = pcCU->getPartitionSize() + uiPart;
  m_pePredMode=pcCU->getPredictionMode()  + uiPart;
  
  m_pbMergeFlag         = pcCU->getMergeFlag()        + uiPart;
  m_puhMergeIndex       = pcCU->getMergeIndex()       + uiPart;

  m_puhLumaIntraDir     = pcCU->getLumaIntraDir()     + uiPart;
  m_puhChromaIntraDir   = pcCU->getChromaIntraDir()   + uiPart;
  m_puhInterDir         = pcCU->getInterDir()         + uiPart;
  m_puhTrIdx            = pcCU->getTransformIdx()     + uiPart;
  
  m_puhCbf[0]= pcCU->getCbf(TEXT_LUMA)            + uiPart;
  m_puhCbf[1]= pcCU->getCbf(TEXT_CHROMA_U)        + uiPart;
  m_puhCbf[2]= pcCU->getCbf(TEXT_CHROMA_V)        + uiPart;
  
  m_puhDepth=pcCU->getDepth()                     + uiPart;
  m_puhWidth=pcCU->getWidth()                     + uiPart;
  m_puhHeight=pcCU->getHeight()                   + uiPart;
  
  m_apiMVPIdx[0]=pcCU->getMVPIdx(REF_PIC_LIST_0)  + uiPart;
  m_apiMVPIdx[1]=pcCU->getMVPIdx(REF_PIC_LIST_1)  + uiPart;
  m_apiMVPNum[0]=pcCU->getMVPNum(REF_PIC_LIST_0)  + uiPart;
  m_apiMVPNum[1]=pcCU->getMVPNum(REF_PIC_LIST_1)  + uiPart;
  
  m_pbIPCMFlag         = pcCU->getIPCMFlag()        + uiPart;

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  UInt uiTmp = uiWidth*uiHeight;
  UInt uiMaxCuWidth=pcCU->getSlice()->getSPS()->getMaxCUWidth();
  UInt uiMaxCuHeight=pcCU->getSlice()->getSPS()->getMaxCUHeight();
  
  UInt uiCoffOffset = uiMaxCuWidth*uiMaxCuHeight*uiAbsPartIdx/pcCU->getPic()->getNumPartInCU();
  
  m_pcTrCoeffY = pcCU->getCoeffY() + uiCoffOffset;
#if ADAPTIVE_QP_SELECTION
  m_pcArlCoeffY= pcCU->getArlCoeffY() + uiCoffOffset;  
#endif
  m_pcIPCMSampleY = pcCU->getPCMSampleY() + uiCoffOffset;

  uiTmp >>= 2;
  uiCoffOffset >>=2;
  m_pcTrCoeffCb=pcCU->getCoeffCb() + uiCoffOffset;
  m_pcTrCoeffCr=pcCU->getCoeffCr() + uiCoffOffset;
#if ADAPTIVE_QP_SELECTION  
  m_pcArlCoeffCb=pcCU->getArlCoeffCb() + uiCoffOffset;
  m_pcArlCoeffCr=pcCU->getArlCoeffCr() + uiCoffOffset;
#endif
  m_pcIPCMSampleCb = pcCU->getPCMSampleCb() + uiCoffOffset;
  m_pcIPCMSampleCr = pcCU->getPCMSampleCr() + uiCoffOffset;

  m_acCUMvField[0].linkToWithOffset( pcCU->getCUMvField(REF_PIC_LIST_0), uiPart );
  m_acCUMvField[1].linkToWithOffset( pcCU->getCUMvField(REF_PIC_LIST_1), uiPart );
  memcpy(m_uiSliceStartCU,pcCU->m_uiSliceStartCU+uiPart,sizeof(UInt)*m_uiNumPartition);
  memcpy(m_uiEntropySliceStartCU,pcCU->m_uiEntropySliceStartCU+uiPart,sizeof(UInt)*m_uiNumPartition);
}

// Copy inter prediction info from the biggest CU
Void TComDataCU::copyInterPredInfoFrom    ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList )
{
  m_pcPic              = pcCU->getPic();
  m_pcSlice            = pcCU->getSlice();
  m_uiCUAddr           = pcCU->getAddr();
  m_uiAbsIdxInLCU      = uiAbsPartIdx;
  
  Int iRastPartIdx     = g_auiZscanToRaster[uiAbsPartIdx];
  m_uiCUPelX           = pcCU->getCUPelX() + m_pcPic->getMinCUWidth ()*( iRastPartIdx % m_pcPic->getNumPartInWidth() );
  m_uiCUPelY           = pcCU->getCUPelY() + m_pcPic->getMinCUHeight()*( iRastPartIdx / m_pcPic->getNumPartInWidth() );
  
  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  m_apcCUColocated[0]  = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1]  = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  m_pePartSize         = pcCU->getPartitionSize ()        + uiAbsPartIdx;
  m_pePredMode         = pcCU->getPredictionMode()        + uiAbsPartIdx;
  m_puhInterDir        = pcCU->getInterDir      ()        + uiAbsPartIdx;
  
  m_puhDepth           = pcCU->getDepth ()                + uiAbsPartIdx;
  m_puhWidth           = pcCU->getWidth ()                + uiAbsPartIdx;
  m_puhHeight          = pcCU->getHeight()                + uiAbsPartIdx;
  
  m_pbMergeFlag        = pcCU->getMergeFlag()             + uiAbsPartIdx;
  m_puhMergeIndex      = pcCU->getMergeIndex()            + uiAbsPartIdx;

  m_apiMVPIdx[eRefPicList] = pcCU->getMVPIdx(eRefPicList) + uiAbsPartIdx;
  m_apiMVPNum[eRefPicList] = pcCU->getMVPNum(eRefPicList) + uiAbsPartIdx;
  
  m_acCUMvField[ eRefPicList ].linkToWithOffset( pcCU->getCUMvField(eRefPicList), uiAbsPartIdx );

  memcpy(m_uiSliceStartCU,pcCU->m_uiSliceStartCU+uiAbsPartIdx,sizeof(UInt)*m_uiNumPartition);
  memcpy(m_uiEntropySliceStartCU,pcCU->m_uiEntropySliceStartCU+uiAbsPartIdx,sizeof(UInt)*m_uiNumPartition);
}

// Copy small CU to bigger CU.
// One of quarter parts overwritten by predicted sub part.
Void TComDataCU::copyPartFrom( TComDataCU* pcCU, UInt uiPartUnitIdx, UInt uiDepth )
{
  assert( uiPartUnitIdx<4 );
  
  m_dTotalCost         += pcCU->getTotalCost();
  m_uiTotalDistortion  += pcCU->getTotalDistortion();
  m_uiTotalBits        += pcCU->getTotalBits();
  
  UInt uiOffset         = pcCU->getTotalNumPart()*uiPartUnitIdx;
  
  UInt uiNumPartition = pcCU->getTotalNumPart();
  Int iSizeInUchar  = sizeof( UChar ) * uiNumPartition;
  Int iSizeInBool   = sizeof( Bool  ) * uiNumPartition;
  
  memcpy( m_phQP       + uiOffset, pcCU->getQP(),             iSizeInUchar                        );
  memcpy( m_pePartSize + uiOffset, pcCU->getPartitionSize(),  sizeof( *m_pePartSize ) * uiNumPartition );
  memcpy( m_pePredMode + uiOffset, pcCU->getPredictionMode(), sizeof( *m_pePredMode ) * uiNumPartition );
  
  memcpy( m_puiAlfCtrlFlag      + uiOffset, pcCU->getAlfCtrlFlag(),       iSizeInBool  );
  memcpy( m_pbMergeFlag         + uiOffset, pcCU->getMergeFlag(),         iSizeInBool  );
  memcpy( m_puhMergeIndex       + uiOffset, pcCU->getMergeIndex(),        iSizeInUchar );
  memcpy( m_puhLumaIntraDir     + uiOffset, pcCU->getLumaIntraDir(),      iSizeInUchar );
  memcpy( m_puhChromaIntraDir   + uiOffset, pcCU->getChromaIntraDir(),    iSizeInUchar );
  memcpy( m_puhInterDir         + uiOffset, pcCU->getInterDir(),          iSizeInUchar );
  memcpy( m_puhTrIdx            + uiOffset, pcCU->getTransformIdx(),      iSizeInUchar );
  
  memcpy( m_puhCbf[0] + uiOffset, pcCU->getCbf(TEXT_LUMA)    , iSizeInUchar );
  memcpy( m_puhCbf[1] + uiOffset, pcCU->getCbf(TEXT_CHROMA_U), iSizeInUchar );
  memcpy( m_puhCbf[2] + uiOffset, pcCU->getCbf(TEXT_CHROMA_V), iSizeInUchar );
  
  memcpy( m_puhDepth  + uiOffset, pcCU->getDepth(),  iSizeInUchar );
  memcpy( m_puhWidth  + uiOffset, pcCU->getWidth(),  iSizeInUchar );
  memcpy( m_puhHeight + uiOffset, pcCU->getHeight(), iSizeInUchar );
  
  memcpy( m_apiMVPIdx[0] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_0), iSizeInUchar );
  memcpy( m_apiMVPIdx[1] + uiOffset, pcCU->getMVPIdx(REF_PIC_LIST_1), iSizeInUchar );
  memcpy( m_apiMVPNum[0] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_0), iSizeInUchar );
  memcpy( m_apiMVPNum[1] + uiOffset, pcCU->getMVPNum(REF_PIC_LIST_1), iSizeInUchar );
  
  memcpy( m_pbIPCMFlag + uiOffset, pcCU->getIPCMFlag(), iSizeInBool );

  m_pcCUAboveLeft      = pcCU->getCUAboveLeft();
  m_pcCUAboveRight     = pcCU->getCUAboveRight();
  m_pcCUAbove          = pcCU->getCUAbove();
  m_pcCULeft           = pcCU->getCULeft();
  
  m_apcCUColocated[0] = pcCU->getCUColocated(REF_PIC_LIST_0);
  m_apcCUColocated[1] = pcCU->getCUColocated(REF_PIC_LIST_1);
  
  m_acCUMvField[0].copyFrom( pcCU->getCUMvField( REF_PIC_LIST_0 ), pcCU->getTotalNumPart(), uiOffset );
  m_acCUMvField[1].copyFrom( pcCU->getCUMvField( REF_PIC_LIST_1 ), pcCU->getTotalNumPart(), uiOffset );
  
  UInt uiTmp  = g_uiMaxCUWidth*g_uiMaxCUHeight >> (uiDepth<<1);
  UInt uiTmp2 = uiPartUnitIdx*uiTmp;
  memcpy( m_pcTrCoeffY  + uiTmp2, pcCU->getCoeffY(),  sizeof(TCoeff)*uiTmp );
#if ADAPTIVE_QP_SELECTION
  memcpy( m_pcArlCoeffY  + uiTmp2, pcCU->getArlCoeffY(),  sizeof(Int)*uiTmp );
#endif
  memcpy( m_pcIPCMSampleY + uiTmp2 , pcCU->getPCMSampleY(), sizeof(Pel) * uiTmp );

  uiTmp >>= 2; uiTmp2>>= 2;
  memcpy( m_pcTrCoeffCb + uiTmp2, pcCU->getCoeffCb(), sizeof(TCoeff)*uiTmp );
  memcpy( m_pcTrCoeffCr + uiTmp2, pcCU->getCoeffCr(), sizeof(TCoeff)*uiTmp );
#if ADAPTIVE_QP_SELECTION
  memcpy( m_pcArlCoeffCb + uiTmp2, pcCU->getArlCoeffCb(), sizeof(Int)*uiTmp );
  memcpy( m_pcArlCoeffCr + uiTmp2, pcCU->getArlCoeffCr(), sizeof(Int)*uiTmp );
#endif
  memcpy( m_pcIPCMSampleCb + uiTmp2 , pcCU->getPCMSampleCb(), sizeof(Pel) * uiTmp );
  memcpy( m_pcIPCMSampleCr + uiTmp2 , pcCU->getPCMSampleCr(), sizeof(Pel) * uiTmp );
  m_uiTotalBins += pcCU->getTotalBins();
  memcpy( m_uiSliceStartCU        + uiOffset, pcCU->m_uiSliceStartCU,        sizeof( UInt ) * uiNumPartition  );
  memcpy( m_uiEntropySliceStartCU + uiOffset, pcCU->m_uiEntropySliceStartCU, sizeof( UInt ) * uiNumPartition  );
}

// Copy current predicted part to a CU in picture.
// It is used to predict for next part
Void TComDataCU::copyToPic( UChar uhDepth )
{
  TComDataCU*& rpcCU = m_pcPic->getCU( m_uiCUAddr );
  
  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;
  
  Int iSizeInUchar  = sizeof( UChar ) * m_uiNumPartition;
  Int iSizeInBool   = sizeof( Bool  ) * m_uiNumPartition;
  
  memcpy( rpcCU->getQP() + m_uiAbsIdxInLCU, m_phQP, iSizeInUchar );
  
  memcpy( rpcCU->getPartitionSize()  + m_uiAbsIdxInLCU, m_pePartSize, sizeof( *m_pePartSize ) * m_uiNumPartition );
  memcpy( rpcCU->getPredictionMode() + m_uiAbsIdxInLCU, m_pePredMode, sizeof( *m_pePredMode ) * m_uiNumPartition );
  
  memcpy( rpcCU->getAlfCtrlFlag()    + m_uiAbsIdxInLCU, m_puiAlfCtrlFlag,    iSizeInBool  );
  
  memcpy( rpcCU->getMergeFlag()         + m_uiAbsIdxInLCU, m_pbMergeFlag,         iSizeInBool  );
  memcpy( rpcCU->getMergeIndex()        + m_uiAbsIdxInLCU, m_puhMergeIndex,       iSizeInUchar );
  memcpy( rpcCU->getLumaIntraDir()      + m_uiAbsIdxInLCU, m_puhLumaIntraDir,     iSizeInUchar );
  memcpy( rpcCU->getChromaIntraDir()    + m_uiAbsIdxInLCU, m_puhChromaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + m_uiAbsIdxInLCU, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()      + m_uiAbsIdxInLCU, m_puhTrIdx,            iSizeInUchar );
  
  memcpy( rpcCU->getCbf(TEXT_LUMA)     + m_uiAbsIdxInLCU, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + m_uiAbsIdxInLCU, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + m_uiAbsIdxInLCU, m_puhCbf[2], iSizeInUchar );
  
  memcpy( rpcCU->getDepth()  + m_uiAbsIdxInLCU, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + m_uiAbsIdxInLCU, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + m_uiAbsIdxInLCU, m_puhHeight, iSizeInUchar );
  
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPIdx[0], iSizeInUchar );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPIdx[1], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + m_uiAbsIdxInLCU, m_apiMVPNum[0], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + m_uiAbsIdxInLCU, m_apiMVPNum[1], iSizeInUchar );
  
  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU );
  
  memcpy( rpcCU->getIPCMFlag() + m_uiAbsIdxInLCU, m_pbIPCMFlag,         iSizeInBool  );

  UInt uiTmp  = (g_uiMaxCUWidth*g_uiMaxCUHeight)>>(uhDepth<<1);
  UInt uiTmp2 = m_uiAbsIdxInLCU*m_pcPic->getMinCUWidth()*m_pcPic->getMinCUHeight();
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
#if ADAPTIVE_QP_SELECTION  
  memcpy( rpcCU->getArlCoeffY()  + uiTmp2, m_pcArlCoeffY,  sizeof(Int)*uiTmp  );
#endif
  memcpy( rpcCU->getPCMSampleY() + uiTmp2 , m_pcIPCMSampleY, sizeof(Pel)*uiTmp );

  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );
#if ADAPTIVE_QP_SELECTION
  memcpy( rpcCU->getArlCoeffCb() + uiTmp2, m_pcArlCoeffCb, sizeof(Int)*uiTmp  );
  memcpy( rpcCU->getArlCoeffCr() + uiTmp2, m_pcArlCoeffCr, sizeof(Int)*uiTmp  );
#endif
  memcpy( rpcCU->getPCMSampleCb() + uiTmp2 , m_pcIPCMSampleCb, sizeof( Pel ) * uiTmp );
  memcpy( rpcCU->getPCMSampleCr() + uiTmp2 , m_pcIPCMSampleCr, sizeof( Pel ) * uiTmp );
  rpcCU->getTotalBins() = m_uiTotalBins;
  memcpy( rpcCU->m_uiSliceStartCU        + m_uiAbsIdxInLCU, m_uiSliceStartCU,        sizeof( UInt ) * m_uiNumPartition  );
  memcpy( rpcCU->m_uiEntropySliceStartCU + m_uiAbsIdxInLCU, m_uiEntropySliceStartCU, sizeof( UInt ) * m_uiNumPartition  );
}

Void TComDataCU::copyToPic( UChar uhDepth, UInt uiPartIdx, UInt uiPartDepth )
{
  TComDataCU*&  rpcCU       = m_pcPic->getCU( m_uiCUAddr );
  UInt          uiQNumPart  = m_uiNumPartition>>(uiPartDepth<<1);
  
  UInt uiPartStart          = uiPartIdx*uiQNumPart;
  UInt uiPartOffset         = m_uiAbsIdxInLCU + uiPartStart;
  
  rpcCU->getTotalCost()       = m_dTotalCost;
  rpcCU->getTotalDistortion() = m_uiTotalDistortion;
  rpcCU->getTotalBits()       = m_uiTotalBits;
  
  Int iSizeInUchar  = sizeof( UChar  ) * uiQNumPart;
  Int iSizeInBool   = sizeof( Bool   ) * uiQNumPart;
  
  memcpy( rpcCU->getQP() + uiPartOffset, m_phQP, iSizeInUchar );
  
  memcpy( rpcCU->getPartitionSize()  + uiPartOffset, m_pePartSize, sizeof( *m_pePartSize ) * uiQNumPart );
  memcpy( rpcCU->getPredictionMode() + uiPartOffset, m_pePredMode, sizeof( *m_pePredMode ) * uiQNumPart );
  
  memcpy( rpcCU->getAlfCtrlFlag()       + uiPartOffset, m_puiAlfCtrlFlag,      iSizeInBool  );
  memcpy( rpcCU->getMergeFlag()         + uiPartOffset, m_pbMergeFlag,         iSizeInBool  );
  memcpy( rpcCU->getMergeIndex()        + uiPartOffset, m_puhMergeIndex,       iSizeInUchar );
  memcpy( rpcCU->getLumaIntraDir()      + uiPartOffset, m_puhLumaIntraDir,     iSizeInUchar );
  memcpy( rpcCU->getChromaIntraDir()    + uiPartOffset, m_puhChromaIntraDir,   iSizeInUchar );
  memcpy( rpcCU->getInterDir()          + uiPartOffset, m_puhInterDir,         iSizeInUchar );
  memcpy( rpcCU->getTransformIdx()      + uiPartOffset, m_puhTrIdx,            iSizeInUchar );
  
  memcpy( rpcCU->getCbf(TEXT_LUMA)     + uiPartOffset, m_puhCbf[0], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_U) + uiPartOffset, m_puhCbf[1], iSizeInUchar );
  memcpy( rpcCU->getCbf(TEXT_CHROMA_V) + uiPartOffset, m_puhCbf[2], iSizeInUchar );
  
  memcpy( rpcCU->getDepth()  + uiPartOffset, m_puhDepth,  iSizeInUchar );
  memcpy( rpcCU->getWidth()  + uiPartOffset, m_puhWidth,  iSizeInUchar );
  memcpy( rpcCU->getHeight() + uiPartOffset, m_puhHeight, iSizeInUchar );
  
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPIdx[0], iSizeInUchar );
  memcpy( rpcCU->getMVPIdx(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPIdx[1], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_0) + uiPartOffset, m_apiMVPNum[0], iSizeInUchar );
  memcpy( rpcCU->getMVPNum(REF_PIC_LIST_1) + uiPartOffset, m_apiMVPNum[1], iSizeInUchar );
  m_acCUMvField[0].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_0 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  m_acCUMvField[1].copyTo( rpcCU->getCUMvField( REF_PIC_LIST_1 ), m_uiAbsIdxInLCU, uiPartStart, uiQNumPart );
  
  memcpy( rpcCU->getIPCMFlag() + uiPartOffset, m_pbIPCMFlag,         iSizeInBool  );

  UInt uiTmp  = (g_uiMaxCUWidth*g_uiMaxCUHeight)>>((uhDepth+uiPartDepth)<<1);
  UInt uiTmp2 = uiPartOffset*m_pcPic->getMinCUWidth()*m_pcPic->getMinCUHeight();
  memcpy( rpcCU->getCoeffY()  + uiTmp2, m_pcTrCoeffY,  sizeof(TCoeff)*uiTmp  );
#if ADAPTIVE_QP_SELECTION
  memcpy( rpcCU->getArlCoeffY()  + uiTmp2, m_pcArlCoeffY,  sizeof(Int)*uiTmp  );
#endif
 
  memcpy( rpcCU->getPCMSampleY() + uiTmp2 , m_pcIPCMSampleY, sizeof( Pel ) * uiTmp );

  uiTmp >>= 2; uiTmp2 >>= 2;
  memcpy( rpcCU->getCoeffCb() + uiTmp2, m_pcTrCoeffCb, sizeof(TCoeff)*uiTmp  );
  memcpy( rpcCU->getCoeffCr() + uiTmp2, m_pcTrCoeffCr, sizeof(TCoeff)*uiTmp  );
#if ADAPTIVE_QP_SELECTION
  memcpy( rpcCU->getArlCoeffCb() + uiTmp2, m_pcArlCoeffCb, sizeof(Int)*uiTmp  );
  memcpy( rpcCU->getArlCoeffCr() + uiTmp2, m_pcArlCoeffCr, sizeof(Int)*uiTmp  );
#endif

  memcpy( rpcCU->getPCMSampleCb() + uiTmp2 , m_pcIPCMSampleCb, sizeof( Pel ) * uiTmp );
  memcpy( rpcCU->getPCMSampleCr() + uiTmp2 , m_pcIPCMSampleCr, sizeof( Pel ) * uiTmp );
  rpcCU->getTotalBins() = m_uiTotalBins;
  memcpy( rpcCU->m_uiSliceStartCU        + uiPartOffset, m_uiSliceStartCU,        sizeof( UInt ) * uiQNumPart  );
  memcpy( rpcCU->m_uiEntropySliceStartCU + uiPartOffset, m_uiEntropySliceStartCU, sizeof( UInt ) * uiQNumPart  );
}

// --------------------------------------------------------------------------------------------------------------------
// Other public functions
// --------------------------------------------------------------------------------------------------------------------

TComDataCU* TComDataCU::getPULeft( UInt& uiLPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction
                                 , Bool bEnforceTileRestriction
                                 )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if ( !RasterAddress::isZeroCol( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
    if ( RasterAddress::isEqualCol( uiAbsPartIdx, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
    {
      TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
      if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
        ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
      {
        return NULL;
      }
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiLPartUnitIdx -= m_uiAbsIdxInLCU;
      if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
        ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
      {
        return NULL;
      }
      return this;
    }
  }
  
  uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + uiNumPartInCUWidth - 1 ];


  if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)))
      ||
       (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)))
      ||
       (bEnforceTileRestriction && ( m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))  )  )
      )
  {
    return NULL;
  }
  return m_pcCULeft;
}


TComDataCU* TComDataCU::getPUAbove( UInt& uiAPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction, Bool MotionDataCompresssion
                                  , Bool planarAtLCUBoundary 
                                  , Bool bEnforceTileRestriction 
                                  )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth ];
    if ( RasterAddress::isEqualRow( uiAbsPartIdx, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
    {
      TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
      if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
        ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
      {
        return NULL;
      }
      return m_pcPic->getCU( getAddr() );
    }
    else
    {
      uiAPartUnitIdx -= m_uiAbsIdxInLCU;
      if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
        ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
      {
        return NULL;
      }
      return this;
    }
  }

  if(planarAtLCUBoundary)
  {
    return NULL;
  }
  
  uiAPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  if(MotionDataCompresssion)
  {
    uiAPartUnitIdx = g_motionRefer[uiAPartUnitIdx];
  }

  if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)))
      ||
       (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || m_pcCUAbove->getSCUAddr()+uiAPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)))
      ||
       (bEnforceTileRestriction &&(m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAbove->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))))
      )
  {
    return NULL;
  }
  return m_pcCUAbove;
}

TComDataCU* TComDataCU::getPUAboveLeft( UInt& uiALPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction, Bool MotionDataCompresssion )
{
  UInt uiAbsPartIdx       = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[m_uiAbsIdxInLCU];
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if ( !RasterAddress::isZeroCol( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroRow( uiAbsPartIdx, uiNumPartInCUWidth ) )
    {
      uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - uiNumPartInCUWidth - 1 ];
      if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdx, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
      {
        TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
        if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
          ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)))) 
        {
          return NULL;
        }
        return m_pcPic->getCU( getAddr() );
      }
      else
      {
        uiALPartUnitIdx -= m_uiAbsIdxInLCU;
        if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
          ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
        {
          return NULL;
        }
        return this;
      }
    }
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx + getPic()->getNumPartInCU() - uiNumPartInCUWidth - 1 ];
    if(MotionDataCompresssion)
    {
      uiALPartUnitIdx = g_motionRefer[uiALPartUnitIdx];
    }
  if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || 
       m_pcCUAbove->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAbove->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || 
       m_pcCUAbove->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAbove->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
    {
      return NULL;
    }
    return m_pcCUAbove;
  }
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdx, uiNumPartInCUWidth ) )
  {
    uiALPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdx - 1 ];
  if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
    {
      return NULL;
    }
    return m_pcCULeft;
  }
  
  uiALPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - 1 ];
  if(MotionDataCompresssion)
  {
    uiALPartUnitIdx = g_motionRefer[uiALPartUnitIdx];
  }
  if ( (bEnforceSliceRestriction && (m_pcCUAboveLeft==NULL || m_pcCUAboveLeft->getSlice()==NULL || 
       m_pcCUAboveLeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAboveLeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCUAboveLeft==NULL || m_pcCUAboveLeft->getSlice()==NULL || 
       m_pcCUAboveLeft->getSCUAddr()+uiALPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAboveLeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
  {
    return NULL;
  }
  return m_pcCUAboveLeft;
}

TComDataCU* TComDataCU::getPUAboveRight( UInt& uiARPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction, Bool MotionDataCompresssion )
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanCol( uiAbsPartIdxRT, uiNumPartInCUWidth - 1, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + 1 ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxRT, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
        {
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
            ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
          {
            return NULL;
          }
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
          if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + 1 ];
    if(MotionDataCompresssion)
    {
      uiARPartUnitIdx = g_motionRefer[uiARPartUnitIdx];
    }

  if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || 
       m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAbove->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || 
       m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAbove->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
    {
      return NULL;
    }
    return m_pcCUAbove;
  }
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth ];
  if(MotionDataCompresssion)
  {
    uiARPartUnitIdx = g_motionRefer[uiARPartUnitIdx];
  }
  if ( (bEnforceSliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL ||
       m_pcPic->getPicSym()->getInverseCUOrderMap( m_pcCUAboveRight->getAddr()) > m_pcPic->getPicSym()->getInverseCUOrderMap( getAddr()) ||
       m_pcCUAboveRight->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAboveRight->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || 
       m_pcPic->getPicSym()->getInverseCUOrderMap( m_pcCUAboveRight->getAddr()) > m_pcPic->getPicSym()->getInverseCUOrderMap( getAddr()) ||
       m_pcCUAboveRight->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAboveRight->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
  {
    return NULL;
  }
  return m_pcCUAboveRight;
}

TComDataCU* TComDataCU::getPUBelowLeft( UInt& uiBLPartUnitIdx, UInt uiCurrPartUnitIdx, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + (m_puhHeight[0] / m_pcPic->getMinCUHeight() - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanRow( uiAbsPartIdxLB, m_pcPic->getNumPartInHeight() - 1, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroCol( uiAbsPartIdxLB, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth - 1 ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxLB, uiAbsZorderCUIdxLB, uiNumPartInCUWidth ) )
        {
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
            ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
          {
            return NULL;
          }
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
          if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiNumPartInCUWidth*2 - 1 ];
  if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
    {
      return NULL;
    }
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}

TComDataCU* TComDataCU::getPUBelowLeftAdi(UInt& uiBLPartUnitIdx, UInt uiPuHeight,  UInt uiCurrPartUnitIdx, UInt uiPartUnitOffset, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdxLB     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdxLB = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ((m_puhHeight[0] / m_pcPic->getMinCUHeight()) - 1)*m_pcPic->getNumPartInWidth();
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxLB] + (m_pcPic->getPicSym()->getMinCUHeight() * uiPartUnitOffset)) >= m_pcSlice->getSPS()->getHeight())
  {
    uiBLPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanRow( uiAbsPartIdxLB, m_pcPic->getNumPartInHeight() - uiPartUnitOffset, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroCol( uiAbsPartIdxLB, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxLB + uiPartUnitOffset * uiNumPartInCUWidth - 1 ] )
      {
        uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + uiPartUnitOffset * uiNumPartInCUWidth - 1 ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxLB, uiAbsZorderCUIdxLB, uiNumPartInCUWidth ) )
        {
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx)))
            ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))
          {
            return NULL;
          }
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiBLPartUnitIdx -= m_uiAbsIdxInLCU;
          if ((bEnforceSliceRestriction        && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
          return this;
        }
      }
      uiBLPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiBLPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxLB + (1+uiPartUnitOffset) * uiNumPartInCUWidth - 1 ];
  if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiBLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
    {
      return NULL;
    }
    return m_pcCULeft;
  }
  
  uiBLPartUnitIdx = MAX_UINT;
  return NULL;
}

TComDataCU* TComDataCU::getPUAboveRightAdi(UInt&  uiARPartUnitIdx, UInt uiPuWidth, UInt uiCurrPartUnitIdx, UInt uiPartUnitOffset, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction )
{
  UInt uiAbsPartIdxRT     = g_auiZscanToRaster[uiCurrPartUnitIdx];
  UInt uiAbsZorderCUIdx   = g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + (m_puhWidth[0] / m_pcPic->getMinCUWidth()) - 1;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  
  if( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxRT] + (m_pcPic->getPicSym()->getMinCUHeight() * uiPartUnitOffset)) >= m_pcSlice->getSPS()->getWidth() )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  if ( RasterAddress::lessThanCol( uiAbsPartIdxRT, uiNumPartInCUWidth - uiPartUnitOffset, uiNumPartInCUWidth ) )
  {
    if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
    {
      if ( uiCurrPartUnitIdx > g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + uiPartUnitOffset ] )
      {
        uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT - uiNumPartInCUWidth + uiPartUnitOffset ];
        if ( RasterAddress::isEqualRowOrCol( uiAbsPartIdxRT, uiAbsZorderCUIdx, uiNumPartInCUWidth ) )
        {
          TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
          if ((bEnforceSliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)))
           ||( bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx))))

          {
            return NULL;
          }
          return m_pcPic->getCU( getAddr() );
        }
        else
        {
          uiARPartUnitIdx -= m_uiAbsIdxInLCU;
          if ((bEnforceSliceRestriction && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL))
            ||(bEnforceEntropySliceRestriction && (this->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx) || m_pcSlice==NULL)))
          {
            return NULL;
          }
          return this;
        }
      }
      uiARPartUnitIdx = MAX_UINT;
      return NULL;
    }
    uiARPartUnitIdx = g_auiRasterToZscan[ uiAbsPartIdxRT + m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + uiPartUnitOffset ];
  if ( (bEnforceSliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || 
       m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAbove->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCUAbove==NULL || m_pcCUAbove->getSlice()==NULL || 
       m_pcCUAbove->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAbove->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
    {
      return NULL;
    }
    return m_pcCUAbove;
  }
  
  if ( !RasterAddress::isZeroRow( uiAbsPartIdxRT, uiNumPartInCUWidth ) )
  {
    uiARPartUnitIdx = MAX_UINT;
    return NULL;
  }
  
  uiARPartUnitIdx = g_auiRasterToZscan[ m_pcPic->getNumPartInCU() - uiNumPartInCUWidth + uiPartUnitOffset-1 ];
  if ( (bEnforceSliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL ||
       m_pcPic->getPicSym()->getInverseCUOrderMap( m_pcCUAboveRight->getAddr()) > m_pcPic->getPicSym()->getInverseCUOrderMap( getAddr()) ||
       m_pcCUAboveRight->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAboveRight->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCUAboveRight==NULL || m_pcCUAboveRight->getSlice()==NULL || 
       m_pcPic->getPicSym()->getInverseCUOrderMap( m_pcCUAboveRight->getAddr()) > m_pcPic->getPicSym()->getInverseCUOrderMap( getAddr()) ||
       m_pcCUAboveRight->getSCUAddr()+uiARPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrPartUnitIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCUAboveRight->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
  {
    return NULL;
  }
  return m_pcCUAboveRight;
}

/** Get left QpMinCu
*\param   uiLPartUnitIdx
*\param   uiCurrAbsIdxInLCU
*\param   bEnforceSliceRestriction
*\param   bEnforceEntropySliceRestriction
*\returns TComDataCU*   point of TComDataCU of left QpMinCu
*/
TComDataCU* TComDataCU::getQpMinCuLeft( UInt& uiLPartUnitIdx, UInt uiCurrAbsIdxInLCU, Bool bEnforceSliceRestriction, Bool bEnforceEntropySliceRestriction)
{
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

  UInt uiAbsRorderQpMinCUIdx   = g_auiZscanToRaster[(uiCurrAbsIdxInLCU>>(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1))];
  UInt uiNumPartInQpMinCUWidth = getSlice()->getPPS()->getMinCuDQPSize()>>2;

  UInt uiAbsZorderQpMinCUIdx   = (uiCurrAbsIdxInLCU>>(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))<<(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1));
  if( (uiCurrAbsIdxInLCU != uiAbsZorderQpMinCUIdx) && 
    ((bEnforceSliceRestriction && (m_pcPic->getCU( getAddr() )->getSliceStartCU(uiCurrAbsIdxInLCU) != m_pcPic->getCU( getAddr() )->getSliceStartCU (uiAbsZorderQpMinCUIdx))) ||
    (bEnforceEntropySliceRestriction &&(m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiCurrAbsIdxInLCU) != m_pcPic->getCU( getAddr() )->getEntropySliceStartCU (uiAbsZorderQpMinCUIdx)) )) )
  {
    return NULL;
  }

  if ( !RasterAddress::isZeroCol( uiAbsRorderQpMinCUIdx, uiNumPartInCUWidth ) )
  {
    uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsRorderQpMinCUIdx - uiNumPartInQpMinCUWidth ];
    TComDataCU* pcTempReconCU = m_pcPic->getCU( getAddr() );
    if ((bEnforceSliceRestriction        && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU       (uiAbsZorderQpMinCUIdx)))
      ||(bEnforceEntropySliceRestriction && (pcTempReconCU==NULL || pcTempReconCU->getSlice() == NULL || pcTempReconCU->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiAbsZorderQpMinCUIdx))))
    {
      return NULL;
    }
    return m_pcPic->getCU( getAddr() );
  }

  uiLPartUnitIdx = g_auiRasterToZscan[ uiAbsRorderQpMinCUIdx + uiNumPartInCUWidth - uiNumPartInQpMinCUWidth ];

  if ( (bEnforceSliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getSliceStartCU(uiAbsZorderQpMinCUIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))||
       (bEnforceEntropySliceRestriction && (m_pcCULeft==NULL || m_pcCULeft->getSlice()==NULL || 
       m_pcCULeft->getSCUAddr()+uiLPartUnitIdx < m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiAbsZorderQpMinCUIdx)||
       (m_pcPic->getPicSym()->getTileBoundaryIndependenceIdr() && m_pcPic->getPicSym()->getTileIdxMap( m_pcCULeft->getAddr() ) != m_pcPic->getPicSym()->getTileIdxMap(getAddr()))
       ))
     )
  {
    return NULL;
  }

  return m_pcCULeft;
}

/** Get reference QP from left QpMinCu or latest coded QP
*\param   uiCurrAbsIdxInLCU
*\returns UChar   reference QP value
*/
UChar TComDataCU::getRefQP( UInt uiCurrAbsIdxInLCU )
{
  // Left CU
  TComDataCU* pcCULeft;
  UInt        uiLPartIdx;
  pcCULeft = getQpMinCuLeft( uiLPartIdx, m_uiAbsIdxInLCU + uiCurrAbsIdxInLCU );
  if ( pcCULeft )
  {
    return pcCULeft->getQP(uiLPartIdx);
  }
  // Last QP
  return getLastCodedQP( uiCurrAbsIdxInLCU );
}

Int TComDataCU::getLastValidPartIdx( Int iAbsPartIdx )
{
  Int iLastValidPartIdx = iAbsPartIdx-1;
  while ( iLastValidPartIdx >= 0
       && getPredictionMode( iLastValidPartIdx ) == MODE_NONE )
  {
    UInt uiDepth = getDepth( iLastValidPartIdx );
    iLastValidPartIdx -= m_uiNumPartition>>(uiDepth<<1);
  }
  return iLastValidPartIdx;
}

UChar TComDataCU::getLastCodedQP( UInt uiAbsPartIdx )
{
  UInt uiQUPartIdxMask = ~((1<<(8-(getSlice()->getPPS()->getMaxCuDQPDepth()<<1)))-1);
  Int iLastValidPartIdx = getLastValidPartIdx( uiAbsPartIdx&uiQUPartIdxMask );
  if ( uiAbsPartIdx < m_uiNumPartition
    && (getSCUAddr()+iLastValidPartIdx < getSliceStartCU(m_uiAbsIdxInLCU+uiAbsPartIdx) || getSCUAddr()+iLastValidPartIdx < getEntropySliceStartCU(m_uiAbsIdxInLCU+uiAbsPartIdx) ))
  {
    return getSlice()->getSliceQp();
  }
  else
  if ( iLastValidPartIdx >= 0 )
  {
    return getQP( iLastValidPartIdx );
  }
  else
  {
    if ( getZorderIdxInCU() > 0 )
    {
      return getPic()->getCU( getAddr() )->getLastCodedQP( getZorderIdxInCU() );
    }
    else if ( getAddr() > 0 )
    {
      return getPic()->getCU( getAddr()-1 )->getLastCodedQP( getPic()->getNumPartInCU() );
    }
    else
    {
      return getSlice()->getSliceQp();
    }
  }
}

/** Get allowed chroma intra modes
*\param   uiAbsPartIdx
*\param   uiModeList  pointer to chroma intra modes array
*\returns 
*- fill uiModeList with chroma intra modes
*/
Void TComDataCU::getAllowedChromaDir( UInt uiAbsPartIdx, UInt* uiModeList )
{
  uiModeList[0] = PLANAR_IDX;
  uiModeList[1] = 1;
  uiModeList[2] = 2;
  uiModeList[3] = DC_IDX;
  uiModeList[4] = LM_CHROMA_IDX;
  uiModeList[5] = DM_CHROMA_IDX;

  UInt uiLumaMode = getLumaIntraDir( uiAbsPartIdx );

  for( Int i = 0; i < NUM_CHROMA_MODE - 2; i++ )
  {
    if( uiLumaMode == uiModeList[i] )
    {
      uiModeList[i] = 7; // VER+8 mode
      break;
    }
  }
}

/** Get most probable intra modes
*\param   uiAbsPartIdx
*\param   uiIntraDirPred  pointer to the array for MPM storage
*\param   piMode          it is set with MPM mode in case both MPM are equal. It is used to restrict RD search at encode side.
*\returns Number of MPM
*/
Int TComDataCU::getIntraDirLumaPredictor( UInt uiAbsPartIdx, Int* uiIntraDirPred, Int* piMode  )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  Int         iLeftIntraDir, iAboveIntraDir;
  Int         uiPredNum = 0;
  
  // Get intra direction of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  
  iLeftIntraDir  = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : PLANAR_IDX ) : PLANAR_IDX;
  
  // Get intra direction of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx, true, true, false, true );
  
  iAboveIntraDir = pcTempCU ? ( pcTempCU->isIntra( uiTempPartIdx ) ? pcTempCU->getLumaIntraDir( uiTempPartIdx ) : PLANAR_IDX ) : PLANAR_IDX;
  
  Int iIdx  = getIntraSizeIdx(uiAbsPartIdx);
  
  
  if ( iLeftIntraDir >= g_aucIntraModeNumAng[iIdx] ) 
  {
    iLeftIntraDir = PLANAR_IDX;
  }
  
  
  if ( iAboveIntraDir >= g_aucIntraModeNumAng[iIdx] ) 
  {
    iAboveIntraDir = PLANAR_IDX;
  }
  
  if(iLeftIntraDir == iAboveIntraDir)
  {
    uiPredNum = 2;
    
    if( piMode )
    {
      *piMode = iLeftIntraDir;
    }
    
    iAboveIntraDir = iLeftIntraDir == PLANAR_IDX ? DC_IDX : PLANAR_IDX; // DC or Planar
    
    assert( iLeftIntraDir != iAboveIntraDir );
    
    uiIntraDirPred[0] = min(iLeftIntraDir, iAboveIntraDir);
    uiIntraDirPred[1] = max(iLeftIntraDir, iAboveIntraDir);
  }
  else
  {
    uiPredNum = 2;
    uiIntraDirPred[0] = min(iLeftIntraDir, iAboveIntraDir);
    uiIntraDirPred[1] = max(iLeftIntraDir, iAboveIntraDir);
  }
  
  
  return uiPredNum;
}

UInt TComDataCU::getCtxSplitFlag( UInt uiAbsPartIdx, UInt uiDepth )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx;
  // Get left split flag
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx  = ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;
  
  // Get above split flag
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx += ( pcTempCU ) ? ( ( pcTempCU->getDepth( uiTempPartIdx ) > uiDepth ) ? 1 : 0 ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxQtCbf( UInt uiAbsPartIdx, TextType eType, UInt uiTrDepth )
{
  if( eType )
  {
    return uiTrDepth;
  }
  else
  {
    const UInt uiDepth = getDepth( uiAbsPartIdx );
    const UInt uiLog2TrafoSize = g_aucConvertToBit[ getSlice()->getSPS()->getMaxCUWidth() ] + 2 - uiDepth - uiTrDepth;
    const UInt uiCtx = uiTrDepth == 0 || uiLog2TrafoSize == getSlice()->getSPS()->getQuadtreeTULog2MaxSize() ? 1 : 0;
    return uiCtx;
  }
}

UInt TComDataCU::getQuadtreeTULog2MinSizeInCU( UInt uiIdx )
{
  UInt uiLog2MinTUSizeInCU = g_aucConvertToBit[getWidth( uiIdx )] + 2;
  
  if ( getPredictionMode( uiIdx ) == MODE_INTRA && getPartitionSize( uiIdx ) == SIZE_NxN )
  {
    uiLog2MinTUSizeInCU--;
  }
 
  UInt uiQuadtreeTUMaxDepth = getPredictionMode( uiIdx ) == MODE_INTRA ? m_pcSlice->getSPS()->getQuadtreeTUMaxDepthIntra() : m_pcSlice->getSPS()->getQuadtreeTUMaxDepthInter();

  PartSize  partSize  = getPartitionSize( uiIdx );
#if G519_TU_AMP_NSQT_HARMONIZATION
  Int   SplitFlag = ((uiQuadtreeTUMaxDepth == 1) && (getPredictionMode( uiIdx ) == MODE_INTER) && (partSize != SIZE_2Nx2N) );
#else
  Int   SplitFlag = ((uiQuadtreeTUMaxDepth == 1) && (getPredictionMode( uiIdx ) == MODE_INTER) && (partSize == SIZE_Nx2N || partSize == SIZE_2NxN) );
#endif

  if (uiLog2MinTUSizeInCU < m_pcSlice->getSPS()->getQuadtreeTULog2MinSize() + uiQuadtreeTUMaxDepth - 1 + SplitFlag)
  {
    uiLog2MinTUSizeInCU = m_pcSlice->getSPS()->getQuadtreeTULog2MinSize();  
  }
  else
  {
    uiLog2MinTUSizeInCU -= uiQuadtreeTUMaxDepth - 1 + SplitFlag;  
  }
  
  if ( uiLog2MinTUSizeInCU > m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize())
  {
    uiLog2MinTUSizeInCU = m_pcSlice->getSPS()->getQuadtreeTULog2MaxSize() - SplitFlag;
  }  

  return uiLog2MinTUSizeInCU;
}

UInt TComDataCU::getCtxSkipFlag( UInt uiAbsPartIdx )
{
  TComDataCU* pcTempCU;
  UInt        uiTempPartIdx;
  UInt        uiCtx = 0;
  
  // Get BCBP of left PU
  pcTempCU = getPULeft( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx    = ( pcTempCU ) ? pcTempCU->isSkipped( uiTempPartIdx ) : 0;
  
  // Get BCBP of above PU
  pcTempCU = getPUAbove( uiTempPartIdx, m_uiAbsIdxInLCU + uiAbsPartIdx );
  uiCtx   += ( pcTempCU ) ? pcTempCU->isSkipped( uiTempPartIdx ) : 0;
  
  return uiCtx;
}

UInt TComDataCU::getCtxInterDir( UInt uiAbsPartIdx )
{
  return getDepth( uiAbsPartIdx );
}

Void TComDataCU::setCbfSubParts( UInt uiCbfY, UInt uiCbfU, UInt uiCbfV, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhCbf[0] + uiAbsPartIdx, uiCbfY, sizeof( UChar ) * uiCurrPartNumb );
  memset( m_puhCbf[1] + uiAbsPartIdx, uiCbfU, sizeof( UChar ) * uiCurrPartNumb );
  memset( m_puhCbf[2] + uiAbsPartIdx, uiCbfV, sizeof( UChar ) * uiCurrPartNumb );
}

Void TComDataCU::setCbfSubParts( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhCbf[g_aucConvertTxtTypeToIdx[eTType]] + uiAbsPartIdx, uiCbf, sizeof( UChar ) * uiCurrPartNumb );
}

/** Sets a coded block flag for all sub-partitions of a partition
 * \param uiCbf The value of the coded block flag to be set
 * \param eTType
 * \param uiAbsPartIdx
 * \param uiPartIdx
 * \param uiDepth
 * \returns Void
 */
Void TComDataCU::setCbfSubParts ( UInt uiCbf, TextType eTType, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<UChar>( uiCbf, m_puhCbf[g_aucConvertTxtTypeToIdx[eTType]], uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setDepthSubParts( UInt uiDepth, UInt uiAbsPartIdx )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  memset( m_puhDepth + uiAbsPartIdx, uiDepth, sizeof(UChar)*uiCurrPartNumb );
}

Bool TComDataCU::isFirstAbsZorderIdxInDepth (UInt uiAbsPartIdx, UInt uiDepth)
{
  UInt uiPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  return (((m_uiAbsIdxInLCU + uiAbsPartIdx)% uiPartNumb) == 0);
}

Void TComDataCU::setAlfCtrlFlagSubParts( Bool uiFlag, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> ( 2 * uiDepth );
  memset( m_puiAlfCtrlFlag + uiAbsPartIdx, uiFlag, sizeof( *m_puiAlfCtrlFlag ) * uiCurrPartNumb );
}

Void TComDataCU::createTmpAlfCtrlFlag()
{
  m_puiTmpAlfCtrlFlag = new Bool[ m_uiNumPartition ];
}

Void TComDataCU::destroyTmpAlfCtrlFlag()
{
  if(m_puiTmpAlfCtrlFlag)
  {
    delete[] m_puiTmpAlfCtrlFlag;
    m_puiTmpAlfCtrlFlag = NULL;
  }
}

Void TComDataCU::copyAlfCtrlFlagToTmp()
{
  memcpy( m_puiTmpAlfCtrlFlag, m_puiAlfCtrlFlag, sizeof( *m_puiAlfCtrlFlag )*m_uiNumPartition );
}

Void TComDataCU::copyAlfCtrlFlagFromTmp()
{
  memcpy( m_puiAlfCtrlFlag, m_puiTmpAlfCtrlFlag, sizeof( *m_puiAlfCtrlFlag )*m_uiNumPartition );
}

Void TComDataCU::setPartSizeSubParts( PartSize eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert( sizeof( *m_pePartSize) == 1 );
  memset( m_pePartSize + uiAbsPartIdx, eMode, m_pcPic->getNumPartInCU() >> ( 2 * uiDepth ) );
}

Void TComDataCU::setPredModeSubParts( PredMode eMode, UInt uiAbsPartIdx, UInt uiDepth )
{
  assert( sizeof( *m_pePartSize) == 1 );
  memset( m_pePredMode + uiAbsPartIdx, eMode, m_pcPic->getNumPartInCU() >> ( 2 * uiDepth ) );
}

Void TComDataCU::setQPSubParts( UInt uiQP, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  TComSlice * pcSlice = getPic()->getSlice(getPic()->getCurrSliceIdx());

  for(UInt uiSCUIdx = uiAbsPartIdx; uiSCUIdx < uiAbsPartIdx+uiCurrPartNumb; uiSCUIdx++)
  {
    if( m_pcPic->getCU( getAddr() )->getEntropySliceStartCU(uiSCUIdx+getZorderIdxInCU()) == pcSlice->getEntropySliceCurStartCUAddr() )
    {
      m_phQP[uiSCUIdx] = uiQP;
    }
  }
}

Void TComDataCU::setLumaIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhLumaIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumb );
}

template<typename T>
Void TComDataCU::setSubPart( T uiParameter, T* puhBaseLCU, UInt uiCUAddr, UInt uiCUDepth, UInt uiPUIdx )
{
  assert( sizeof(T) == 1 ); // Using memset() works only for types of size 1
  
  UInt uiCurrPartNumQ = (m_pcPic->getNumPartInCU() >> (2 * uiCUDepth)) >> 2;
  switch ( m_pePartSize[ uiCUAddr ] )
  {
    case SIZE_2Nx2N:
      memset( puhBaseLCU + uiCUAddr, uiParameter, 4 * uiCurrPartNumQ );
      break;
    case SIZE_2NxN:
      memset( puhBaseLCU + uiCUAddr, uiParameter, 2 * uiCurrPartNumQ );
      break;
    case SIZE_Nx2N:
      memset( puhBaseLCU + uiCUAddr, uiParameter, uiCurrPartNumQ );
      memset( puhBaseLCU + uiCUAddr + 2 * uiCurrPartNumQ, uiParameter, uiCurrPartNumQ );
      break;
    case SIZE_NxN:
      memset( puhBaseLCU + uiCUAddr, uiParameter, uiCurrPartNumQ ); 
      break;
    case SIZE_2NxnU:
      if ( uiPUIdx == 0 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, (uiCurrPartNumQ >> 1) );                      
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ, uiParameter, (uiCurrPartNumQ >> 1) );                      
      }
      else if ( uiPUIdx == 1 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, (uiCurrPartNumQ >> 1) );                      
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ, uiParameter, ((uiCurrPartNumQ >> 1) + (uiCurrPartNumQ << 1)) );                      
      }
      else
      {
        assert(0);
      }
      break;
    case SIZE_2NxnD:
      if ( uiPUIdx == 0 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, ((uiCurrPartNumQ << 1) + (uiCurrPartNumQ >> 1)) );                      
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1) + uiCurrPartNumQ, uiParameter, (uiCurrPartNumQ >> 1) );                      
      }
      else if ( uiPUIdx == 1 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, (uiCurrPartNumQ >> 1) );                      
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ, uiParameter, (uiCurrPartNumQ >> 1) );                      
      }
      else
      {
        assert(0);
      }
      break;
    case SIZE_nLx2N:
      if ( uiPUIdx == 0 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, (uiCurrPartNumQ >> 2) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ >> 2) ); 
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1), uiParameter, (uiCurrPartNumQ >> 2) ); 
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1) + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ >> 2) ); 
      }
      else if ( uiPUIdx == 1 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, (uiCurrPartNumQ >> 2) );
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ + (uiCurrPartNumQ >> 2)) ); 
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1), uiParameter, (uiCurrPartNumQ >> 2) ); 
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1) + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ + (uiCurrPartNumQ >> 2)) ); 
      }
      else
      {
        assert(0);
      }
      break;
    case SIZE_nRx2N:
      if ( uiPUIdx == 0 )
      {      
        memset( puhBaseLCU + uiCUAddr, uiParameter, (uiCurrPartNumQ + (uiCurrPartNumQ >> 2)) );                           
        memset( puhBaseLCU + uiCUAddr + uiCurrPartNumQ + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ >> 2) );                           
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1), uiParameter, (uiCurrPartNumQ + (uiCurrPartNumQ >> 2)) );                           
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1) + uiCurrPartNumQ + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ >> 2) );                           
      }
      else if ( uiPUIdx == 1 )
      {
        memset( puhBaseLCU + uiCUAddr, uiParameter, (uiCurrPartNumQ >> 2) );                           
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ >> 2) );                           
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1), uiParameter, (uiCurrPartNumQ >> 2) );                           
        memset( puhBaseLCU + uiCUAddr + (uiCurrPartNumQ << 1) + (uiCurrPartNumQ >> 1), uiParameter, (uiCurrPartNumQ >> 2) );                          
      }
      else
      {
        assert(0);
      }
      break;
    default:
      assert( 0 );
  }
}

Void TComDataCU::setMergeFlagSubParts ( Bool bMergeFlag, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart( bMergeFlag, m_pbMergeFlag, uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setMergeIndexSubParts ( UInt uiMergeIndex, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<UChar>( uiMergeIndex, m_puhMergeIndex, uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setChromIntraDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhChromaIntraDir + uiAbsPartIdx, uiDir, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setInterDirSubParts( UInt uiDir, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<UChar>( uiDir, m_puhInterDir, uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setMVPIdxSubParts( Int iMVPIdx, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<Char>( iMVPIdx, m_apiMVPIdx[eRefPicList], uiAbsPartIdx, uiDepth, uiPartIdx );
}

Void TComDataCU::setMVPNumSubParts( Int iMVPNum, RefPicList eRefPicList, UInt uiAbsPartIdx, UInt uiPartIdx, UInt uiDepth )
{
  setSubPart<Char>( iMVPNum, m_apiMVPNum[eRefPicList], uiAbsPartIdx, uiDepth, uiPartIdx );
}


Void TComDataCU::setTrIdxSubParts( UInt uiTrIdx, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhTrIdx + uiAbsPartIdx, uiTrIdx, sizeof(UChar)*uiCurrPartNumb );
}

Void TComDataCU::setSizeSubParts( UInt uiWidth, UInt uiHeight, UInt uiAbsPartIdx, UInt uiDepth )
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);
  
  memset( m_puhWidth  + uiAbsPartIdx, uiWidth,  sizeof(UChar)*uiCurrPartNumb );
  memset( m_puhHeight + uiAbsPartIdx, uiHeight, sizeof(UChar)*uiCurrPartNumb );
}

UChar TComDataCU::getNumPartInter()
{
  UChar iNumPart = 0;
  
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:    iNumPart = 1; break;
    case SIZE_2NxN:     iNumPart = 2; break;
    case SIZE_Nx2N:     iNumPart = 2; break;
    case SIZE_NxN:      iNumPart = 4; break;
    case SIZE_2NxnU:    iNumPart = 2; break;
    case SIZE_2NxnD:    iNumPart = 2; break;
    case SIZE_nLx2N:    iNumPart = 2; break;
    case SIZE_nRx2N:    iNumPart = 2; break;
    default:            assert (0);   break;
  }
  
  return  iNumPart;
}

Void TComDataCU::getPartIndexAndSize( UInt uiPartIdx, UInt& ruiPartAddr, Int& riWidth, Int& riHeight )
{
  switch ( m_pePartSize[0] )
  {
    case SIZE_2NxN:
      riWidth = getWidth(0);      riHeight = getHeight(0) >> 1; ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      riWidth = getWidth(0) >> 1; riHeight = getHeight(0);      ruiPartAddr = ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2;
      break;
    case SIZE_NxN:
      riWidth = getWidth(0) >> 1; riHeight = getHeight(0) >> 1; ruiPartAddr = ( m_uiNumPartition >> 2 ) * uiPartIdx;
      break;
    case SIZE_2NxnU:
      riWidth     = getWidth(0);
      riHeight    = ( uiPartIdx == 0 ) ?  getHeight(0) >> 2 : ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 );
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : m_uiNumPartition >> 3;
      break;
    case SIZE_2NxnD:
      riWidth     = getWidth(0);
      riHeight    = ( uiPartIdx == 0 ) ?  ( getHeight(0) >> 2 ) + ( getHeight(0) >> 1 ) : getHeight(0) >> 2;
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 3);
      break;
    case SIZE_nLx2N:
      riWidth     = ( uiPartIdx == 0 ) ? getWidth(0) >> 2 : ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 );
      riHeight    = getHeight(0);
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : m_uiNumPartition >> 4;
      break;
    case SIZE_nRx2N:
      riWidth     = ( uiPartIdx == 0 ) ? ( getWidth(0) >> 2 ) + ( getWidth(0) >> 1 ) : getWidth(0) >> 2;
      riHeight    = getHeight(0);
      ruiPartAddr = ( uiPartIdx == 0 ) ? 0 : (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
      break;
    default:
      assert ( m_pePartSize[0] == SIZE_2Nx2N );
      riWidth = getWidth(0);      riHeight = getHeight(0);      ruiPartAddr = 0;
      break;
  }
}


Void TComDataCU::getMvField ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefPicList, TComMvField& rcMvField )
{
  if ( pcCU == NULL )  // OUT OF BOUNDARY
  {
    TComMv  cZeroMv;
    rcMvField.setMvField( cZeroMv, NOT_VALID );
    return;
  }
  
  TComCUMvField*  pcCUMvField = pcCU->getCUMvField( eRefPicList );
  rcMvField.setMvField( pcCUMvField->getMv( uiAbsPartIdx ), pcCUMvField->getRefIdx( uiAbsPartIdx ) );
}

Void TComDataCU::deriveLeftRightTopIdxGeneral ( PartSize eCUMode, UInt uiAbsPartIdx, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT )
{
  ruiPartIdxLT = m_uiAbsIdxInLCU + uiAbsPartIdx;
  UInt uiPUWidth = 0;
  
  switch ( m_pePartSize[uiAbsPartIdx] )
  {
    case SIZE_2Nx2N: uiPUWidth = m_puhWidth[uiAbsPartIdx];  break;
    case SIZE_2NxN:  uiPUWidth = m_puhWidth[uiAbsPartIdx];   break;
    case SIZE_Nx2N:  uiPUWidth = m_puhWidth[uiAbsPartIdx]  >> 1;  break;
    case SIZE_NxN:   uiPUWidth = m_puhWidth[uiAbsPartIdx]  >> 1; break;
    case SIZE_2NxnU:   uiPUWidth = m_puhWidth[uiAbsPartIdx]; break;
    case SIZE_2NxnD:   uiPUWidth = m_puhWidth[uiAbsPartIdx]; break;
    case SIZE_nLx2N:   
      if ( uiPartIdx == 0 )
      {
        uiPUWidth = m_puhWidth[uiAbsPartIdx]  >> 2; 
      }
      else if ( uiPartIdx == 1 )
      {
        uiPUWidth = (m_puhWidth[uiAbsPartIdx]  >> 1) + (m_puhWidth[uiAbsPartIdx]  >> 2); 
      }
      else
      {
        assert(0);
      }
      break;
    case SIZE_nRx2N:   
      if ( uiPartIdx == 0 )
      {
        uiPUWidth = (m_puhWidth[uiAbsPartIdx]  >> 1) + (m_puhWidth[uiAbsPartIdx]  >> 2); 
      }
      else if ( uiPartIdx == 1 )
      {
        uiPUWidth = m_puhWidth[uiAbsPartIdx]  >> 2; 
      }
      else
      {
        assert(0);
      }
      break;
    default:
      assert (0);
      break;
  }
  
  ruiPartIdxRT = g_auiRasterToZscan [g_auiZscanToRaster[ ruiPartIdxLT ] + uiPUWidth / m_pcPic->getMinCUWidth() - 1 ];
}

Void TComDataCU::deriveLeftBottomIdxGeneral( PartSize eCUMode, UInt uiAbsPartIdx, UInt uiPartIdx, UInt& ruiPartIdxLB )
{
  UInt uiPUHeight = 0;
  switch ( m_pePartSize[uiAbsPartIdx] )
  {
    case SIZE_2Nx2N: uiPUHeight = m_puhHeight[uiAbsPartIdx];    break;
    case SIZE_2NxN:  uiPUHeight = m_puhHeight[uiAbsPartIdx] >> 1;    break;
    case SIZE_Nx2N:  uiPUHeight = m_puhHeight[uiAbsPartIdx];  break;
    case SIZE_NxN:   uiPUHeight = m_puhHeight[uiAbsPartIdx] >> 1;    break;
    case SIZE_2NxnU: 
      if ( uiPartIdx == 0 )
      {
        uiPUHeight = m_puhHeight[uiAbsPartIdx] >> 2;    
      }
      else if ( uiPartIdx == 1 )
      {
        uiPUHeight = (m_puhHeight[uiAbsPartIdx] >> 1) + (m_puhHeight[uiAbsPartIdx] >> 2);    
      }
      else
      {
        assert(0);
      }
      break;
    case SIZE_2NxnD: 
      if ( uiPartIdx == 0 )
      {
        uiPUHeight = (m_puhHeight[uiAbsPartIdx] >> 1) + (m_puhHeight[uiAbsPartIdx] >> 2);    
      }
      else if ( uiPartIdx == 1 )
      {
        uiPUHeight = m_puhHeight[uiAbsPartIdx] >> 2;    
      }
      else
      {
        assert(0);
      }
      break;
    case SIZE_nLx2N: uiPUHeight = m_puhHeight[uiAbsPartIdx];  break;
    case SIZE_nRx2N: uiPUHeight = m_puhHeight[uiAbsPartIdx];  break;
    default:
      assert (0);
      break;
  }
  
  ruiPartIdxLB      = g_auiRasterToZscan [g_auiZscanToRaster[ m_uiAbsIdxInLCU + uiAbsPartIdx ] + ((uiPUHeight / m_pcPic->getMinCUHeight()) - 1)*m_pcPic->getNumPartInWidth()];
}

Void TComDataCU::deriveLeftRightTopIdx ( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxLT, UInt& ruiPartIdxRT )
{
  ruiPartIdxLT = m_uiAbsIdxInLCU;
  ruiPartIdxRT = g_auiRasterToZscan [g_auiZscanToRaster[ ruiPartIdxLT ] + m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1 ];
  
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:                                                                                                                                break;
    case SIZE_2NxN:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1; ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 2; ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : m_uiNumPartition >> 2;
      break;
    case SIZE_NxN:
      ruiPartIdxLT += ( m_uiNumPartition >> 2 ) * uiPartIdx;         ruiPartIdxRT +=  ( m_uiNumPartition >> 2 ) * ( uiPartIdx - 1 );
      break;
    case SIZE_2NxnU:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
      ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 3;
      break;
    case SIZE_2NxnD:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
      ruiPartIdxRT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 1 ) + ( m_uiNumPartition >> 3 );
      break;
    case SIZE_nLx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 4;
      ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
      break;
    case SIZE_nRx2N:
      ruiPartIdxLT += ( uiPartIdx == 0 )? 0 : ( m_uiNumPartition >> 2 ) + ( m_uiNumPartition >> 4 );
      ruiPartIdxRT -= ( uiPartIdx == 1 )? 0 : m_uiNumPartition >> 4;
      break;
    default:
      assert (0);
      break;
  }
  
}

Void TComDataCU::deriveLeftBottomIdx( PartSize      eCUMode,   UInt  uiPartIdx,      UInt&      ruiPartIdxLB )
{
  ruiPartIdxLB      = g_auiRasterToZscan [g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ( ((m_puhHeight[0] / m_pcPic->getMinCUHeight())>>1) - 1)*m_pcPic->getNumPartInWidth()];
  
  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:
      ruiPartIdxLB += m_uiNumPartition >> 1;
      break;
    case SIZE_2NxN:
      ruiPartIdxLB += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;
      break;
    case SIZE_Nx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 )? m_uiNumPartition >> 1 : (m_uiNumPartition >> 2)*3;
      break;
    case SIZE_NxN:
      ruiPartIdxLB += ( m_uiNumPartition >> 2 ) * uiPartIdx;
      break;
    case SIZE_2NxnU:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? -((Int)m_uiNumPartition >> 3) : m_uiNumPartition >> 1;
      break;
    case SIZE_2NxnD:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? (m_uiNumPartition >> 2) + (m_uiNumPartition >> 3): m_uiNumPartition >> 1;
      break;
    case SIZE_nLx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 4);
      break;
    case SIZE_nRx2N:
      ruiPartIdxLB += ( uiPartIdx == 0 ) ? m_uiNumPartition >> 1 : (m_uiNumPartition >> 1) + (m_uiNumPartition >> 2) + (m_uiNumPartition >> 4);
      break;
    default:
      assert (0);
      break;
  }
}

/** Derives the partition index of neighbouring bottom right block
 * \param [in]  eCUMode
 * \param [in]  uiPartIdx 
 * \param [out] ruiPartIdxRB 
 */
Void TComDataCU::deriveRightBottomIdx( PartSize      eCUMode,   UInt  uiPartIdx,      UInt&      ruiPartIdxRB )
{
  ruiPartIdxRB      = g_auiRasterToZscan [g_auiZscanToRaster[ m_uiAbsIdxInLCU ] + ( ((m_puhHeight[0] / m_pcPic->getMinCUHeight())>>1) - 1)*m_pcPic->getNumPartInWidth() +  m_puhWidth[0] / m_pcPic->getMinCUWidth() - 1];

  switch ( m_pePartSize[0] )
  {
    case SIZE_2Nx2N:  
      ruiPartIdxRB += m_uiNumPartition >> 1;    
      break;
    case SIZE_2NxN:  
      ruiPartIdxRB += ( uiPartIdx == 0 )? 0 : m_uiNumPartition >> 1;   
      break;
    case SIZE_Nx2N:  
      ruiPartIdxRB += ( uiPartIdx == 0 )? m_uiNumPartition >> 2 : (m_uiNumPartition >> 1);   
      break;
    case SIZE_NxN:   
      ruiPartIdxRB += ( m_uiNumPartition >> 2 ) * ( uiPartIdx - 1 );   
      break;
    case SIZE_2NxnU:
      ruiPartIdxRB += ( uiPartIdx == 0 ) ? -((Int)m_uiNumPartition >> 3) : m_uiNumPartition >> 1;
      break;
    case SIZE_2NxnD:
      ruiPartIdxRB += ( uiPartIdx == 0 ) ? (m_uiNumPartition >> 2) + (m_uiNumPartition >> 3): m_uiNumPartition >> 1;
      break;
    case SIZE_nLx2N:
      ruiPartIdxRB += ( uiPartIdx == 0 ) ? (m_uiNumPartition >> 3) + (m_uiNumPartition >> 4): m_uiNumPartition >> 1;
      break;
    case SIZE_nRx2N:
      ruiPartIdxRB += ( uiPartIdx == 0 ) ? (m_uiNumPartition >> 2) + (m_uiNumPartition >> 3) + (m_uiNumPartition >> 4) : m_uiNumPartition >> 1;
      break;
    default:
      assert (0);
      break;
  }
}

Void TComDataCU::deriveLeftRightTopIdxAdi ( UInt& ruiPartIdxLT, UInt& ruiPartIdxRT, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiNumPartInWidth = (m_puhWidth[0]/m_pcPic->getMinCUWidth())>>uiPartDepth;
  ruiPartIdxLT = m_uiAbsIdxInLCU + uiPartOffset;
  ruiPartIdxRT = g_auiRasterToZscan[ g_auiZscanToRaster[ ruiPartIdxLT ] + uiNumPartInWidth - 1 ];
}

Void TComDataCU::deriveLeftBottomIdxAdi( UInt& ruiPartIdxLB, UInt uiPartOffset, UInt uiPartDepth )
{
  UInt uiAbsIdx;
  UInt uiMinCuWidth, uiWidthInMinCus;
  
  uiMinCuWidth    = getPic()->getMinCUWidth();
  uiWidthInMinCus = (getWidth(0)/uiMinCuWidth)>>uiPartDepth;
  uiAbsIdx        = getZorderIdxInCU()+uiPartOffset+(m_uiNumPartition>>(uiPartDepth<<1))-1;
  uiAbsIdx        = g_auiZscanToRaster[uiAbsIdx]-(uiWidthInMinCus-1);
  ruiPartIdxLB    = g_auiRasterToZscan[uiAbsIdx];
}

Bool TComDataCU::hasEqualMotion( UInt uiAbsPartIdx, TComDataCU* pcCandCU, UInt uiCandAbsPartIdx )
{
  assert( getInterDir( uiAbsPartIdx ) != 0 );

  if ( getInterDir( uiAbsPartIdx ) != pcCandCU->getInterDir( uiCandAbsPartIdx ) )
  {
    return false;
  }

  for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
  {
    if ( getInterDir( uiAbsPartIdx ) & ( 1 << uiRefListIdx ) )
    {
      if ( getCUMvField( RefPicList( uiRefListIdx ) )->getMv( uiAbsPartIdx )     != pcCandCU->getCUMvField( RefPicList( uiRefListIdx ) )->getMv( uiCandAbsPartIdx ) || 
        getCUMvField( RefPicList( uiRefListIdx ) )->getRefIdx( uiAbsPartIdx ) != pcCandCU->getCUMvField( RefPicList( uiRefListIdx ) )->getRefIdx( uiCandAbsPartIdx ) )
      {
        return false;
      }
    }
  }

  return true;
}

/** Constructs a list of merging candidates
 * \param uiAbsPartIdx
 * \param uiPUIdx 
 * \param uiDepth
 * \param pcMvFieldNeighbours
 * \param puhInterDirNeighbours
 * \param numValidMergeCand
 */
Void TComDataCU::getInterMergeCandidates( UInt uiAbsPartIdx, UInt uiPUIdx, UInt uiDepth, TComMvField* pcMvFieldNeighbours, UChar* puhInterDirNeighbours, Int& numValidMergeCand )
{
  UInt uiAbsPartAddr = m_uiAbsIdxInLCU + uiAbsPartIdx;
  UInt uiIdx = 1;
  bool abCandIsInter[ MRG_MAX_NUM_CANDS ];
  for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ++ui )
  {
    abCandIsInter[ui] = false;
  }
  Int iCount = 0;

  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;
  PartSize cCurPS = getPartitionSize( uiAbsPartIdx );
  deriveLeftRightTopIdxGeneral( cCurPS, uiAbsPartIdx, uiPUIdx, uiPartIdxLT, uiPartIdxRT );
  deriveLeftBottomIdxGeneral( cCurPS, uiAbsPartIdx, uiPUIdx, uiPartIdxLB );

  //left
  UInt uiLeftPartIdx = 0;
  TComDataCU* pcCULeft = 0;
  pcCULeft = getPULeft( uiLeftPartIdx, uiPartIdxLB );
  PartSize partSize = getPartitionSize( uiAbsPartIdx );
  if (!(uiPUIdx == 1 && (partSize == SIZE_Nx2N || partSize == SIZE_nLx2N || partSize == SIZE_nRx2N)))
  {
  if ( pcCULeft && !pcCULeft->isIntra( uiLeftPartIdx ) )
  {
    abCandIsInter[iCount] = true;
    // get Inter Dir
    puhInterDirNeighbours[iCount] = pcCULeft->getInterDir( uiLeftPartIdx );
    // get Mv from Left
    pcCULeft->getMvField( pcCULeft, uiLeftPartIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[iCount<<1] );
    if ( getSlice()->isInterB() )
    {
      pcCULeft->getMvField( pcCULeft, uiLeftPartIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[(iCount<<1)+1] );
    }
    iCount ++;
  }
  }

  // above
  partSize = getPartitionSize( uiAbsPartIdx );
  if (!(uiPUIdx == 1 && (partSize == SIZE_2NxN || partSize == SIZE_2NxnU || partSize == SIZE_2NxnD)))
  {
  UInt uiAbovePartIdx = 0;
  TComDataCU* pcCUAbove = 0;
  pcCUAbove = getPUAbove( uiAbovePartIdx, uiPartIdxRT, true, true, true );
  if ( pcCUAbove && !pcCUAbove->isIntra( uiAbovePartIdx ) )
  {
    abCandIsInter[iCount] = true;
    // get Inter Dir
    puhInterDirNeighbours[iCount] = pcCUAbove->getInterDir( uiAbovePartIdx );
    // get Mv from Left
    pcCUAbove->getMvField( pcCUAbove, uiAbovePartIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[iCount<<1] );
    if ( getSlice()->isInterB() )
    {
      pcCUAbove->getMvField( pcCUAbove, uiAbovePartIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[(iCount<<1)+1] );
    }
    iCount ++;
  }
  }

  // above right
  UInt uiAboveRightPartIdx = 0;
  TComDataCU* pcCUAboveRight = 0;
  pcCUAboveRight = getPUAboveRight( uiAboveRightPartIdx, uiPartIdxRT, true, true, true);
  if ( pcCUAboveRight && !pcCUAboveRight->isIntra( uiAboveRightPartIdx ) )
  {
    abCandIsInter[iCount] = true;
    // get Inter Dir
    puhInterDirNeighbours[iCount] = pcCUAboveRight->getInterDir( uiAboveRightPartIdx );
    // get Mv from Left
    pcCUAboveRight->getMvField( pcCUAboveRight, uiAboveRightPartIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[iCount<<1] );
    if ( getSlice()->isInterB() )
    {
      pcCUAboveRight->getMvField( pcCUAboveRight, uiAboveRightPartIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[(iCount<<1)+1] );
    }
    iCount ++;
  }

  //left bottom
  UInt uiLeftBottomPartIdx = 0;
  TComDataCU* pcCULeftBottom = 0;
  pcCULeftBottom = this->getPUBelowLeft( uiLeftBottomPartIdx, uiPartIdxLB );
  if ( pcCULeftBottom && !pcCULeftBottom->isIntra( uiLeftBottomPartIdx ) )
  {
    abCandIsInter[iCount] = true;
    // get Inter Dir
    puhInterDirNeighbours[iCount] = pcCULeftBottom->getInterDir( uiLeftBottomPartIdx );
    // get Mv from Left
    pcCULeftBottom->getMvField( pcCULeftBottom, uiLeftBottomPartIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[iCount<<1] );
    if ( getSlice()->isInterB() )
    {
      pcCULeftBottom->getMvField( pcCULeftBottom, uiLeftBottomPartIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[(iCount<<1)+1] );
    }
    iCount ++;
  }

  // above left 
  if( iCount < 4 )
  {
    UInt uiAboveLeftPartIdx = 0;
    TComDataCU* pcCUAboveLeft = 0;
    pcCUAboveLeft = getPUAboveLeft( uiAboveLeftPartIdx, uiAbsPartAddr, true, true, true );
    if( pcCUAboveLeft && !pcCUAboveLeft->isIntra( uiAboveLeftPartIdx ) )
    {
      abCandIsInter[iCount] = true;
      // get Inter Dir
      puhInterDirNeighbours[iCount] = pcCUAboveLeft->getInterDir( uiAboveLeftPartIdx );
      // get Mv from Left
      pcCUAboveLeft->getMvField( pcCUAboveLeft, uiAboveLeftPartIdx, REF_PIC_LIST_0, pcMvFieldNeighbours[iCount<<1] );
      if ( getSlice()->isInterB() )
      {
        pcCUAboveLeft->getMvField( pcCUAboveLeft, uiAboveLeftPartIdx, REF_PIC_LIST_1, pcMvFieldNeighbours[(iCount<<1)+1] );
      }
      iCount ++;
    }
  }

  if ( getSlice()->getPPS()->getEnableTMVPFlag() )
  {
    // col [2]
    Int iRefIdxSkip[2] = {-1, -1};
    for (Int i=0; i<2; i++)
    {
      RefPicList  eRefPicList = ( i==1 ? REF_PIC_LIST_1 : REF_PIC_LIST_0 );
      Int iRefIdxTmp = (pcCULeft != NULL) ? pcCULeft->getCUMvField(eRefPicList)->getRefIdx(uiLeftPartIdx) : -1;
      iRefIdxSkip[i] = (iRefIdxTmp != -1) ? iRefIdxTmp : 0;
    }
    //>> MTK colocated-RightBottom
    UInt uiPartIdxRB;
    Int uiLCUIdx = getAddr();
    PartSize eCUMode = getPartitionSize( 0 );

    deriveRightBottomIdx( eCUMode, uiPUIdx, uiPartIdxRB );  

    UInt uiAbsPartIdxTmp = g_auiZscanToRaster[uiPartIdxRB];
    UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();

    TComMv cColMv;
    Int iRefIdx;

    if      ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdxTmp] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )  // image boundary check
    {
      uiLCUIdx = -1;
    }
    else if ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdxTmp] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
    {
      uiLCUIdx = -1;
    }
    else
    {
      if ( ( uiAbsPartIdxTmp % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 ) &&           // is not at the last column of LCU 
        ( uiAbsPartIdxTmp / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) ) // is not at the last row    of LCU
      {
        uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdxTmp + uiNumPartInCUWidth + 1 ];
        uiLCUIdx = getAddr();
      }
      else if ( uiAbsPartIdxTmp % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )           // is not at the last column of LCU But is last row of LCU
      {
        uiAbsPartAddr = g_auiRasterToZscan[ (uiAbsPartIdxTmp + uiNumPartInCUWidth + 1) % m_pcPic->getNumPartInCU() ];
        uiLCUIdx = -1 ; 
      }
      else if ( uiAbsPartIdxTmp / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) // is not at the last row of LCU But is last column of LCU
      {
        uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdxTmp + 1 ];
        uiLCUIdx = getAddr() + 1;
      }
      else //is the right bottom corner of LCU                       
      {
        uiAbsPartAddr = 0;
        uiLCUIdx = -1 ; 
      }
    }
    iRefIdx = iRefIdxSkip[0];

    Bool bExistMV = false;
    UInt uiPartIdxCenter;
    UInt uiCurLCUIdx = getAddr();
    xDeriveCenterIdx( eCUMode, uiPUIdx, uiPartIdxCenter );
    bExistMV = uiLCUIdx >= 0 && xGetColMVP( REF_PIC_LIST_0, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx );
    if( bExistMV == false )
    {
      bExistMV = xGetColMVP( REF_PIC_LIST_0, uiCurLCUIdx, uiPartIdxCenter,  cColMv, iRefIdx );
    }
    if( bExistMV )
    {
      UInt uiArrayAddr = iCount;
      abCandIsInter[uiArrayAddr] = true;
      pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( cColMv, iRefIdx );

      if ( getSlice()->isInterB() )
      {       
        iRefIdx = iRefIdxSkip[1];
        bExistMV = uiLCUIdx >= 0 && xGetColMVP( REF_PIC_LIST_1, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx);
        if( bExistMV == false )
        {
          bExistMV = xGetColMVP( REF_PIC_LIST_1, uiCurLCUIdx, uiPartIdxCenter,  cColMv, iRefIdx );
        }
        if( bExistMV )
        {
          pcMvFieldNeighbours[ ( uiArrayAddr << 1 ) + 1 ].setMvField( cColMv, iRefIdx );
          puhInterDirNeighbours[uiArrayAddr] = 3;
        }
        else
        {
          puhInterDirNeighbours[uiArrayAddr] = 1;
        }
      }
      else
      {
        puhInterDirNeighbours[uiArrayAddr] = 1;
      }
    }
    uiIdx++;

  }

  for( UInt uiOuter = 0; uiOuter < MRG_MAX_NUM_CANDS; uiOuter++ )
  {
    if( abCandIsInter[ uiOuter ] )
    {
      for( UInt uiIter = uiOuter + 1; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
      {
        UInt uiMvFieldNeighIdxCurr = uiOuter << 1;
        UInt uiMvFieldNeighIdxComp = uiIter << 1;
        if( getSlice()->isInterB() )
        {
          if( puhInterDirNeighbours[ uiOuter ] == 3 )
          {
            if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() && pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getRefIdx() &&
              pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() && pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getMv() &&
              puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] )
            {
              abCandIsInter[ uiIter ] = false;
            }
          }
          else if( puhInterDirNeighbours[ uiOuter ] == 2 )
          {
            if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getRefIdx() &&
              pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getMv() &&
              puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] )
            {
              abCandIsInter[ uiIter ] = false;
            }
          }
          else
          {
            if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() &&
              pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() &&
              puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] )
            {
              abCandIsInter[ uiIter ] = false;
            }
          }
        }
        else
        {
          if( pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() && 
              pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv() == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() )
          {
            assert( puhInterDirNeighbours[ uiOuter ] == puhInterDirNeighbours[ uiIter ] );
            abCandIsInter[ uiIter ] = false;
          }
        }
      }
    }
  }

  bool abCandIsInterFlag[MRG_MAX_NUM_CANDS];

  for (int i=0; i<MRG_MAX_NUM_CANDS; i++)
  {
    abCandIsInterFlag[i] = abCandIsInter[ i ];
    abCandIsInter[ i ] = false;
  }

  UInt uiArrayAddr = 0;
    
  for (int i=0; i<MRG_MAX_NUM_CANDS; i++)
  {
    if (abCandIsInterFlag[i])
    {
      abCandIsInter[uiArrayAddr] = true;
      puhInterDirNeighbours[uiArrayAddr] = puhInterDirNeighbours[i];
      pcMvFieldNeighbours[uiArrayAddr<<1].setMvField(pcMvFieldNeighbours[i<<1].getMv(), pcMvFieldNeighbours[i<<1].getRefIdx());
      pcMvFieldNeighbours[(uiArrayAddr<<1)+1].setMvField(pcMvFieldNeighbours[(i<<1)+1].getMv(), pcMvFieldNeighbours[(i<<1)+1].getRefIdx());
      ++uiArrayAddr;
    }
  }

  for (int i=uiArrayAddr; i<MRG_MAX_NUM_CANDS; i++)
  {
    puhInterDirNeighbours[i] = (getSlice()->isInterB()) ? 3 : 1;
    abCandIsInter[i] = false;
    TComMv tmpMv;
    tmpMv.set(0,0);
    pcMvFieldNeighbours[i<<1].setMvField(tmpMv, 0);
    if ( getSlice()->isInterB() )
    {
      pcMvFieldNeighbours[(i<<1)+1].setMvField(tmpMv, 0);
    }
  }

  UInt uiCutoff = uiArrayAddr;
    
  if ( getSlice()->isInterB() )
  {
    Int iCombinedCount = 0;
    UInt uiPriorityList0[12] = {0 , 1, 0, 2, 1, 2, 0, 3, 1, 3, 2, 3};
    UInt uiPriorityList1[12] = {1 , 0, 2, 0, 2, 1, 3, 0, 3, 1, 3, 2};

    for (Int idx=0; idx<uiCutoff*(uiCutoff-1) && uiArrayAddr!=MRG_MAX_NUM_CANDS && iCombinedCount<5; idx++)
    {
      Int i = uiPriorityList0[idx]; Int j = uiPriorityList1[idx];
      if (abCandIsInter[i] && abCandIsInter[j]&& (puhInterDirNeighbours[i]&0x1)&&(puhInterDirNeighbours[j]&0x2))
      {
        abCandIsInter[uiArrayAddr] = true;
        puhInterDirNeighbours[uiArrayAddr] = 3;

        // get Mv from cand[i] and cand[j]
        pcMvFieldNeighbours[uiArrayAddr << 1].setMvField(pcMvFieldNeighbours[i<<1].getMv(), pcMvFieldNeighbours[i<<1].getRefIdx());
        pcMvFieldNeighbours[( uiArrayAddr << 1 ) + 1].setMvField(pcMvFieldNeighbours[(j<<1)+1].getMv(), pcMvFieldNeighbours[(j<<1)+1].getRefIdx());

        Int iRefPOCL0 = m_pcSlice->getRefPOC( REF_PIC_LIST_0, pcMvFieldNeighbours[(uiArrayAddr<<1)].getRefIdx() );
        Int iRefPOCL1 = m_pcSlice->getRefPOC( REF_PIC_LIST_1, pcMvFieldNeighbours[(uiArrayAddr<<1)+1].getRefIdx() );
        if (iRefPOCL0 == iRefPOCL1 && pcMvFieldNeighbours[(uiArrayAddr<<1)].getMv() == pcMvFieldNeighbours[(uiArrayAddr<<1)+1].getMv())
        {
          abCandIsInter[uiArrayAddr] = false;
        }
        else
        {
          uiArrayAddr++;
          iCombinedCount++;
        }
      }
    }
  
    Int iScaledCount = 0;
    for (int i=0; i<uiCutoff && uiArrayAddr!=MRG_MAX_NUM_CANDS && iScaledCount<1; i++)
    {
      for (int j=0; j<2 && uiArrayAddr!=MRG_MAX_NUM_CANDS && iScaledCount<1; j++)
      {
        if (abCandIsInter[i] && pcMvFieldNeighbours[(i<<1)+j].getRefIdx()>=0 && pcMvFieldNeighbours[(i<<1)+j].getRefIdx() < m_pcSlice->getNumRefIdx((j==0)?REF_PIC_LIST_1:REF_PIC_LIST_0))
        {
          Int iCurrPOC = m_pcSlice->getPOC();
          Int iCurrRefPOC1 = m_pcSlice->getRefPOC( (j==0)?REF_PIC_LIST_1:REF_PIC_LIST_0, pcMvFieldNeighbours[(i<<1)+j].getRefIdx() );
          Int iCurrRefPOC2 = m_pcSlice->getRefPOC( (j==0)?REF_PIC_LIST_0:REF_PIC_LIST_1, pcMvFieldNeighbours[(i<<1)+j].getRefIdx() );

          if (iCurrRefPOC1 != iCurrRefPOC2 && abs(iCurrPOC-iCurrRefPOC1) == abs(iCurrPOC-iCurrRefPOC2))
          {
            abCandIsInter[uiArrayAddr] = true;
            puhInterDirNeighbours[uiArrayAddr] = 3;
            
            TComMv cMvPred = pcMvFieldNeighbours[(i<<1)+j].getMv();
            TComMv rcMv;

            rcMv.set(-1*cMvPred.getHor(), -1*cMvPred.getVer());

            if (j==0)
            {
              pcMvFieldNeighbours[uiArrayAddr << 1].setMvField(pcMvFieldNeighbours[(i<<1)].getMv(), pcMvFieldNeighbours[(i<<1)].getRefIdx());
              pcMvFieldNeighbours[( uiArrayAddr << 1 ) + 1].setMvField(rcMv, pcMvFieldNeighbours[(i<<1)].getRefIdx());
            }
            else
            {
              pcMvFieldNeighbours[uiArrayAddr << 1].setMvField(rcMv, pcMvFieldNeighbours[(i<<1)+1].getRefIdx());
              pcMvFieldNeighbours[( uiArrayAddr << 1 ) + 1].setMvField(pcMvFieldNeighbours[(i<<1)+1].getMv(), pcMvFieldNeighbours[(i<<1)+1].getRefIdx());
            }

            uiArrayAddr++;
            iScaledCount++;
          }
        }
      }
    }
  }

  Int iNumRefIdx = (getSlice()->isInterB()) ? min(m_pcSlice->getNumRefIdx(REF_PIC_LIST_0), m_pcSlice->getNumRefIdx(REF_PIC_LIST_1)) : m_pcSlice->getNumRefIdx(REF_PIC_LIST_0);
  for (int r=0; r<iNumRefIdx && uiArrayAddr!=MRG_MAX_NUM_CANDS; r++)
  {
    abCandIsInter[uiArrayAddr] = true;
    puhInterDirNeighbours[uiArrayAddr] = 1;
    pcMvFieldNeighbours[uiArrayAddr << 1].setMvField( TComMv(0, 0), r);

    if ( getSlice()->isInterB() )
    {
      puhInterDirNeighbours[uiArrayAddr] = 3;
      pcMvFieldNeighbours[(uiArrayAddr << 1) + 1].setMvField(TComMv(0, 0), r);
    }
    uiArrayAddr++;
  }
  if (uiArrayAddr > MRG_MAX_NUM_CANDS_SIGNALED)
  {
    uiArrayAddr = MRG_MAX_NUM_CANDS_SIGNALED;
  }
  numValidMergeCand = uiArrayAddr;
}

/** Check the duplicated candidate in the list
 * \param pcMvFieldNeighbours
 * \param puhInterDirNeighbours 
 * \param pbCandIsInter
 * \param ruiArrayAddr
 * \returns Void
 */
Void TComDataCU::xCheckDuplicateCand(TComMvField* pcMvFieldNeighbours, UChar* puhInterDirNeighbours, bool* pbCandIsInter, UInt& ruiArrayAddr)
{
  if (getSlice()->isInterB())
  {
    UInt uiMvFieldNeighIdxCurr = ruiArrayAddr << 1;
    Int iRefIdxL0 = pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx();
    Int iRefIdxL1 = pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getRefIdx();
    TComMv MvL0 = pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv();
    TComMv MvL1 = pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr + 1 ].getMv();

    for (int k=0; k<ruiArrayAddr; k++)
    {
      UInt uiMvFieldNeighIdxComp = k << 1;
      if (iRefIdxL0 == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() && 
          iRefIdxL1 == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getRefIdx() &&
          MvL0 == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() && 
          MvL1 == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp + 1 ].getMv() &&
          puhInterDirNeighbours[ ruiArrayAddr ] == puhInterDirNeighbours[ k ] )
      {
        pbCandIsInter[ruiArrayAddr] = false;
        break;
      }
    }
  }
  else
  {
    UInt uiMvFieldNeighIdxCurr = ruiArrayAddr << 1;
    Int iRefIdxL0 = pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getRefIdx();
    TComMv MvL0 = pcMvFieldNeighbours[ uiMvFieldNeighIdxCurr ].getMv();

    for (int k=0; k<ruiArrayAddr; k++)
    {
      UInt uiMvFieldNeighIdxComp = k << 1;
      if (iRefIdxL0 == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getRefIdx() && 
          MvL0 == pcMvFieldNeighbours[ uiMvFieldNeighIdxComp ].getMv() && 
          puhInterDirNeighbours[ ruiArrayAddr ] == puhInterDirNeighbours[ k ] )
      {
        pbCandIsInter[ruiArrayAddr] = false;
        break;
      }
    }
  }

  if (pbCandIsInter[ruiArrayAddr])
  {
    ++ruiArrayAddr;
  }
}

Void TComDataCU::xCheckCornerCand( TComDataCU* pcCorner, UInt uiCornerPUIdx, UInt uiIter, Bool& rbValidCand )
{
  if( uiIter == 0 )
  {
    if( pcCorner && !pcCorner->isIntra( uiCornerPUIdx ) )
    {
      rbValidCand = true;
      if( getSlice()->isInterB() )
      {
        if ( pcCorner->getInterDir( uiCornerPUIdx ) == 1 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) != 0 )
          {
            rbValidCand = false;
          }
        }
        else if ( pcCorner->getInterDir( uiCornerPUIdx ) == 2 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) != 0 )
          {
            rbValidCand = false;
          }
        }
        else
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) != 0 || pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) != 0 )
          {
            rbValidCand = false;
          }
        }
      }
      else if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) != 0 )
      {
        rbValidCand = false;
      }
    }
  }
  else
  {
    if( pcCorner && !pcCorner->isIntra( uiCornerPUIdx ) )
    {
      rbValidCand = true;
      if( getSlice()->isInterB() )
      {
        if ( pcCorner->getInterDir( uiCornerPUIdx ) == 1 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) < 0 )
          {
            rbValidCand = false;
          }
        }
        else if ( pcCorner->getInterDir( uiCornerPUIdx ) == 2 )
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) < 0 )
          {
            rbValidCand = false;
          }
        }
        else
        {
          if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) < 0 || pcCorner->getCUMvField(REF_PIC_LIST_1)->getRefIdx( uiCornerPUIdx ) < 0 )
          {
            rbValidCand = false;
          }
        }
      }
      else if( pcCorner->getCUMvField(REF_PIC_LIST_0)->getRefIdx( uiCornerPUIdx ) < 0 )
      {
        rbValidCand = false;
      }
    }
  }
}

AMVP_MODE TComDataCU::getAMVPMode(UInt uiIdx)
{
  return m_pcSlice->getSPS()->getAMVPMode(m_puhDepth[uiIdx]);
}

/** Constructs a list of candidates for AMVP
 * \param uiPartIdx
 * \param uiPartAddr 
 * \param eRefPicList
 * \param iRefIdx
 * \param pInfo
 */
Void TComDataCU::fillMvpCand ( UInt uiPartIdx, UInt uiPartAddr, RefPicList eRefPicList, Int iRefIdx, AMVPInfo* pInfo )
{
  PartSize eCUMode = getPartitionSize( 0 );
  
  TComMv cMvPred;
  Bool bAddedSmvp = false;

  pInfo->iN = 0;  
  if (iRefIdx < 0)
  {
    return;
  }
  
  //-- Get Spatial MV
  UInt uiPartIdxLT, uiPartIdxRT, uiPartIdxLB;
  UInt uiNumPartInCUWidth = m_pcPic->getNumPartInWidth();
  Bool bAdded = false;
  
  deriveLeftRightTopIdx( eCUMode, uiPartIdx, uiPartIdxLT, uiPartIdxRT );
  deriveLeftBottomIdx( eCUMode, uiPartIdx, uiPartIdxLB );
  
  TComDataCU* tmpCU = NULL;
  UInt idx;
  tmpCU = getPUBelowLeft(idx, uiPartIdxLB);
  bAddedSmvp = (tmpCU != NULL) && (tmpCU->getPredictionMode(idx) != MODE_INTRA);

  if (!bAddedSmvp)
  {
    tmpCU = getPULeft(idx, uiPartIdxLB);
    bAddedSmvp = (tmpCU != NULL) && (tmpCU->getPredictionMode(idx) != MODE_INTRA);
  }

  // Left predictor search
  bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_BELOW_LEFT);
  if (!bAdded) 
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_LEFT );
  }
  
  if(!bAdded)
  {
    bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_BELOW_LEFT);
    if (!bAdded) 
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxLB, MD_LEFT );
    }
  }
  // Above predictor search
  bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE_RIGHT);

  if (!bAdded) 
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE);
  }

  if(!bAdded)
  {
    bAdded = xAddMVPCand( pInfo, eRefPicList, iRefIdx, uiPartIdxLT, MD_ABOVE_LEFT);
  }
  bAdded = bAddedSmvp;
  if (pInfo->iN==2) bAdded = true;

  if(!bAdded)
  {
    bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE_RIGHT);
    if (!bAdded) 
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxRT, MD_ABOVE);
    }

    if(!bAdded)
    {
      bAdded = xAddMVPCandOrder( pInfo, eRefPicList, iRefIdx, uiPartIdxLT, MD_ABOVE_LEFT);
    }
  }
  
  if (getAMVPMode(uiPartAddr) == AM_NONE)  //Should be optimized later for special cases
  {
    assert(pInfo->iN > 0);
    pInfo->iN = 1;
    return;
  }
  
  if ( getSlice()->getPPS()->getEnableTMVPFlag() )
  {
    // Get Temporal Motion Predictor
    int iRefIdx_Col = iRefIdx;
    TComMv cColMv;
    UInt uiPartIdxRB;
    UInt uiAbsPartIdx;  
    UInt uiAbsPartAddr;
    int uiLCUIdx = getAddr();

    deriveRightBottomIdx( eCUMode, uiPartIdx, uiPartIdxRB );
    uiAbsPartAddr = m_uiAbsIdxInLCU + uiPartAddr;

    //----  co-located RightBottom Temporal Predictor (H) ---//
    uiAbsPartIdx = g_auiZscanToRaster[uiPartIdxRB];
    if ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelX() + g_auiRasterToPelX[uiAbsPartIdx] + m_pcPic->getMinCUWidth() ) >= m_pcSlice->getSPS()->getWidth() )  // image boundary check
    {
      uiLCUIdx = -1;
    }
    else if ( ( m_pcPic->getCU(m_uiCUAddr)->getCUPelY() + g_auiRasterToPelY[uiAbsPartIdx] + m_pcPic->getMinCUHeight() ) >= m_pcSlice->getSPS()->getHeight() )
    {
      uiLCUIdx = -1;
    }
    else
    {
      if ( ( uiAbsPartIdx % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 ) &&           // is not at the last column of LCU 
        ( uiAbsPartIdx / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) ) // is not at the last row    of LCU
      {
        uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdx + uiNumPartInCUWidth + 1 ];
        uiLCUIdx = getAddr();
      }
      else if ( uiAbsPartIdx % uiNumPartInCUWidth < uiNumPartInCUWidth - 1 )           // is not at the last column of LCU But is last row of LCU
      {
        uiAbsPartAddr = g_auiRasterToZscan[ (uiAbsPartIdx + uiNumPartInCUWidth + 1) % m_pcPic->getNumPartInCU() ];
        uiLCUIdx      = -1 ; 
      }
      else if ( uiAbsPartIdx / uiNumPartInCUWidth < m_pcPic->getNumPartInHeight() - 1 ) // is not at the last row of LCU But is last column of LCU
      {
        uiAbsPartAddr = g_auiRasterToZscan[ uiAbsPartIdx + 1 ];
        uiLCUIdx = getAddr() + 1;
      }
      else //is the right bottom corner of LCU                       
      {
        uiAbsPartAddr = 0;
        uiLCUIdx      = -1 ; 
      }
    }
    if ( uiLCUIdx >= 0 && xGetColMVP( eRefPicList, uiLCUIdx, uiAbsPartAddr, cColMv, iRefIdx_Col ) )
    {
      pInfo->m_acMvCand[pInfo->iN++] = cColMv;
    }
    else 
    {
      UInt uiPartIdxCenter;
      UInt uiCurLCUIdx = getAddr();
      xDeriveCenterIdx( eCUMode, uiPartIdx, uiPartIdxCenter );
      if (xGetColMVP( eRefPicList, uiCurLCUIdx, uiPartIdxCenter,  cColMv, iRefIdx_Col ))
      {
        pInfo->m_acMvCand[pInfo->iN++] = cColMv;
      }
    }
    //----  co-located RightBottom Temporal Predictor  ---//
  }

  // Check No MV Candidate
  xUniqueMVPCand( pInfo );

  if (pInfo->iN > AMVP_MAX_NUM_CANDS)
  {
    pInfo->iN = AMVP_MAX_NUM_CANDS;
  }
  else if (pInfo->iN < AMVP_MAX_NUM_CANDS)
  {
    int j;
    for (j=0; j<pInfo->iN; j++)
    {
      if (pInfo->m_acMvCand[j].getHor() == 0 && pInfo->m_acMvCand[j].getVer() == 0)
      {
        break;
      }
    }
    if (j == pInfo->iN)
    {
      pInfo->m_acMvCand[pInfo->iN].set(0,0);
      pInfo->iN++;
    }
  }
  return ;
}

Void TComDataCU::clipMv    (TComMv&  rcMv)
{
  Int  iMvShift = 2;
  Int iOffset = 8;
  Int iHorMax = ( m_pcSlice->getSPS()->getWidth() + iOffset - m_uiCUPelX - 1 ) << iMvShift;
  Int iHorMin = (       -(Int)g_uiMaxCUWidth - iOffset - (Int)m_uiCUPelX + 1 ) << iMvShift;
  
  Int iVerMax = ( m_pcSlice->getSPS()->getHeight() + iOffset - m_uiCUPelY - 1 ) << iMvShift;
  Int iVerMin = (       -(Int)g_uiMaxCUHeight - iOffset - (Int)m_uiCUPelY + 1 ) << iMvShift;
  
  rcMv.setHor( min (iHorMax, max (iHorMin, rcMv.getHor())) );
  rcMv.setVer( min (iVerMax, max (iVerMin, rcMv.getVer())) );
}


Void TComDataCU::convertTransIdx( UInt uiAbsPartIdx, UInt uiTrIdx, UInt& ruiLumaTrMode, UInt& ruiChromaTrMode )
{
  ruiLumaTrMode   = uiTrIdx;
  ruiChromaTrMode = uiTrIdx;
  return;
}

UInt TComDataCU::getIntraSizeIdx(UInt uiAbsPartIdx)
{
  UInt uiShift = ( (m_puhTrIdx[uiAbsPartIdx]==0) && (m_pePartSize[uiAbsPartIdx]==SIZE_NxN) ) ? m_puhTrIdx[uiAbsPartIdx]+1 : m_puhTrIdx[uiAbsPartIdx];
  uiShift = ( m_pePartSize[uiAbsPartIdx]==SIZE_NxN ? 1 : 0 );
  
  UChar uiWidth = m_puhWidth[uiAbsPartIdx]>>uiShift;
  UInt  uiCnt = 0;
  while( uiWidth )
  {
    uiCnt++;
    uiWidth>>=1;
  }
  uiCnt-=2;
  return uiCnt > 6 ? 6 : uiCnt;
}

Void TComDataCU::clearCbf( UInt uiIdx, TextType eType, UInt uiNumParts )
{
  ::memset( &m_puhCbf[g_aucConvertTxtTypeToIdx[eType]][uiIdx], 0, sizeof(UChar)*uiNumParts);
}

/** Set a I_PCM flag for all sub-partitions of a partition.
 * \param bIpcmFlag I_PCM flag
 * \param uiAbsPartIdx patition index
 * \param uiDepth CU depth
 * \returns Void
 */
Void TComDataCU::setIPCMFlagSubParts  (Bool bIpcmFlag, UInt uiAbsPartIdx, UInt uiDepth)
{
  UInt uiCurrPartNumb = m_pcPic->getNumPartInCU() >> (uiDepth << 1);

  memset(m_pbIPCMFlag + uiAbsPartIdx, bIpcmFlag, sizeof(Bool)*uiCurrPartNumb );
}

/** Test whether the current block is skipped
 * \param uiPartIdx Block index
 * \returns Flag indicating whether the block is skipped
 */
Bool TComDataCU::isSkipped( UInt uiPartIdx )
{
  if ( m_pcSlice->isIntra () )
  {
    return false;
  }
  if ( getSlice()->getSPS()->getUseMRG() )
  {
    return ( m_pePredMode[ uiPartIdx ] == MODE_SKIP && getMergeFlag( uiPartIdx ) && !getQtRootCbf( uiPartIdx ) );
  }
  else
  {
    if ( m_pcSlice->isInterP() )
    {
      return ( ( m_pePredMode[ uiPartIdx ] == MODE_SKIP ) && ( ( m_puhCbf[0][uiPartIdx] & 0x1 ) + ( m_puhCbf[1][uiPartIdx] & 0x1 ) + ( m_puhCbf[2][uiPartIdx] & 0x1 ) == 0) );
    }
    else //if ( m_pcSlice->isInterB()  )
    {
      return ( ( m_pePredMode[ uiPartIdx ] == MODE_SKIP ) && ( ( m_puhCbf[0][uiPartIdx] & 0x1 ) + ( m_puhCbf[1][uiPartIdx] & 0x1 ) + ( m_puhCbf[2][uiPartIdx] & 0x1 ) == 0) && (m_puhInterDir[uiPartIdx] == 3) );
    }
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Bool TComDataCU::xAddMVPCand( AMVPInfo* pInfo, RefPicList eRefPicList, Int iRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir )
{
  TComDataCU* pcTmpCU = NULL;
  UInt uiIdx;
  switch( eDir )
  {
    case MD_LEFT:
    {
      pcTmpCU = getPULeft(uiIdx, uiPartUnitIdx);
      break;
    }
    case MD_ABOVE:
    {
      pcTmpCU = getPUAbove(uiIdx, uiPartUnitIdx, true, true, true);
      break;
    }
    case MD_ABOVE_RIGHT:
    {
      pcTmpCU = getPUAboveRight(uiIdx, uiPartUnitIdx, true, true, true);
      break;
    }
    case MD_BELOW_LEFT:
    {
      pcTmpCU = getPUBelowLeft(uiIdx, uiPartUnitIdx);
      break;
    }
    case MD_ABOVE_LEFT:
    {
      pcTmpCU = getPUAboveLeft(uiIdx, uiPartUnitIdx, true, true, true);
      break;
    }
    default:
    {
      break;
    }
  }
 
  if ( pcTmpCU != NULL && m_pcSlice->isEqualRef(eRefPicList, pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdx), iRefIdx) )
  {
    TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList)->getMv(uiIdx);
    
    pInfo->m_acMvCand[ pInfo->iN++] = cMvPred;
    return true;
  }

  if ( pcTmpCU == NULL ) 
  {
    return false;
  }
  
  RefPicList eRefPicList2nd = REF_PIC_LIST_0;
  if(       eRefPicList == REF_PIC_LIST_0 )
  {
    eRefPicList2nd = REF_PIC_LIST_1;
  }
  else if ( eRefPicList == REF_PIC_LIST_1)
  {
    eRefPicList2nd = REF_PIC_LIST_0;
  }


  Int iCurrRefPOC = m_pcSlice->getRefPic( eRefPicList, iRefIdx)->getPOC();
  Int iNeibRefPOC;


  if( pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) >= 0 )
  {
    iNeibRefPOC = pcTmpCU->getSlice()->getRefPOC( eRefPicList2nd, pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) );
    if( iNeibRefPOC == iCurrRefPOC ) // Same Reference Frame But Diff List//
    {
      TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList2nd)->getMv(uiIdx);
      pInfo->m_acMvCand[ pInfo->iN++] = cMvPred;
      return true;
    }
  }
  return false;
}

/**
 * Reduce list of motion vector predictors to a set of unique predictors
 * \param pInfo list of motion vector predictors
 */
Void TComDataCU::xUniqueMVPCand(AMVPInfo* pInfo)
{
  Int n = 1;
  if ( pInfo->iN == 0 )
  {
    // Add a zero candidate is none is available
    pInfo->m_acMvCand[ 0 ].setZero();
  }
  else
  {
    for (Int i = 1; i < pInfo->iN; i++)
    {
      Int j;
      for (j = n - 1; j >= 0; j--)
      {
        if ( pInfo->m_acMvCand[ i ] == pInfo->m_acMvCand[ j ] )
        {
          break;
        }
      }
      if ( j < 0 )
      {
        pInfo->m_acMvCand[ n++ ] = pInfo->m_acMvCand[ i ];
      }
    }
  }
  pInfo->iN = n;
}

/** 
 * \param pInfo
 * \param eRefPicList 
 * \param iRefIdx
 * \param uiPartUnitIdx
 * \param eDir
 * \returns Bool
 */
Bool TComDataCU::xAddMVPCandOrder( AMVPInfo* pInfo, RefPicList eRefPicList, Int iRefIdx, UInt uiPartUnitIdx, MVP_DIR eDir )
{
  TComDataCU* pcTmpCU = NULL;
  UInt uiIdx;
  switch( eDir )
  {
  case MD_LEFT:
    {
      pcTmpCU = getPULeft(uiIdx, uiPartUnitIdx);
      break;
    }
  case MD_ABOVE:
    {
      pcTmpCU = getPUAbove(uiIdx, uiPartUnitIdx, true, true, true);
      break;
    }
  case MD_ABOVE_RIGHT:
    {
      pcTmpCU = getPUAboveRight(uiIdx, uiPartUnitIdx, true, true, true);
      break;
    }
  case MD_BELOW_LEFT:
    {
      pcTmpCU = getPUBelowLeft(uiIdx, uiPartUnitIdx);
      break;
    }
  case MD_ABOVE_LEFT:
    {
      pcTmpCU = getPUAboveLeft(uiIdx, uiPartUnitIdx, true, true, true);
      break;
    }
  default:
    {
      break;
    }
  }

  if ( pcTmpCU == NULL ) 
  {
    return false;
  }
  
  RefPicList eRefPicList2nd = REF_PIC_LIST_0;
  if(       eRefPicList == REF_PIC_LIST_0 )
  {
    eRefPicList2nd = REF_PIC_LIST_1;
  }
  else if ( eRefPicList == REF_PIC_LIST_1)
  {
    eRefPicList2nd = REF_PIC_LIST_0;
  }

  Int iCurrPOC = m_pcSlice->getPOC();
  Int iCurrRefPOC = m_pcSlice->getRefPic( eRefPicList, iRefIdx)->getPOC();
  Int iNeibPOC = iCurrPOC;
  Int iNeibRefPOC;

  //---------------  V1 (END) ------------------//
  if( pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdx) >= 0)
  {
    iNeibRefPOC = pcTmpCU->getSlice()->getRefPOC( eRefPicList, pcTmpCU->getCUMvField(eRefPicList)->getRefIdx(uiIdx) );
    TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList)->getMv(uiIdx);
    TComMv rcMv;

    Int iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iNeibPOC, iNeibRefPOC );
    if ( iScale == 4096 )
    {
      rcMv = cMvPred;
    }
    else
    {
      rcMv = cMvPred.scaleMv( iScale );
    }
    pInfo->m_acMvCand[ pInfo->iN++] = rcMv;
    return true;
  }
  //---------------------- V2(END) --------------------//
  if( pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) >= 0)
  {
    iNeibRefPOC = pcTmpCU->getSlice()->getRefPOC( eRefPicList2nd, pcTmpCU->getCUMvField(eRefPicList2nd)->getRefIdx(uiIdx) );
    TComMv cMvPred = pcTmpCU->getCUMvField(eRefPicList2nd)->getMv(uiIdx);
    TComMv rcMv;

    Int iScale = xGetDistScaleFactor( iCurrPOC, iCurrRefPOC, iNeibPOC, iNeibRefPOC );
    if ( iScale == 4096 )
    {
      rcMv = cMvPred;
    }
    else
    {
      rcMv = cMvPred.scaleMv( iScale );
    }
    pInfo->m_acMvCand[ pInfo->iN++] = rcMv;
    return true;
  }
  //---------------------- V3(END) --------------------//
  return false;
}

/** 
 * \param eRefPicList
 * \param uiCUAddr 
 * \param uiPartUnitIdx
 * \param riRefIdx
 * \returns Bool
 */
Bool TComDataCU::xGetColMVP( RefPicList eRefPicList, Int uiCUAddr, Int uiPartUnitIdx, TComMv& rcMv, Int& riRefIdx )
{
  UInt uiAbsPartAddr = uiPartUnitIdx;

  RefPicList  eColRefPicList;
  Int iColPOC, iColRefPOC, iCurrPOC, iCurrRefPOC, iScale;
  TComMv cColMv;

  iCurrPOC = m_pcSlice->getPOC();    
  iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, riRefIdx)->getPOC();

  // use coldir.
  TComPic *pColPic = getSlice()->getRefPic( RefPicList(getSlice()->isInterB() ? getSlice()->getColDir() : 0), 0);
  TComDataCU *pColCU = pColPic->getCU( uiCUAddr );
  iColPOC = pColCU->getSlice()->getPOC();  

  if (pColCU->isIntra(uiAbsPartAddr))
  {
    return false;
  }

  if ( !pColPic->getUsedForTMVP() )
  {
    return false;
  }

  eColRefPicList = getSlice()->getCheckLDC() ? eRefPicList : RefPicList(1-getSlice()->getColDir());

  Int iColRefIdx = pColCU->getCUMvField(RefPicList(eColRefPicList))->getRefIdx(uiAbsPartAddr);

  if (iColRefIdx < 0 )
  {
    eColRefPicList = RefPicList(1 - eColRefPicList);
    iColRefIdx = pColCU->getCUMvField(RefPicList(eColRefPicList))->getRefIdx(uiAbsPartAddr);

    if (iColRefIdx < 0 )
    {
      return false;
    }
  }

  // Scale the vector.
  iColRefPOC = pColCU->getSlice()->getRefPOC(eColRefPicList, iColRefIdx);
  cColMv = pColCU->getCUMvField(eColRefPicList)->getMv(uiAbsPartAddr);

  iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, riRefIdx)->getPOC();
  iScale = xGetDistScaleFactor(iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC);
  if ( iScale == 4096 )
  {
    rcMv = cColMv;
  }
  else
  {
    rcMv = cColMv.scaleMv( iScale );
  }

  return true;
}

UInt TComDataCU::xGetMvdBits(TComMv cMvd)
{
  return ( xGetComponentBits(cMvd.getHor()) + xGetComponentBits(cMvd.getVer()) );
}

UInt TComDataCU::xGetComponentBits(Int iVal)
{
  UInt uiLength = 1;
  UInt uiTemp   = ( iVal <= 0) ? (-iVal<<1)+1: (iVal<<1);
  
  assert ( uiTemp );
  
  while ( 1 != uiTemp )
  {
    uiTemp >>= 1;
    uiLength += 2;
  }
  
  return uiLength;
}


Int TComDataCU::xGetDistScaleFactor(Int iCurrPOC, Int iCurrRefPOC, Int iColPOC, Int iColRefPOC)
{
  Int iDiffPocD = iColPOC - iColRefPOC;
  Int iDiffPocB = iCurrPOC - iCurrRefPOC;
  
  if( iDiffPocD == iDiffPocB )
  {
    return 4096;
  }
  else
  {
    Int iTDB      = Clip3( -128, 127, iDiffPocB );
    Int iTDD      = Clip3( -128, 127, iDiffPocD );
    Int iX        = (0x4000 + abs(iTDD/2)) / iTDD;
    Int iScale    = Clip3( -4096, 4095, (iTDB * iX + 32) >> 6 );
    return iScale;
  }
}

/** 
 * \param eCUMode
 * \param uiPartIdx 
 * \param ruiPartIdxCenter
 * \returns Void
 */
Void TComDataCU::xDeriveCenterIdx( PartSize eCUMode, UInt uiPartIdx, UInt& ruiPartIdxCenter )
{
  UInt uiPartAddr;
  Int  iPartWidth;
  Int  iPartHeight;
  getPartIndexAndSize( uiPartIdx, uiPartAddr, iPartWidth, iPartHeight);
  
  ruiPartIdxCenter = m_uiAbsIdxInLCU+uiPartAddr; // partition origin.
  ruiPartIdxCenter = g_auiRasterToZscan[ g_auiZscanToRaster[ ruiPartIdxCenter ]
                                        + ( iPartHeight/m_pcPic->getMinCUHeight()  )/2*m_pcPic->getNumPartInWidth()
                                        + ( iPartWidth/m_pcPic->getMinCUWidth()  )/2];
}

/** 
 * \param uiPartIdx
 * \param eRefPicList 
 * \param iRefIdx
 * \param pcMv
 * \returns Bool
 */
Bool TComDataCU::xGetCenterCol( UInt uiPartIdx, RefPicList eRefPicList, int iRefIdx, TComMv *pcMv )
{
  PartSize eCUMode = getPartitionSize( 0 );
  
  Int iCurrPOC = m_pcSlice->getPOC();
  
  // use coldir.
  TComPic *pColPic = getSlice()->getRefPic( RefPicList(getSlice()->isInterB() ? getSlice()->getColDir() : 0), 0);
  TComDataCU *pColCU = pColPic->getCU( m_uiCUAddr );
  
  Int iColPOC = pColCU->getSlice()->getPOC();
  UInt uiPartIdxCenter;
  xDeriveCenterIdx( eCUMode, uiPartIdx, uiPartIdxCenter );
  
  if (pColCU->isIntra(uiPartIdxCenter))
  {
    return false;
  }
  
  // Prefer a vector crossing us.  Prefer shortest.
  RefPicList eColRefPicList = REF_PIC_LIST_0;
  bool bFirstCrosses = false;
  Int  iFirstColDist = -1;
  for (Int l = 0; l < 2; l++)
  {
    bool bSaveIt = false;
    int iColRefIdx = pColCU->getCUMvField(RefPicList(l))->getRefIdx(uiPartIdxCenter);
    if (iColRefIdx < 0)
    {
      continue;
    }
    int iColRefPOC = pColCU->getSlice()->getRefPOC(RefPicList(l), iColRefIdx);
    int iColDist = abs(iColRefPOC - iColPOC);
    bool bCrosses = iColPOC < iCurrPOC ? iColRefPOC > iCurrPOC : iColRefPOC < iCurrPOC;
    if (iFirstColDist < 0)
    {
      bSaveIt = true;
    }
    else if (bCrosses && !bFirstCrosses)
    {
      bSaveIt = true;
    }
    else if (bCrosses == bFirstCrosses && l == eRefPicList)
    {
      bSaveIt = true;
    }
    
    if (bSaveIt)
    {
      bFirstCrosses = bCrosses;
      iFirstColDist = iColDist;
      eColRefPicList = RefPicList(l);
    }
  }
  
  // Scale the vector.
  Int iColRefPOC = pColCU->getSlice()->getRefPOC(eColRefPicList, pColCU->getCUMvField(eColRefPicList)->getRefIdx(uiPartIdxCenter));
  TComMv cColMv = pColCU->getCUMvField(eColRefPicList)->getMv(uiPartIdxCenter);
  
  Int iCurrRefPOC = m_pcSlice->getRefPic(eRefPicList, iRefIdx)->getPOC();
  Int iScale = xGetDistScaleFactor(iCurrPOC, iCurrRefPOC, iColPOC, iColRefPOC);
  if ( iScale == 4096 )
  {
    pcMv[0] = cColMv;
  }
  else
  {
    pcMv[0] = cColMv.scaleMv( iScale );
  }
  
  return true;
}

Void TComDataCU::compressMV()
{
  Int scaleFactor = 4 * AMVP_DECIMATION_FACTOR / m_unitSize;
  if (scaleFactor > 0)
  {
    m_acCUMvField[0].compress(m_pePredMode, scaleFactor);
    m_acCUMvField[1].compress(m_pePredMode, scaleFactor);    
  }
}

UInt TComDataCU::getCoefScanIdx(UInt uiAbsPartIdx, UInt uiWidth, Bool bIsLuma, Bool bIsIntra)
{
  static const UChar aucIntraDirToScanIdx[MAX_CU_DEPTH][NUM_INTRA_MODE] =
  {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {0, 1, 2, 0, 0, 1, 1, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0
    },
    {0, 1, 2, 0, 0, 1, 1, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0
    },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
  };
  
  UInt uiCTXIdx;
  UInt uiScanIdx;
  UInt uiDirMode;

  if ( !bIsIntra ) 
  {
    uiScanIdx = SCAN_ZIGZAG;
    return uiScanIdx;
  }

  switch(uiWidth)
  {
    case  2: uiCTXIdx = 6; break;
    case  4: uiCTXIdx = 5; break;
    case  8: uiCTXIdx = 4; break;
    case 16: uiCTXIdx = 3; break;
    case 32: uiCTXIdx = 2; break;
    case 64: uiCTXIdx = 1; break;
    default: uiCTXIdx = 0; break;
  }

  if ( bIsLuma )
  {
    uiDirMode = getLumaIntraDir(uiAbsPartIdx);
    uiScanIdx = aucIntraDirToScanIdx[uiCTXIdx][uiDirMode];
  }
  else
  {
    uiDirMode = getChromaIntraDir(uiAbsPartIdx);
    if( uiDirMode == DM_CHROMA_IDX )
    {
      uiDirMode = getLumaIntraDir(uiAbsPartIdx);
    }
    uiScanIdx = aucIntraDirToScanIdx[max<Int>(uiCTXIdx-1,0)][uiDirMode];
  }

  return uiScanIdx;
}

Bool TComDataCU::useNonSquareTrans(UInt uiTrMode)
{
  const UInt uiLog2TrSize = g_aucConvertToBit[ getSlice()->getSPS()->getMaxCUWidth() >> ( m_puhDepth[ 0 ] + uiTrMode ) ] + 2;
  if ( uiTrMode && uiLog2TrSize < getSlice()->getSPS()->getQuadtreeTULog2MaxSize() && getWidth( 0 ) > 8 &&
      ( m_pePartSize[0] == SIZE_Nx2N || m_pePartSize[0] == SIZE_2NxN || ( m_pePartSize[0] >= SIZE_2NxnU && m_pePartSize[0] <= SIZE_nRx2N ) ) )
  {
    return getSlice()->getSPS()->getUseNSQT();
  }
  else
  {
    return false;
  }
}

Bool TComDataCU::useNonSquareTrans(UInt uiTrMode, Int absPartIdx)
{
  const UInt uiLog2TrSize = g_aucConvertToBit[ getSlice()->getSPS()->getMaxCUWidth() >> ( m_puhDepth[ absPartIdx ] + uiTrMode ) ] + 2;
  if ( uiTrMode && uiLog2TrSize < getSlice()->getSPS()->getQuadtreeTULog2MaxSize() && getWidth( absPartIdx ) > 8 &&
      ( m_pePartSize[absPartIdx] == SIZE_Nx2N || m_pePartSize[absPartIdx] == SIZE_2NxN || ( m_pePartSize[absPartIdx] >= SIZE_2NxnU && m_pePartSize[absPartIdx] <= SIZE_nRx2N ) ) )
  {
    return getSlice()->getSPS()->getUseNSQT();
  }
  else
  {
    return false;
  }
}

Void TComDataCU::getNSQTSize(Int trMode, Int absPartIdx, Int &trWidth, Int &trHeight)
{
  if ( useNonSquareTrans( trMode, absPartIdx ) && trWidth > 4 )
  {
    trWidth  = ( m_pePartSize[absPartIdx] == SIZE_Nx2N || m_pePartSize[absPartIdx] == SIZE_nLx2N || m_pePartSize[absPartIdx] == SIZE_nRx2N )? trWidth >> 1 : trWidth << 1;
    trHeight = ( m_pePartSize[absPartIdx] == SIZE_Nx2N || m_pePartSize[absPartIdx] == SIZE_nLx2N || m_pePartSize[absPartIdx] == SIZE_nRx2N )? trHeight << 1 : trHeight >> 1;
  }
}
  
Void TComDataCU::getPixOffset(UInt uiTrMode,  UInt ui, UInt uiAbsPartIdx, UInt uiDepth, UInt& uiPix_X, UInt& uiPix_Y, TextType eTxt)
{
  UInt uiPartDir = ( m_pePartSize[ 0 ] == SIZE_Nx2N || m_pePartSize[ 0 ] == SIZE_nLx2N || m_pePartSize[ 0 ] == SIZE_nRx2N ) ? 0 : 1;
  UInt uiLog2TrSize = g_aucConvertToBit[ getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;

  if( eTxt != TEXT_LUMA )
  {
    uiLog2TrSize --;
  }
  
  if( uiTrMode == 1 )
  {
    uiPix_X = uiPartDir ? 0 : ui * ( 1 << ( uiLog2TrSize - 1 ) );
    uiPix_Y = uiPartDir ? ui * ( 1 << ( uiLog2TrSize - 1 ) ) : 0;
  }
  else if( ( ( eTxt == TEXT_LUMA && uiLog2TrSize == ( getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - 1 ) ) 
    || ( eTxt != TEXT_LUMA && uiLog2TrSize == ( getSlice()->getSPS()->getQuadtreeTULog2MaxSize() - 2 ) ) ) && uiTrMode == 2)
  {
    UInt uiQPartNumSubdiv = getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
    UInt uilastdepth = uiAbsPartIdx / uiQPartNumSubdiv;
    uiPix_X = ( ( uilastdepth & 0x01 ) << ( uiLog2TrSize + 1 ) ) + ( uiPartDir ? 0 : ui * ( 1 << ( uiLog2TrSize - 1 ) ) );
    uiPix_Y = ( ( ( uilastdepth >> 1 ) & 0x01 ) << ( uiLog2TrSize + 1 ) ) + ( uiPartDir ? ui * ( 1<< ( uiLog2TrSize - 1 ) ) : 0 );
  }
  else 
  {
    UInt uiQPartNumSubdiv = getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1);
    UInt uilastdepth = uiAbsPartIdx / uiQPartNumSubdiv;
    if( ( 1 << uiLog2TrSize ) == 4 )
    {
      uiPix_X = uiPartDir ? ui * ( 1 << uiLog2TrSize ) : uilastdepth * ( 1 << uiLog2TrSize );
      uiPix_Y = uiPartDir ? uilastdepth * ( 1 << uiLog2TrSize ) : ui * ( 1 << uiLog2TrSize );
    }
    else
    {
      uiPix_X = uiPartDir ? ( ui & 0x01 ) * ( 1 << ( uiLog2TrSize + 1 ) ) : uilastdepth * ( 1 << uiLog2TrSize ) + ( ui & 0x01 ) * ( 1 << ( uiLog2TrSize - 1 ) );
      uiPix_Y = uiPartDir ? uilastdepth * ( 1 << uiLog2TrSize ) + ( ( ui >> 1 ) & 0x01 ) * ( 1 << ( uiLog2TrSize - 1 ) ) : ( ( ui >> 1 ) & 0x01 ) * ( 1 << ( uiLog2TrSize + 1 ) );
    }
  }
}

UInt TComDataCU::getSCUAddr()
{ 
  return getPic()->getPicSym()->getInverseCUOrderMap(m_uiCUAddr)*(1<<(m_pcSlice->getSPS()->getMaxCUDepth()<<1))+m_uiAbsIdxInLCU; 
}

/** Set neighboring blocks availabilities for non-deblocked filtering 
 * \param numLCUInPicWidth number of LCUs in picture width
 * \param numLCUInPicHeight number of LCUs in picture height
 * \param numSUInLCUWidth number of SUs in LCU width
 * \param numSUInLCUHeight number of SUs in LCU height
 * \param picWidth picture width
 * \param picHeight picture height
 * \param bIndependentSliceBoundaryEnabled true for independent slice boundary enabled
 * \param bTopTileBoundary true means that top boundary coincides tile boundary
 * \param bDownTileBoundary true means that bottom boundary coincides tile boundary
 * \param bLeftTileBoundary true means that left boundary coincides tile boundary
 * \param bRightTileBoundary true means that right boundary coincides tile boundary
 * \param bIndependentTileBoundaryEnabled true for independent tile boundary enabled
 */
Void TComDataCU::setNDBFilterBlockBorderAvailability(UInt numLCUInPicWidth, UInt numLCUInPicHeight, UInt numSUInLCUWidth, UInt numSUInLCUHeight, UInt picWidth, UInt picHeight
                                                    ,Bool bIndependentSliceBoundaryEnabled
                                                    ,Bool bTopTileBoundary, Bool bDownTileBoundary, Bool bLeftTileBoundary, Bool bRightTileBoundary
                                                    ,Bool bIndependentTileBoundaryEnabled)
{
  UInt numSUInLCU = numSUInLCUWidth*numSUInLCUHeight;
  Int* pSliceIDMapLCU = m_piSliceSUMap;

  UInt uiLPelX, uiTPelY;
  UInt width, height;
  Bool bPicRBoundary, bPicBBoundary, bPicTBoundary, bPicLBoundary;
  Bool bLCURBoundary= false, bLCUBBoundary= false, bLCUTBoundary= false, bLCULBoundary= false;
  Bool* pbAvailBorder;
  Bool* pbAvail;
  UInt rTLSU, rBRSU, widthSU, heightSU;
  UInt zRefSU;
  Int* pRefID;
  Int* pRefMapLCU;
  UInt rTRefSU= 0, rBRefSU= 0, rLRefSU= 0, rRRefSU= 0;
  Int* pRRefMapLCU= NULL;
  Int* pLRefMapLCU= NULL;
  Int* pTRefMapLCU= NULL;
  Int* pBRefMapLCU= NULL;
  Int  sliceID;
  UInt numSGU = (UInt)m_vNDFBlock.size();

  for(Int i=0; i< numSGU; i++)
  {
    NDBFBlockInfo& rSGU = m_vNDFBlock[i];

    sliceID = rSGU.sliceID;
    uiLPelX = rSGU.posX;
    uiTPelY = rSGU.posY;
    width   = rSGU.width;
    height  = rSGU.height;

    rTLSU     = g_auiZscanToRaster[ rSGU.startSU ];
    rBRSU     = g_auiZscanToRaster[ rSGU.endSU   ];
    widthSU   = rSGU.widthSU;
    heightSU  = rSGU.heightSU;

    pbAvailBorder = rSGU.isBorderAvailable;

    bPicTBoundary= (uiTPelY == 0                       )?(true):(false);
    bPicLBoundary= (uiLPelX == 0                       )?(true):(false);
    bPicRBoundary= (!(uiLPelX+ width < picWidth )  )?(true):(false);
    bPicBBoundary= (!(uiTPelY + height < picHeight))?(true):(false);

    bLCULBoundary = (rTLSU % numSUInLCUWidth == 0)?(true):(false);
    bLCURBoundary = ( (rTLSU+ widthSU) % numSUInLCUWidth == 0)?(true):(false);
    bLCUTBoundary = ( (UInt)(rTLSU / numSUInLCUWidth)== 0)?(true):(false);
    bLCUBBoundary = ( (UInt)(rBRSU / numSUInLCUWidth) == (numSUInLCUHeight-1) )?(true):(false);

    //       SGU_L
    pbAvail = &(pbAvailBorder[SGU_L]);
    if(bPicLBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      //      bLCULBoundary = (rTLSU % uiNumSUInLCUWidth == 0)?(true):(false);
      if(bLCULBoundary)
      {
        rLRefSU     = rTLSU + numSUInLCUWidth -1;
        zRefSU      = g_auiRasterToZscan[rLRefSU];
        pRefMapLCU = pLRefMapLCU= (pSliceIDMapLCU - numSUInLCU);
      }
      else
      {
        zRefSU   = g_auiRasterToZscan[rTLSU - 1];
        pRefMapLCU  = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    //       SGU_R
    pbAvail = &(pbAvailBorder[SGU_R]);
    if(bPicRBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      //       bLCURBoundary = ( (rTLSU+ uiWidthSU) % uiNumSUInLCUWidth == 0)?(true):(false);
      if(bLCURBoundary)
      {
        rRRefSU      = rTLSU + widthSU - numSUInLCUWidth;
        zRefSU       = g_auiRasterToZscan[rRRefSU];
        pRefMapLCU  = pRRefMapLCU= (pSliceIDMapLCU + numSUInLCU);
      }
      else
      {
        zRefSU       = g_auiRasterToZscan[rTLSU + widthSU];
        pRefMapLCU  = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    //       SGU_T
    pbAvail = &(pbAvailBorder[SGU_T]);
    if(bPicTBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      //      bLCUTBoundary = ( (UInt)(rTLSU / uiNumSUInLCUWidth)== 0)?(true):(false);
      if(bLCUTBoundary)
      {
        rTRefSU      = numSUInLCU - (numSUInLCUWidth - rTLSU);
        zRefSU       = g_auiRasterToZscan[rTRefSU];
        pRefMapLCU  = pTRefMapLCU= (pSliceIDMapLCU - (numLCUInPicWidth*numSUInLCU));
      }
      else
      {
        zRefSU       = g_auiRasterToZscan[rTLSU - numSUInLCUWidth];
        pRefMapLCU  = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    //       SGU_B
    pbAvail = &(pbAvailBorder[SGU_B]);
    if(bPicBBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      //      bLCUBBoundary = ( (UInt)(rBRSU / uiNumSUInLCUWidth) == (uiNumSUInLCUHeight-1) )?(true):(false);
      if(bLCUBBoundary)
      {
        rBRefSU      = rTLSU % numSUInLCUWidth;
        zRefSU       = g_auiRasterToZscan[rBRefSU];
        pRefMapLCU  = pBRefMapLCU= (pSliceIDMapLCU + (numLCUInPicWidth*numSUInLCU));
      }
      else
      {
        zRefSU       = g_auiRasterToZscan[rTLSU + (heightSU*numSUInLCUWidth)];
        pRefMapLCU  = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    //       SGU_TL
    pbAvail = &(pbAvailBorder[SGU_TL]);
    if(bPicTBoundary || bPicLBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      if(bLCUTBoundary && bLCULBoundary)
      {
        zRefSU       = numSUInLCU -1;
        pRefMapLCU  = pSliceIDMapLCU - ( (numLCUInPicWidth+1)*numSUInLCU);
      }
      else if(bLCUTBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rTRefSU- 1];
        pRefMapLCU  = pTRefMapLCU;
      }
      else if(bLCULBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rLRefSU- numSUInLCUWidth ];
        pRefMapLCU  = pLRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU       = g_auiRasterToZscan[ rTLSU - numSUInLCUWidth -1];
        pRefMapLCU  = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    //       SGU_TR
    pbAvail = &(pbAvailBorder[SGU_TR]);
    if(bPicTBoundary || bPicRBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      if(bLCUTBoundary && bLCURBoundary)
      {
        zRefSU      = g_auiRasterToZscan[numSUInLCU - numSUInLCUWidth];
        pRefMapLCU  = pSliceIDMapLCU - ( (numLCUInPicWidth-1)*numSUInLCU);        
      }
      else if(bLCUTBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rTRefSU+ widthSU];
        pRefMapLCU  = pTRefMapLCU;
      }
      else if(bLCURBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rRRefSU- numSUInLCUWidth ];
        pRefMapLCU  = pRRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU       = g_auiRasterToZscan[ rTLSU - numSUInLCUWidth +widthSU];
        pRefMapLCU  = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    //       SGU_BL
    pbAvail = &(pbAvailBorder[SGU_BL]);
    if(bPicBBoundary || bPicLBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      if(bLCUBBoundary && bLCULBoundary)
      {
        zRefSU      = g_auiRasterToZscan[numSUInLCUWidth - 1];
        pRefMapLCU  = pSliceIDMapLCU + ( (numLCUInPicWidth-1)*numSUInLCU);        
      }
      else if(bLCUBBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rBRefSU - 1];
        pRefMapLCU  = pBRefMapLCU;
      }
      else if(bLCULBoundary)
      {
        zRefSU       = g_auiRasterToZscan[ rLRefSU+ heightSU*numSUInLCUWidth ];
        pRefMapLCU  = pLRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU       = g_auiRasterToZscan[ rTLSU + heightSU*numSUInLCUWidth -1];
        pRefMapLCU  = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    //       SGU_BR
    pbAvail = &(pbAvailBorder[SGU_BR]);
    if(bPicBBoundary || bPicRBoundary)
    {
      *pbAvail = false;
    }
    else if (!bIndependentSliceBoundaryEnabled)
    {
      *pbAvail = true;
    }
    else
    {
      if(bLCUBBoundary && bLCURBoundary)
      {
        zRefSU = 0;
        pRefMapLCU = pSliceIDMapLCU+ ( (numLCUInPicWidth+1)*numSUInLCU);
      }
      else if(bLCUBBoundary)
      {
        zRefSU      = g_auiRasterToZscan[ rBRefSU + widthSU];
        pRefMapLCU = pBRefMapLCU;
      }
      else if(bLCURBoundary)
      {
        zRefSU      = g_auiRasterToZscan[ rRRefSU + (heightSU*numSUInLCUWidth)];
        pRefMapLCU = pRRefMapLCU;
      }
      else //inside LCU
      {
        zRefSU      = g_auiRasterToZscan[ rTLSU + (heightSU*numSUInLCUWidth)+ widthSU];
        pRefMapLCU = pSliceIDMapLCU;
      }
      pRefID = pRefMapLCU + zRefSU;
      *pbAvail = (*pRefID == sliceID)?(true):(false);
    }

    if(bIndependentTileBoundaryEnabled)
    {
      //left LCU boundary
      if(!bPicLBoundary && bLCULBoundary)
      {
        if(bLeftTileBoundary)
        {
          pbAvailBorder[SGU_L] = pbAvailBorder[SGU_TL] = pbAvailBorder[SGU_BL] = false;
        }
      }
      //right LCU boundary
      if(!bPicRBoundary && bLCURBoundary)
      {
        if(bRightTileBoundary)
        {
          pbAvailBorder[SGU_R] = pbAvailBorder[SGU_TR] = pbAvailBorder[SGU_BR] = false;
        }
      }
      //top LCU boundary
      if(!bPicTBoundary && bLCUTBoundary)
      {
        if(bTopTileBoundary)
        {
          pbAvailBorder[SGU_T] = pbAvailBorder[SGU_TL] = pbAvailBorder[SGU_TR] = false;
        }
      }
      //down LCU boundary
      if(!bPicBBoundary && bLCUBBoundary)
      {
        if(bDownTileBoundary)
        {
          pbAvailBorder[SGU_B] = pbAvailBorder[SGU_BL] = pbAvailBorder[SGU_BR] = false;
        }
      }
    }

  }
}


//! \}
