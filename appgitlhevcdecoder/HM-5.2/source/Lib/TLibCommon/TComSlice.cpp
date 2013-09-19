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

/** \file     TComSlice.cpp
    \brief    slice header and SPS class
*/

#include "CommonDef.h"
#include "TComSlice.h"
#include "TComPic.h"

//! \ingroup TLibCommon
//! \{

TComSlice::TComSlice()
: m_iPPSId                        ( -1 )
, m_iPOC                          ( 0 )
, m_iLastIDR                      ( 0 )
, m_eNalUnitType                  ( NAL_UNIT_CODED_SLICE_IDR )
, m_eSliceType                    ( I_SLICE )
, m_iSliceQp                      ( 0 )
#if ADAPTIVE_QP_SELECTION
, m_iSliceQpBase                  ( 0 )
#endif
, m_bLoopFilterDisable            ( false )
, m_inheritDblParamFromAPS       ( true )
, m_loopFilterBetaOffsetDiv2    ( 0 )
, m_loopFilterTcOffsetDiv2      ( 0 )
, m_bRefPicListModificationFlagLC ( false )
, m_bRefPicListCombinationFlag    ( false )
, m_bCheckLDC                     ( false )
, m_iSliceQpDelta                 ( 0 )
, m_iDepth                        ( 0 )
, m_bRefenced                     ( false )
, m_pcSPS                         ( NULL )
, m_pcPPS                         ( NULL )
, m_pcPic                         ( NULL )
, m_uiColDir                      ( 0 )
#if ALF_CHROMA_LAMBDA || SAO_CHROMA_LAMBDA
, m_dLambdaLuma( 0.0 )
, m_dLambdaChroma( 0.0 )
#else
, m_dLambda                       ( 0.0 )
#endif
, m_bNoBackPredFlag               ( false )
, m_bRefIdxCombineCoding          ( false )
, m_uiTLayer                      ( 0 )
, m_bTLayerSwitchingFlag          ( false )
, m_uiSliceMode                   ( 0 )
, m_uiSliceArgument               ( 0 )
, m_uiSliceCurStartCUAddr         ( 0 )
, m_uiSliceCurEndCUAddr           ( 0 )
, m_uiSliceIdx                    ( 0 )
, m_uiEntropySliceMode            ( 0 )
, m_uiEntropySliceArgument        ( 0 )
, m_uiEntropySliceCurStartCUAddr  ( 0 )
, m_uiEntropySliceCurEndCUAddr    ( 0 )
, m_bNextSlice                    ( false )
, m_bNextEntropySlice             ( false )
, m_uiSliceBits                   ( 0 )
, m_uiEntropySliceCounter         ( 0 )
, m_bFinalized                    ( false )
, m_uiTileByteLocation            ( NULL )
, m_uiTileCount                   ( 0 )
, m_iTileMarkerFlag               ( 0 )
, m_uiTileOffstForMultES          ( 0 )
, m_puiSubstreamSizes             ( NULL )
, m_cabacInitIdc                 ( -1 )
{
  m_aiNumRefIdx[0] = m_aiNumRefIdx[1] = m_aiNumRefIdx[2] = 0;
  
  initEqualRef();
  
  for(Int iNumCount = 0; iNumCount < MAX_NUM_REF_LC; iNumCount++)
  {
    m_iRefIdxOfLC[REF_PIC_LIST_0][iNumCount]=-1;
    m_iRefIdxOfLC[REF_PIC_LIST_1][iNumCount]=-1;
    m_eListIdFromIdxOfLC[iNumCount]=0;
    m_iRefIdxFromIdxOfLC[iNumCount]=0;
    m_iRefIdxOfL0FromRefIdxOfL1[iNumCount] = -1;
    m_iRefIdxOfL1FromRefIdxOfL0[iNumCount] = -1;
  }    
  for(Int iNumCount = 0; iNumCount < MAX_NUM_REF; iNumCount++)
  {
    m_apcRefPicList [0][iNumCount] = NULL;
    m_apcRefPicList [1][iNumCount] = NULL;
    m_aiRefPOCList  [0][iNumCount] = 0;
    m_aiRefPOCList  [1][iNumCount] = 0;
  }
  m_bCombineWithReferenceFlag = 0;
  resetWpScaling(m_weightPredTable);
  resetWpScalingLC(m_weightPredTableLC);
  initWpAcDcParam();
}

TComSlice::~TComSlice()
{
  if (m_uiTileByteLocation) 
  {
    delete [] m_uiTileByteLocation;
    m_uiTileByteLocation = NULL;
  }
  delete[] m_puiSubstreamSizes;
  m_puiSubstreamSizes = NULL;
}


Void TComSlice::initSlice()
{
  m_aiNumRefIdx[0]      = 0;
  m_aiNumRefIdx[1]      = 0;
  
  m_uiColDir = 0;
  
  initEqualRef();
  m_bNoBackPredFlag = false;
  m_bRefIdxCombineCoding = false;
  m_bRefPicListCombinationFlag = false;
  m_bRefPicListModificationFlagLC = false;
  m_bCheckLDC = false;

  m_aiNumRefIdx[REF_PIC_LIST_C]      = 0;

  m_uiMaxNumMergeCand = MRG_MAX_NUM_CANDS_SIGNALED;

  m_bFinalized=false;

#if !PARAMSET_VLC_CLEANUP
  Int iWidth             = m_pcSPS->getWidth();
  Int iHeight            = m_pcSPS->getHeight();
  UInt uiWidthInCU       = ( iWidth %g_uiMaxCUWidth  ) ? iWidth /g_uiMaxCUWidth  + 1 : iWidth /g_uiMaxCUWidth;
  UInt uiHeightInCU      = ( iHeight%g_uiMaxCUHeight ) ? iHeight/g_uiMaxCUHeight + 1 : iHeight/g_uiMaxCUHeight;
  UInt uiNumCUsInFrame   = uiWidthInCU * uiHeightInCU;

  m_uiTileCount          = 0;
  if (m_uiTileByteLocation==NULL) m_uiTileByteLocation   = new UInt[uiNumCUsInFrame];
#else
  m_uiTileCount          = 0;
#endif
}

#if PARAMSET_VLC_CLEANUP
Void TComSlice::initTiles()
{
  Int iWidth             = m_pcSPS->getWidth();
  Int iHeight            = m_pcSPS->getHeight();
  UInt uiWidthInCU       = ( iWidth %g_uiMaxCUWidth  ) ? iWidth /g_uiMaxCUWidth  + 1 : iWidth /g_uiMaxCUWidth;
  UInt uiHeightInCU      = ( iHeight%g_uiMaxCUHeight ) ? iHeight/g_uiMaxCUHeight + 1 : iHeight/g_uiMaxCUHeight;
  UInt uiNumCUsInFrame   = uiWidthInCU * uiHeightInCU;

  if (m_uiTileByteLocation==NULL) m_uiTileByteLocation   = new UInt[uiNumCUsInFrame];
}
#endif


/**
 - allocate table to contain substream sizes to be written to the slice header.
 .
 \param uiNumSubstreams Number of substreams -- the allocation will be this value - 1.
 */
Void  TComSlice::allocSubstreamSizes(UInt uiNumSubstreams)
{
  delete[] m_puiSubstreamSizes;
  m_puiSubstreamSizes = new UInt[uiNumSubstreams > 0 ? uiNumSubstreams-1 : 0];
}

Void  TComSlice::sortPicList        (TComList<TComPic*>& rcListPic)
{
  TComPic*    pcPicExtract;
  TComPic*    pcPicInsert;
  
  TComList<TComPic*>::iterator    iterPicExtract;
  TComList<TComPic*>::iterator    iterPicExtract_1;
  TComList<TComPic*>::iterator    iterPicInsert;
  
  for (Int i = 1; i < (Int)(rcListPic.size()); i++)
  {
    iterPicExtract = rcListPic.begin();
    for (Int j = 0; j < i; j++) iterPicExtract++;
    pcPicExtract = *(iterPicExtract);
    pcPicExtract->setCurrSliceIdx(0);
    
    iterPicInsert = rcListPic.begin();
    while (iterPicInsert != iterPicExtract)
    {
      pcPicInsert = *(iterPicInsert);
      pcPicInsert->setCurrSliceIdx(0);
      if (pcPicInsert->getPOC() >= pcPicExtract->getPOC())
      {
        break;
      }
      
      iterPicInsert++;
    }
    
    iterPicExtract_1 = iterPicExtract;    iterPicExtract_1++;
    
    //  swap iterPicExtract and iterPicInsert, iterPicExtract = curr. / iterPicInsert = insertion position
    rcListPic.insert (iterPicInsert, iterPicExtract, iterPicExtract_1);
    rcListPic.erase  (iterPicExtract);
  }
}

TComPic* TComSlice::xGetRefPic (TComList<TComPic*>& rcListPic,
                                UInt                uiPOC)
{
  TComList<TComPic*>::iterator  iterPic = rcListPic.begin();  
  TComPic*                      pcPic = *(iterPic);
  while ( iterPic != rcListPic.end() )
  {
    if(pcPic->getPOC() == uiPOC)
      break;
    iterPic++;
    pcPic = *(iterPic);
  }
  return  pcPic;
}


TComPic* TComSlice::xGetLongTermRefPic (TComList<TComPic*>& rcListPic,
                                UInt                uiPOC)
{
  TComList<TComPic*>::iterator  iterPic = rcListPic.begin();  
  TComPic*                      pcPic = *(iterPic);
  TComPic*                      pcStPic = pcPic;
  while ( iterPic != rcListPic.end() )
  {
    pcPic = *(iterPic);
    if(pcPic && (pcPic->getPOC()%(1<<getSPS()->getBitsForPOC())) == (uiPOC%(1<<getSPS()->getBitsForPOC())))
    {
      if(pcPic->getIsLongTerm())
        return pcPic;
      else
        pcStPic = pcPic;
      break;
    }

    iterPic++;
  }
  return  pcStPic;
}

Void TComSlice::setRefPOCList       ()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  {
    for (Int iNumRefIdx = 0; iNumRefIdx < m_aiNumRefIdx[iDir]; iNumRefIdx++)
    {
      m_aiRefPOCList[iDir][iNumRefIdx] = m_apcRefPicList[iDir][iNumRefIdx]->getPOC();
    }
  }

}

Void TComSlice::generateCombinedList()
{
  if(m_aiNumRefIdx[REF_PIC_LIST_C] > 0)
  {
    m_aiNumRefIdx[REF_PIC_LIST_C]=0;
    for(Int iNumCount = 0; iNumCount < MAX_NUM_REF_LC; iNumCount++)
    {
      m_iRefIdxOfLC[REF_PIC_LIST_0][iNumCount]=-1;
      m_iRefIdxOfLC[REF_PIC_LIST_1][iNumCount]=-1;
      m_eListIdFromIdxOfLC[iNumCount]=0;
      m_iRefIdxFromIdxOfLC[iNumCount]=0;
      m_iRefIdxOfL0FromRefIdxOfL1[iNumCount] = -1;
      m_iRefIdxOfL1FromRefIdxOfL0[iNumCount] = -1;
    }

    for (Int iNumRefIdx = 0; iNumRefIdx < MAX_NUM_REF; iNumRefIdx++)
    {
      if(iNumRefIdx < m_aiNumRefIdx[REF_PIC_LIST_0])
      {
        Bool bTempRefIdxInL2 = true;
        for ( Int iRefIdxLC = 0; iRefIdxLC < m_aiNumRefIdx[REF_PIC_LIST_C]; iRefIdxLC++ )
        {
          if ( m_apcRefPicList[REF_PIC_LIST_0][iNumRefIdx]->getPOC() == m_apcRefPicList[m_eListIdFromIdxOfLC[iRefIdxLC]][m_iRefIdxFromIdxOfLC[iRefIdxLC]]->getPOC() )
          {
            m_iRefIdxOfL1FromRefIdxOfL0[iNumRefIdx] = m_iRefIdxFromIdxOfLC[iRefIdxLC];
            m_iRefIdxOfL0FromRefIdxOfL1[m_iRefIdxFromIdxOfLC[iRefIdxLC]] = iNumRefIdx;
            bTempRefIdxInL2 = false;
            break;
          }
        }

        if(bTempRefIdxInL2 == true)
        { 
          m_eListIdFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = REF_PIC_LIST_0;
          m_iRefIdxFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = iNumRefIdx;
          m_iRefIdxOfLC[REF_PIC_LIST_0][iNumRefIdx] = m_aiNumRefIdx[REF_PIC_LIST_C]++;
        }
      }

      if(iNumRefIdx < m_aiNumRefIdx[REF_PIC_LIST_1])
      {
        Bool bTempRefIdxInL2 = true;
        for ( Int iRefIdxLC = 0; iRefIdxLC < m_aiNumRefIdx[REF_PIC_LIST_C]; iRefIdxLC++ )
        {
          if ( m_apcRefPicList[REF_PIC_LIST_1][iNumRefIdx]->getPOC() == m_apcRefPicList[m_eListIdFromIdxOfLC[iRefIdxLC]][m_iRefIdxFromIdxOfLC[iRefIdxLC]]->getPOC() )
          {
            m_iRefIdxOfL0FromRefIdxOfL1[iNumRefIdx] = m_iRefIdxFromIdxOfLC[iRefIdxLC];
            m_iRefIdxOfL1FromRefIdxOfL0[m_iRefIdxFromIdxOfLC[iRefIdxLC]] = iNumRefIdx;
            bTempRefIdxInL2 = false;
            break;
          }
        }
        if(bTempRefIdxInL2 == true)
        {
          m_eListIdFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = REF_PIC_LIST_1;
          m_iRefIdxFromIdxOfLC[m_aiNumRefIdx[REF_PIC_LIST_C]] = iNumRefIdx;
          m_iRefIdxOfLC[REF_PIC_LIST_1][iNumRefIdx] = m_aiNumRefIdx[REF_PIC_LIST_C]++;
        }
      }
    }
  }
}

Void TComSlice::setRefPicList( TComList<TComPic*>& rcListPic )
{
  if (m_eSliceType == I_SLICE)
  {
    ::memset( m_apcRefPicList, 0, sizeof (m_apcRefPicList));
    ::memset( m_aiNumRefIdx,   0, sizeof ( m_aiNumRefIdx ));
    
    return;
  }
  
  m_aiNumRefIdx[0] = getNumRefIdx(REF_PIC_LIST_0);
  m_aiNumRefIdx[1] = getNumRefIdx(REF_PIC_LIST_1);

  TComPic*  pcRefPic;
  TComPic*  RefPicSetStCurr0[16];
  TComPic*  RefPicSetStCurr1[16];
  TComPic*  RefPicSetLtCurr[16];
  UInt NumPocStCurr0 = 0;
  UInt NumPocStCurr1 = 0;
  UInt NumPocLtCurr = 0;
  Int i;

  for(i=0; i < m_pcRPS->getNumberOfNegativePictures(); i++)
  {
    if(m_pcRPS->getUsed(i))
    {
      pcRefPic = xGetRefPic(rcListPic, getPOC()+m_pcRPS->getDeltaPOC(i));
      pcRefPic->setIsLongTerm(0);
      pcRefPic->getPicYuvRec()->extendPicBorder();
      RefPicSetStCurr0[NumPocStCurr0] = pcRefPic;
      NumPocStCurr0++;
    }
  }
  for(; i < m_pcRPS->getNumberOfNegativePictures()+m_pcRPS->getNumberOfPositivePictures(); i++)
  {
    if(m_pcRPS->getUsed(i))
    {
      pcRefPic = xGetRefPic(rcListPic, getPOC()+m_pcRPS->getDeltaPOC(i));
      pcRefPic->setIsLongTerm(0);
      pcRefPic->getPicYuvRec()->extendPicBorder();
      RefPicSetStCurr1[NumPocStCurr1] = pcRefPic;
      NumPocStCurr1++;
    }
  }
  for(i = m_pcRPS->getNumberOfNegativePictures()+m_pcRPS->getNumberOfPositivePictures()+m_pcRPS->getNumberOfLongtermPictures()-1; i > m_pcRPS->getNumberOfNegativePictures()+m_pcRPS->getNumberOfPositivePictures()-1 ; i--)
  {
    if(m_pcRPS->getUsed(i))
    {
      pcRefPic = xGetLongTermRefPic(rcListPic, m_pcRPS->getPOC(i));
      pcRefPic->setIsLongTerm(1);
      pcRefPic->getPicYuvRec()->extendPicBorder();
      RefPicSetLtCurr[NumPocLtCurr] = pcRefPic;
      NumPocLtCurr++;
    }
  }

  // ref_pic_list_init
  UInt cIdx = 0;
  UInt num_ref_idx_l0_active_minus1 = m_aiNumRefIdx[0] - 1;
  UInt num_ref_idx_l1_active_minus1 = m_aiNumRefIdx[1] - 1;
  while( cIdx <= num_ref_idx_l0_active_minus1 )
  {
    for( i=0; i < NumPocStCurr0 && cIdx <= num_ref_idx_l0_active_minus1; cIdx++, i++ )
      m_apcRefPicList[0][ cIdx ] = RefPicSetStCurr0[ i ];
    for( i=0;  i < NumPocStCurr1 && cIdx <= num_ref_idx_l0_active_minus1; cIdx++, i++ )
      m_apcRefPicList[0][ cIdx ] = RefPicSetStCurr1[ i ]; 
    for( i=0; i < NumPocLtCurr && cIdx <= num_ref_idx_l0_active_minus1; cIdx++, i++ )
      m_apcRefPicList[0][ cIdx ] = RefPicSetLtCurr[ i ];
  }

  if ( m_eSliceType == P_SLICE )
    {
      m_aiNumRefIdx[1] = 0;
      ::memset( m_apcRefPicList[1], 0, sizeof(m_apcRefPicList[1]));
    }
  else
  {
    // ref_pic_list_init
    cIdx = 0;
    while( cIdx <= num_ref_idx_l1_active_minus1 )
    {
      for( i=0; i < NumPocStCurr1 && cIdx <= num_ref_idx_l1_active_minus1; cIdx++, i++ )
        m_apcRefPicList[1][ cIdx ] = RefPicSetStCurr1[ i ];
      for( i=0;  i < NumPocStCurr0 && cIdx <= num_ref_idx_l1_active_minus1; cIdx++, i++ )
        m_apcRefPicList[1][ cIdx ] = RefPicSetStCurr0[ i ];
      for( i=0; i < NumPocLtCurr && cIdx <= num_ref_idx_l1_active_minus1; cIdx++, i++ )
        m_apcRefPicList[1][ cIdx ] = RefPicSetLtCurr[ i ];
    }
  }

  //ref_pic_list_modification_l0
  if(m_RefPicListModification.getRefPicListModificationFlagL0())
  {
    for( i = 0; i < m_RefPicListModification.getNumberOfRefPicListModificationsL0(); i++)
    {
      for( cIdx = num_ref_idx_l0_active_minus1 + 1; cIdx > i; cIdx-- )
        m_apcRefPicList[0][ cIdx ] = m_apcRefPicList[0][ cIdx - 1];
      if(m_RefPicListModification.getListIdcL0(i) == 0)
        m_apcRefPicList[0][ i ] =  RefPicSetStCurr0[ m_RefPicListModification.getRefPicSetIdxL0(i) ];
      else if(m_RefPicListModification.getListIdcL0(i) == 1)
        m_apcRefPicList[0][ i ] =  RefPicSetStCurr1[ m_RefPicListModification.getRefPicSetIdxL0(i) ];
      else if(m_RefPicListModification.getListIdcL0(i) == 2)
        m_apcRefPicList[0][ i ] =  RefPicSetLtCurr[ m_RefPicListModification.getRefPicSetIdxL0(i) ];
      UInt nIdx = i+1;
      for( cIdx = i+1; cIdx <= num_ref_idx_l0_active_minus1 + 1; cIdx++ )
      {
        if( m_apcRefPicList[0][ cIdx ] != m_apcRefPicList[0][ i ] )
          m_apcRefPicList[0][ nIdx++ ] = m_apcRefPicList[0][ cIdx ];
      }
    }
  }
  
  //ref_pic_list_modification_l1
  if(m_eSliceType == B_SLICE && m_RefPicListModification.getRefPicListModificationFlagL1())
  {
    for( i = 0; i < m_RefPicListModification.getNumberOfRefPicListModificationsL1(); i++)
    {
      for( cIdx = num_ref_idx_l1_active_minus1 + 1; cIdx > i; cIdx-- )
        m_apcRefPicList[1][ cIdx ] = m_apcRefPicList[1][ cIdx - 1];
      if(m_RefPicListModification.getListIdcL1(i) == 0)
        m_apcRefPicList[1][ i ] =  RefPicSetStCurr1[ m_RefPicListModification.getRefPicSetIdxL1(i) ];
      else if(m_RefPicListModification.getListIdcL1(i) == 1)
        m_apcRefPicList[1][ i ] =  RefPicSetStCurr0[ m_RefPicListModification.getRefPicSetIdxL1(i) ];
      else if(m_RefPicListModification.getListIdcL1(i) == 2)
        m_apcRefPicList[1][ i ] =  RefPicSetLtCurr[ m_RefPicListModification.getRefPicSetIdxL1(i) ];
      UInt nIdx = i+1;
      for( cIdx = i+1; cIdx <= num_ref_idx_l1_active_minus1 + 1; cIdx++ )
      {
        if( m_apcRefPicList[1][ cIdx ] != m_apcRefPicList[1][ i ] )
          m_apcRefPicList[1][ nIdx++ ] = m_apcRefPicList[1][ cIdx ];
      }
    }
  }
}

Void TComSlice::initEqualRef()
{
  for (Int iDir = 0; iDir < 2; iDir++)
  {
    for (Int iRefIdx1 = 0; iRefIdx1 < MAX_NUM_REF; iRefIdx1++)
    {
      for (Int iRefIdx2 = iRefIdx1; iRefIdx2 < MAX_NUM_REF; iRefIdx2++)
      {
        m_abEqualRef[iDir][iRefIdx1][iRefIdx2] = m_abEqualRef[iDir][iRefIdx2][iRefIdx1] = (iRefIdx1 == iRefIdx2? true : false);
      }
    }
  }
}

Void TComSlice::checkCRA(TComReferencePictureSet *pReferencePictureSet, UInt& pocCRA, TComList<TComPic*>& rcListPic)
{
  for(Int i = 0; i < pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures(); i++)
  {
    if(pocCRA < MAX_UINT && getPOC() > pocCRA)
    {
      assert(getPOC()+pReferencePictureSet->getDeltaPOC(i) >= pocCRA);
    }
  }
  for(Int i = pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures(); i < pReferencePictureSet->getNumberOfPictures(); i++)
  {
    if(pocCRA < MAX_UINT && getPOC() > pocCRA)
    {
      assert(pReferencePictureSet->getPOC(i) >= pocCRA);
    }
  }
  if (getNalUnitType() == NAL_UNIT_CODED_SLICE_CDR) // CDR picture found
  {
    pocCRA = getPOC();
  }
}

/** Function for marking the reference pictures when an IDR and CDR is encountered.
 * \param uiPOCCDR POC of the CDR picture
 * \param bRefreshPending flag indicating if a deferred decoding refresh is pending
 * \param rcListPic reference to the reference picture list
 * This function marks the reference pictures as "unused for reference" in the following conditions.
 * If the nal_unit_type is IDR all pictures in the reference picture list  
 * is marked as "unused for reference" 
 * Otherwise do for the CDR case (non CDR case has no effect since both if conditions below will not be true)
 *    If the bRefreshPending flag is true (a deferred decoding refresh is pending) and the current 
 *    temporal reference is greater than the temporal reference of the latest CDR picture (uiPOCCDR), 
 *    mark all reference pictures except the latest CDR picture as "unused for reference" and set 
 *    the bRefreshPending flag to false.
 *    If the nal_unit_type is CDR, set the bRefreshPending flag to true and iPOCCDR to the temporal 
 *    reference of the current picture.
 * Note that the current picture is already placed in the reference list and its marking is not changed.
 * If the current picture has a nal_ref_idc that is not 0, it will remain marked as "used for reference".
 */
Void TComSlice::decodingRefreshMarking(UInt& uiPOCCDR, Bool& bRefreshPending, TComList<TComPic*>& rcListPic)
{
  TComPic*                 rpcPic;
  UInt uiPOCCurr = getPOC(); 

  if (getNalUnitType() == NAL_UNIT_CODED_SLICE_IDR)  // IDR
  {
    // mark all pictures as not used for reference
    TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
    while (iterPic != rcListPic.end())
    {
      rpcPic = *(iterPic);
      rpcPic->setCurrSliceIdx(0);
      if (rpcPic->getPOC() != uiPOCCurr) rpcPic->getSlice(0)->setReferenced(false);
      iterPic++;
    }
  }
  else // CDR or No DR
  {
    if (bRefreshPending==true && uiPOCCurr > uiPOCCDR) // CDR reference marking pending 
    {
      TComList<TComPic*>::iterator        iterPic       = rcListPic.begin();
      while (iterPic != rcListPic.end())
      {
        rpcPic = *(iterPic);
        if (rpcPic->getPOC() != uiPOCCurr && rpcPic->getPOC() != uiPOCCDR) rpcPic->getSlice(0)->setReferenced(false);
        iterPic++;
      }
      bRefreshPending = false; 
    }
    if (getNalUnitType() == NAL_UNIT_CODED_SLICE_CDR) // CDR picture found
    {
      bRefreshPending = true; 
      uiPOCCDR = uiPOCCurr;
    }
  }
}

Void TComSlice::copySliceInfo(TComSlice *pSrc)
{
  assert( pSrc != NULL );

  Int i, j, k;

  m_iPOC                 = pSrc->m_iPOC;
  m_eNalUnitType         = pSrc->m_eNalUnitType;
  m_eSliceType           = pSrc->m_eSliceType;
  m_iSliceQp             = pSrc->m_iSliceQp;
#if ADAPTIVE_QP_SELECTION
  m_iSliceQpBase         = pSrc->m_iSliceQpBase;
#endif
  m_bLoopFilterDisable   = pSrc->m_bLoopFilterDisable;
  m_inheritDblParamFromAPS = pSrc->m_inheritDblParamFromAPS;
  m_loopFilterBetaOffsetDiv2 = pSrc->m_loopFilterBetaOffsetDiv2;
  m_loopFilterTcOffsetDiv2 = pSrc->m_loopFilterTcOffsetDiv2;
  
  for (i = 0; i < 3; i++)
  {
    m_aiNumRefIdx[i]     = pSrc->m_aiNumRefIdx[i];
  }

  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < MAX_NUM_REF_LC; j++)
    {
       m_iRefIdxOfLC[i][j]  = pSrc->m_iRefIdxOfLC[i][j];
    }
  }
  for (i = 0; i < MAX_NUM_REF_LC; i++)
  {
    m_eListIdFromIdxOfLC[i] = pSrc->m_eListIdFromIdxOfLC[i];
    m_iRefIdxFromIdxOfLC[i] = pSrc->m_iRefIdxFromIdxOfLC[i];
    m_iRefIdxOfL1FromRefIdxOfL0[i] = pSrc->m_iRefIdxOfL1FromRefIdxOfL0[i];
    m_iRefIdxOfL0FromRefIdxOfL1[i] = pSrc->m_iRefIdxOfL0FromRefIdxOfL1[i];
  }
  m_bRefPicListModificationFlagLC = pSrc->m_bRefPicListModificationFlagLC;
  m_bRefPicListCombinationFlag    = pSrc->m_bRefPicListCombinationFlag;
  m_bCheckLDC             = pSrc->m_bCheckLDC;
  m_iSliceQpDelta        = pSrc->m_iSliceQpDelta;
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < MAX_NUM_REF; j++)
    {
      m_apcRefPicList[i][j]  = pSrc->m_apcRefPicList[i][j];
      m_aiRefPOCList[i][j]   = pSrc->m_aiRefPOCList[i][j];
    }
  }  
  m_iDepth               = pSrc->m_iDepth;

  // referenced slice
  m_bRefenced            = pSrc->m_bRefenced;

  // access channel
  m_pcSPS                = pSrc->m_pcSPS;
  m_pcPPS                = pSrc->m_pcPPS;
  m_pcRPS                = pSrc->m_pcRPS;
  m_iLastIDR             = pSrc->m_iLastIDR;

  m_pcPic                = pSrc->m_pcPic;
  m_pcAPS                = pSrc->m_pcAPS;
  m_iAPSId               = pSrc->m_iAPSId;

  m_uiColDir             = pSrc->m_uiColDir;
#if ALF_CHROMA_LAMBDA || SAO_CHROMA_LAMBDA 
  m_dLambdaLuma          = pSrc->m_dLambdaLuma;
  m_dLambdaChroma        = pSrc->m_dLambdaChroma;
#else
  m_dLambda              = pSrc->m_dLambda;
#endif
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < MAX_NUM_REF; j++)
    {
      for (k =0; k < MAX_NUM_REF; k++)
      {
        m_abEqualRef[i][j][k] = pSrc->m_abEqualRef[i][j][k];
      }
    }
  }

  m_bNoBackPredFlag      = pSrc->m_bNoBackPredFlag;
  m_bRefIdxCombineCoding = pSrc->m_bRefIdxCombineCoding;

  m_uiTLayer                      = pSrc->m_uiTLayer;
  m_bTLayerSwitchingFlag          = pSrc->m_bTLayerSwitchingFlag;

  m_uiSliceMode                   = pSrc->m_uiSliceMode;
  m_uiSliceArgument               = pSrc->m_uiSliceArgument;
  m_uiSliceCurStartCUAddr         = pSrc->m_uiSliceCurStartCUAddr;
  m_uiSliceCurEndCUAddr           = pSrc->m_uiSliceCurEndCUAddr;
  m_uiSliceIdx                    = pSrc->m_uiSliceIdx;
  m_uiEntropySliceMode            = pSrc->m_uiEntropySliceMode;
  m_uiEntropySliceArgument        = pSrc->m_uiEntropySliceArgument; 
  m_uiEntropySliceCurStartCUAddr  = pSrc->m_uiEntropySliceCurStartCUAddr;
  m_uiEntropySliceCurEndCUAddr    = pSrc->m_uiEntropySliceCurEndCUAddr;
  m_bNextSlice                    = pSrc->m_bNextSlice;
  m_bNextEntropySlice             = pSrc->m_bNextEntropySlice;
  m_iTileMarkerFlag             = pSrc->m_iTileMarkerFlag;
  for ( int e=0 ; e<2 ; e++ )
    for ( int n=0 ; n<MAX_NUM_REF ; n++ )
      memcpy(m_weightPredTable[e][n], pSrc->m_weightPredTable[e][n], sizeof(wpScalingParam)*3 );
}

int TComSlice::m_iPrevPOC = 0;
/** Function for setting the slice's temporal layer ID and corresponding temporal_layer_switching_point_flag.
 * \param uiTLayer Temporal layer ID of the current slice
 * The decoder calls this function to set temporal_layer_switching_point_flag for each temporal layer based on 
 * the SPS's temporal_id_nesting_flag and the parsed PPS.  Then, current slice's temporal layer ID and 
 * temporal_layer_switching_point_flag is set accordingly.
 */
Void TComSlice::setTLayerInfo( UInt uiTLayer )
{
  // If temporal_id_nesting_flag == 1, then num_temporal_layer_switching_point_flags shall be inferred to be 0 and temporal_layer_switching_point_flag shall be inferred to be 1 for all temporal layers
  if ( m_pcSPS->getTemporalIdNestingFlag() ) 
  {
    m_pcPPS->setNumTLayerSwitchingFlags( 0 );
    for ( UInt i = 0; i < MAX_TLAYER; i++ )
    {
      m_pcPPS->setTLayerSwitchingFlag( i, true );
    }
  }
  else 
  {
    for ( UInt i = m_pcPPS->getNumTLayerSwitchingFlags(); i < MAX_TLAYER; i++ )
    {
      m_pcPPS->setTLayerSwitchingFlag( i, false );
    }
  }

  m_uiTLayer = uiTLayer;
  m_bTLayerSwitchingFlag = m_pcPPS->getTLayerSwitchingFlag( uiTLayer );
}

/** Function for applying picture marking based on the Reference Picture Set in pReferencePictureSet.
*/
Void TComSlice::applyReferencePictureSet( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet)
{
  TComPic* rpcPic;
  Int i, isReference;

  Int j = 0;
  // loop through all pictures in the reference picture buffer
  TComList<TComPic*>::iterator iterPic = rcListPic.begin();
  while ( iterPic != rcListPic.end())
  {
    j++;
    rpcPic = *(iterPic++);

    isReference = 0;
    // loop through all pictures in the Reference Picture Set
    // to see if the picture should be kept as reference picture
    for(i=0;i<pReferencePictureSet->getNumberOfPositivePictures()+pReferencePictureSet->getNumberOfNegativePictures();i++)
    {
      if(!rpcPic->getIsLongTerm() && rpcPic->getPicSym()->getSlice(0)->getPOC() == this->getPOC() + pReferencePictureSet->getDeltaPOC(i))
      {
        isReference = 1;
        rpcPic->setUsedByCurr(pReferencePictureSet->getUsed(i));
        rpcPic->setIsLongTerm(0);
      }
    }
    for(;i<pReferencePictureSet->getNumberOfPictures();i++)
    {
      if(rpcPic->getIsLongTerm() && (rpcPic->getPicSym()->getSlice(0)->getPOC()%(1<<rpcPic->getPicSym()->getSlice(0)->getSPS()->getBitsForPOC())) == pReferencePictureSet->getPOC(i)%(1<<rpcPic->getPicSym()->getSlice(0)->getSPS()->getBitsForPOC()))
      {
        isReference = 1;
        rpcPic->setUsedByCurr(pReferencePictureSet->getUsed(i));
      }
    }
    // mark the picture as "unused for reference" if it is not in
    // the Reference Picture Set
    if(rpcPic->getPicSym()->getSlice(0)->getPOC() != this->getPOC() && isReference == 0)    
    {            
      rpcPic->getSlice( 0 )->setReferenced( false );   
      rpcPic->setIsLongTerm(0);
    }
  }  
}

/** Function for applying picture marking based on the Reference Picture Set in pReferencePictureSet.
*/
Int TComSlice::checkThatAllRefPicsAreAvailable( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet, Bool outputFlag)
{
  TComPic* rpcPic;
  Int i, isAvailable, j;
  Int atLeastOneLost = 0;
  Int atLeastOneRemoved = 0;
  Int iPocLost = 0;

  // loop through all long-term pictures in the Reference Picture Set
  // to see if the picture should be kept as reference picture
  for(i=pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures();i<pReferencePictureSet->getNumberOfPictures();i++)
  {
    j = 0;
    isAvailable = 0;
    // loop through all pictures in the reference picture buffer
    TComList<TComPic*>::iterator iterPic = rcListPic.begin();
    while ( iterPic != rcListPic.end())
    {
      j++;
      rpcPic = *(iterPic++);
      if(rpcPic->getIsLongTerm() && (rpcPic->getPicSym()->getSlice(0)->getPOC()%(1<<rpcPic->getPicSym()->getSlice(0)->getSPS()->getBitsForPOC())) == pReferencePictureSet->getPOC(i)%(1<<rpcPic->getPicSym()->getSlice(0)->getSPS()->getBitsForPOC()) && rpcPic->getSlice(0)->isReferenced())
      {
        isAvailable = 1;
      }
    }
    // if there was no such long-term check the short terms
    if(!isAvailable)
    {
      iterPic = rcListPic.begin();
      while ( iterPic != rcListPic.end())
      {
        j++;
        rpcPic = *(iterPic++);

        if((rpcPic->getPicSym()->getSlice(0)->getPOC()%(1<<rpcPic->getPicSym()->getSlice(0)->getSPS()->getBitsForPOC())) == (this->getPOC() + pReferencePictureSet->getDeltaPOC(i))%(1<<rpcPic->getPicSym()->getSlice(0)->getSPS()->getBitsForPOC()) && rpcPic->getSlice(0)->isReferenced())
        {
          isAvailable = 1;
          rpcPic->setIsLongTerm(1);
          break;
        }
      }
    }
    // report that a picture is lost if it is in the Reference Picture Set
    // but not available as reference picture
    if(isAvailable == 0)    
    {            
      if(!pReferencePictureSet->getUsed(i) )
      {
        if(outputFlag)
          printf("\nLong-term reference picture with POC = %3d seems to have been removed or not correctly decoded.", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneRemoved = 1;
      }
      else
      {
        if(outputFlag)
          printf("\nLong-term reference picture with POC = %3d is lost or not correctly decoded!", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneLost = 1;
        iPocLost=this->getPOC() + pReferencePictureSet->getDeltaPOC(i);
      }
    }
  }  
  // loop through all short-term pictures in the Reference Picture Set
  // to see if the picture should be kept as reference picture
  for(i=0;i<pReferencePictureSet->getNumberOfNegativePictures()+pReferencePictureSet->getNumberOfPositivePictures();i++)
  {
    j = 0;
    isAvailable = 0;
    // loop through all pictures in the reference picture buffer
    TComList<TComPic*>::iterator iterPic = rcListPic.begin();
    while ( iterPic != rcListPic.end())
    {
      j++;
      rpcPic = *(iterPic++);

      if(!rpcPic->getIsLongTerm() && rpcPic->getPicSym()->getSlice(0)->getPOC() == this->getPOC() + pReferencePictureSet->getDeltaPOC(i) && rpcPic->getSlice(0)->isReferenced())
      {
        isAvailable = 1;
      }
    }
    // report that a picture is lost if it is in the Reference Picture Set
    // but not available as reference picture
    if(isAvailable == 0)    
    {            
      if(!pReferencePictureSet->getUsed(i) )
      {
        if(outputFlag)
          printf("\nShort-term reference picture with POC = %3d seems to have been removed or not correctly decoded.", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneRemoved = 1;
      }
      else
      {
        if(outputFlag)
          printf("\nShort-term reference picture with POC = %3d is lost or not correctly decoded!", this->getPOC() + pReferencePictureSet->getDeltaPOC(i));
        atLeastOneLost = 1;
        iPocLost=this->getPOC() + pReferencePictureSet->getDeltaPOC(i);
      }
    }
  }    
  if(atLeastOneLost)
  {
    return iPocLost+1;
  }
  if(atLeastOneRemoved)
  {
    return -2;
  }
  else
    return 0;
}

/** Function for constructing an explicit Reference Picture Set out of the available pictures in a referenced Reference Picture Set
*/
Void TComSlice::createExplicitReferencePictureSetFromReference( TComList<TComPic*>& rcListPic, TComReferencePictureSet *pReferencePictureSet)
{
  TComPic* rpcPic;
  Int i, j;
  Int k = 0;
  Int nrOfNegativePictures = 0;
  Int nrOfPositivePictures = 0;
  TComReferencePictureSet* pcRPS = this->getLocalRPS();

  // loop through all pictures in the Reference Picture Set
  for(i=0;i<pReferencePictureSet->getNumberOfPictures();i++)
  {
    j = 0;
    // loop through all pictures in the reference picture buffer
    TComList<TComPic*>::iterator iterPic = rcListPic.begin();
    while ( iterPic != rcListPic.end())
    {
      j++;
      rpcPic = *(iterPic++);

      if(rpcPic->getPicSym()->getSlice(0)->getPOC() == this->getPOC() + pReferencePictureSet->getDeltaPOC(i) && rpcPic->getSlice(0)->isReferenced())
      {
        // This picture exists as a reference picture
        // and should be added to the explicit Reference Picture Set
        pcRPS->setDeltaPOC(k, pReferencePictureSet->getDeltaPOC(i));
        pcRPS->setUsed(k, pReferencePictureSet->getUsed(i));
        if(pcRPS->getDeltaPOC(k) < 0)
          nrOfNegativePictures++;
        else
          nrOfPositivePictures++;
        k++;
      }
    }
  }
  pcRPS->setNumberOfNegativePictures(nrOfNegativePictures);
  pcRPS->setNumberOfPositivePictures(nrOfPositivePictures);
  pcRPS->setNumberOfPictures(nrOfNegativePictures+nrOfPositivePictures);
  // This is a simplistic inter rps example. A smarter encoder will look for a better reference RPS to do the 
  // inter RPS prediction with.  Here we just use the reference used by pReferencePictureSet.
  // If pReferencePictureSet is not inter_RPS_predicted, then inter_RPS_prediction is for the current RPS also disabled.
  if (!pReferencePictureSet->getInterRPSPrediction())
  {
    pcRPS->setInterRPSPrediction(false);
    pcRPS->setNumRefIdc(0);
  }
  else
  {
    Int rIdx =  this->getRPSidx() - pReferencePictureSet->getDeltaRIdxMinus1() - 1;
    Int deltaRPS = pReferencePictureSet->getDeltaRPS();
    TComReferencePictureSet* pcRefRPS = this->getPPS()->getRPSList()->getReferencePictureSet(rIdx);
    Int iRefPics = pcRefRPS->getNumberOfPictures();
    Int iNewIdc=0;
    for(i=0; i<= iRefPics; i++) 
    {
      Int deltaPOC = ((i != iRefPics)? pcRefRPS->getDeltaPOC(i) : 0);  // check if the reference abs POC is >= 0
      Int iRefIdc = 0;
      for (j=0; j < pcRPS->getNumberOfPictures(); j++) // loop through the  pictures in the new RPS
      {
        if ( (deltaPOC + deltaRPS) == pcRPS->getDeltaPOC(j))
        {
          if (pcRPS->getUsed(j))
          {
            iRefIdc = 1;
          }
          else
          {
            iRefIdc = 2;
          }
        }
      }
      pcRPS->setRefIdc(i, iRefIdc);
      iNewIdc++;
    }
    pcRPS->setInterRPSPrediction(true);
    pcRPS->setNumRefIdc(iNewIdc);
    pcRPS->setDeltaRPS(deltaRPS); 
    pcRPS->setDeltaRIdxMinus1(pReferencePictureSet->getDeltaRIdxMinus1() + this->getPPS()->getRPSList()->getNumberOfReferencePictureSets() - this->getRPSidx());
  }

  this->setRPS(pcRPS);
  this->setRPSidx(-1);
}

Void TComSlice::decodingMarkingForNoTMVP( TComList<TComPic*>& rcListPic, Int currentPOC )
{
  TComList<TComPic*>::iterator it;
  for ( it = rcListPic.begin(); it != rcListPic.end(); it++ )
  {
    if ( (*it)->getSlice(0)->getPOC() != currentPOC )
    {
      (*it)->setUsedForTMVP( false );
    }
  }
}

/** get AC and DC values for weighted pred
 * \param *wp
 * \returns Void
 */
Void  TComSlice::getWpAcDcParam(wpACDCParam *&wp)
{
  wp = m_weightACDCParam;
}

/** init AC and DC values for weighted pred
 * \returns Void
 */
Void  TComSlice::initWpAcDcParam()
{
  for(Int iComp = 0; iComp < 3; iComp++ )
  {
    m_weightACDCParam[iComp].iAC = 0;
    m_weightACDCParam[iComp].iDC = 0;
  }
}

/** get WP tables for weighted pred
 * \param RefPicList
 * \param iRefIdx
 * \param *&wpScalingParam
 * \returns Void
 */
Void  TComSlice::getWpScaling( RefPicList e, Int iRefIdx, wpScalingParam *&wp )
{
  wp = m_weightPredTable[e][iRefIdx];
}

/** reset Default WP tables settings : no weight. 
 * \param wpScalingParam
 * \returns Void
 */
Void  TComSlice::resetWpScaling(wpScalingParam  wp[2][MAX_NUM_REF][3])
{
  for ( int e=0 ; e<2 ; e++ ) 
  {
    for ( int i=0 ; i<MAX_NUM_REF ; i++ )
    {
      for ( int yuv=0 ; yuv<3 ; yuv++ ) 
      {
        wpScalingParam  *pwp = &(wp[e][i][yuv]);
        pwp->bPresentFlag      = false;
        pwp->uiLog2WeightDenom = 0;
        pwp->uiLog2WeightDenom = 0;
        pwp->iWeight           = 1;
        pwp->iOffset           = 0;
      }
    }
  }
}

/** init WP table
 * \returns Void
 */
Void  TComSlice::initWpScaling()
{
  initWpScaling(m_weightPredTable);
}

/** set WP tables 
 * \param wpScalingParam
 * \returns Void
 */
Void  TComSlice::initWpScaling(wpScalingParam  wp[2][MAX_NUM_REF][3])
{
  for ( int e=0 ; e<2 ; e++ ) 
  {
    for ( int i=0 ; i<MAX_NUM_REF ; i++ )
    {
      for ( int yuv=0 ; yuv<3 ; yuv++ ) 
      {
        wpScalingParam  *pwp = &(wp[e][i][yuv]);
        if ( !pwp->bPresentFlag ) {
          // Inferring values not present :
          pwp->iWeight = (1 << pwp->uiLog2WeightDenom);
          pwp->iOffset = 0;
        }

        pwp->w      = pwp->iWeight;
        pwp->o      = pwp->iOffset * (1 << (g_uiBitDepth-8));
        pwp->shift  = pwp->uiLog2WeightDenom;
        pwp->round  = (pwp->uiLog2WeightDenom>=1) ? (1 << (pwp->uiLog2WeightDenom-1)) : (0);
      }
    }
  }
}

/** get WP tables for weighted pred of LC
 * \param iRefIdxLC
 * \param *&wpScalingParam
 * \returns Void
 */
Void TComSlice::getWpScalingLC( Int iRefIdx, wpScalingParam *&wp )
{
  wp = m_weightPredTableLC[iRefIdx];
}
/** reset Default WP tables settings for LC : no weight. 
 * \param wpScalingParam
 * \returns Void
 */
Void TComSlice::resetWpScalingLC(wpScalingParam  wp[2*MAX_NUM_REF][3])
{
  for ( int i=0 ; i<2*MAX_NUM_REF ; i++ )
  {
    for ( int yuv=0 ; yuv<3 ; yuv++ ) 
    {
      wpScalingParam  *pwp = &(wp[i][yuv]);
      pwp->bPresentFlag      = false;
      pwp->uiLog2WeightDenom = 0;
      pwp->uiLog2WeightDenom = 0;
      pwp->iWeight           = 1;
      pwp->iOffset           = 0;
    }
  }
}
/** set current WP tables settings for LC
 * \returns Void
 */
Void TComSlice::setWpParamforLC()
{
  for ( Int iRefIdx=0 ; iRefIdx<getNumRefIdx(REF_PIC_LIST_C) ; iRefIdx++ )
  {
    RefPicList eRefPicList = (RefPicList)getListIdFromIdxOfLC(iRefIdx);
    Int iCombRefIdx = getRefIdxFromIdxOfLC(iRefIdx);

    wpScalingParam  *wp_src, *wp_dst;
    getWpScalingLC(iRefIdx, wp_src);
    getWpScaling(eRefPicList, iCombRefIdx, wp_dst);
    copyWPtable(wp_src, wp_dst);

    if(eRefPicList == REF_PIC_LIST_0)
    {
      Int iRefIdxL1 = getRefIdxOfL1FromRefIdxOfL0(iCombRefIdx);
      if(iRefIdxL1 >= 0)
      {
        getWpScaling(REF_PIC_LIST_1, iRefIdxL1, wp_dst);
        copyWPtable(wp_src, wp_dst);
      }
    }
    if(eRefPicList == REF_PIC_LIST_1)
    {
      Int iRefIdxL0 = getRefIdxOfL0FromRefIdxOfL1(iCombRefIdx);
      if(iRefIdxL0 >= 0)
      {
        getWpScaling(REF_PIC_LIST_0, iRefIdxL0, wp_dst);
        copyWPtable(wp_src, wp_dst);
      }
    }
  }
  initWpScaling();
}
/** copy source WP tables to destination table for LC
 * \param wpScalingParam *&wp_src : source
 * \param wpScalingParam *&wp_dst : destination
 * \returns Void
 */
Void TComSlice::copyWPtable(wpScalingParam *&wp_src, wpScalingParam *&wp_dst)
{
  for ( Int iComp = 0; iComp < 3; iComp++ )
  {
    wp_dst[iComp].uiLog2WeightDenom = (iComp==0) ? wp_src[0].uiLog2WeightDenom : wp_src[1].uiLog2WeightDenom;
    wp_dst[iComp].bPresentFlag = wp_src[iComp].bPresentFlag;
    wp_dst[iComp].iWeight = wp_src[iComp].iWeight;
    wp_dst[iComp].iOffset = wp_src[iComp].iOffset;
  }
}


// ------------------------------------------------------------------------------------------------
// Sequence parameter set (SPS)
// ------------------------------------------------------------------------------------------------

TComSPS::TComSPS()
: m_SPSId                     (  0)
, m_ProfileIdc                (  0)
, m_LevelIdc                  (  0)
, m_chromaFormatIdc           (CHROMA_420)
, m_uiMaxTLayers              (  1)
// Structure
, m_uiWidth                   (352)
, m_uiHeight                  (288)
, m_uiMaxCUWidth              ( 32)
, m_uiMaxCUHeight             ( 32)
, m_uiMaxCUDepth              (  3)
, m_uiMinTrDepth              (  0)
, m_uiMaxTrDepth              (  1)
, m_numReorderFrames          (  0)
, m_uiQuadtreeTULog2MaxSize   (  0)
, m_uiQuadtreeTULog2MinSize   (  0)
, m_uiQuadtreeTUMaxDepthInter (  0)
, m_uiQuadtreeTUMaxDepthIntra (  0)
// Tool list
, m_usePCM                   (false)
, m_pcmLog2MaxSize            (  5)
, m_uiPCMLog2MinSize          (  7)
, m_bDisInter4x4              (  1)
, m_bUseALF                   (false)
, m_bUsePAD                   (false)
, m_bUseMRG                   (false)
, m_bUseLMChroma              (false)
, m_bUseLComb                 (false)
, m_bLCMod                    (false)
, m_uiBitDepth                (  8)
, m_uiBitIncrement            (  0)
, m_uiPCMBitDepthLuma         (  8)
, m_uiPCMBitDepthChroma       (  8)
, m_bPCMFilterDisableFlag     (false)
, m_uiBitsForPOC              (  8)
, m_uiMaxTrSize               ( 32)
, m_bLFCrossSliceBoundaryFlag (false)
, m_bUseSAO                   (false) 
, m_bTemporalIdNestingFlag    (false)
, m_scalingListEnabledFlag    (false)
, m_uiMaxDecFrameBuffering    (  0)
, m_uiMaxLatencyIncrease      (  0)
{
  // AMVP parameter
  ::memset( m_aeAMVPMode, 0, sizeof( m_aeAMVPMode ) );
}

TComSPS::~TComSPS()
{
  if( m_iNumColumnsMinus1 > 0 && m_iUniformSpacingIdr == 0 )
  {
    delete [] m_puiColumnWidth; 
    m_puiColumnWidth = NULL;
  }
  if( m_iNumRowsMinus1 > 0 && m_iUniformSpacingIdr == 0 )
  {
    delete [] m_puiRowHeight;
    m_puiRowHeight = NULL;
  }
}

TComPPS::TComPPS()
: m_PPSId                       (0)
, m_SPSId                       (0)
, m_picInitQPMinus26            (0)
, m_useDQP                      (false)
, m_bConstrainedIntraPred       (false)
, m_pcSPS                       (NULL)
, m_uiMaxCuDQPDepth             (0)
, m_uiMinCuDQPSize              (0)
, m_iChromaQpOffset             (0)
, m_iChromaQpOffset2nd          (0)
, m_bLongTermRefsPresent        (false)
, m_uiBitsForLongTermRefs       (0)
, m_uiNumTlayerSwitchingFlags   (0)
, m_iSliceGranularity           (0)
, m_iTileBehaviorControlPresentFlag (0)
, m_bLFCrossTileBoundaryFlag     (true)
, m_iColumnRowInfoPresent        (0)
, m_iUniformSpacingIdr           (0)
, m_iTileBoundaryIndependenceIdr (0)
, m_iNumColumnsMinus1            (0)
, m_puiColumnWidth               (NULL)
, m_iNumRowsMinus1               (0)
, m_puiRowHeight                 (NULL)
,  m_iEntropyCodingSynchro      (0)
,  m_bCabacIstateReset          (false)
,  m_iNumSubstreams             (1)
{
  for ( UInt i = 0; i < MAX_TLAYER; i++ )
  {
    m_abTLayerSwitchingFlag[i] = false;
  }
}

TComPPS::~TComPPS()
{
  if( m_iNumColumnsMinus1 > 0 && m_iUniformSpacingIdr == 0 )
  {
    if (m_puiColumnWidth) delete [] m_puiColumnWidth; 
    m_puiColumnWidth = NULL;
  }
  if( m_iNumRowsMinus1 > 0 && m_iUniformSpacingIdr == 0 )
  {
    if (m_puiRowHeight) delete [] m_puiRowHeight;
    m_puiRowHeight = NULL;
  }
}

TComReferencePictureSet::TComReferencePictureSet()
: m_uiNumberOfPictures (0)
, m_uiNumberOfNegativePictures (0)
, m_uiNumberOfPositivePictures (0)
, m_uiNumberOfLongtermPictures (0)
, m_bInterRPSPrediction (0) 
, m_iDeltaRIdxMinus1 (0)   
, m_iDeltaRPS (0) 
, m_iNumRefIdc (0) 
{
  ::memset( m_piDeltaPOC, 0, sizeof(m_piDeltaPOC) );
  ::memset( m_piPOC, 0, sizeof(m_piPOC) );
  ::memset( m_pbUsed, 0, sizeof(m_pbUsed) );
  ::memset( m_piRefIdc, 0, sizeof(m_piRefIdc) );
}

TComReferencePictureSet::~TComReferencePictureSet()
{
}

Void TComReferencePictureSet::setUsed(UInt uiBufferNum, Bool bUsed)
{
   m_pbUsed[uiBufferNum] = bUsed;
}

Void TComReferencePictureSet::setDeltaPOC(UInt uiBufferNum, Int iDeltaPOC)
{
   m_piDeltaPOC[uiBufferNum] = iDeltaPOC;
}

Void TComReferencePictureSet::setNumberOfPictures(UInt NumberOfPictures)
{
   m_uiNumberOfPictures = NumberOfPictures;
}

UInt TComReferencePictureSet::getUsed(UInt uiBufferNum)
{
   return (UInt)m_pbUsed[uiBufferNum];
}

Int TComReferencePictureSet::getDeltaPOC(UInt uiBufferNum)
{
   return m_piDeltaPOC[uiBufferNum];
}

UInt TComReferencePictureSet::getNumberOfPictures()
{
   return m_uiNumberOfPictures;
}

Int TComReferencePictureSet::getPOC(UInt uiBufferNum)
{
   return m_piPOC[uiBufferNum];
}
Void TComReferencePictureSet::setPOC(UInt uiBufferNum, Int iPOC)
{
   m_piPOC[uiBufferNum] = iPOC;
}

/** set the reference idc value at uiBufferNum entry to the value of iRefIdc
 * \param uiBufferNum
 * \param iRefIdc
 * \returns Void
 */
Void TComReferencePictureSet::setRefIdc(UInt uiBufferNum, Int iRefIdc)
{
   m_piRefIdc[uiBufferNum] = iRefIdc;
}

/** get the reference idc value at uiBufferNum
 * \param uiBufferNum
 * \returns Int
 */
Int  TComReferencePictureSet::getRefIdc(UInt uiBufferNum)
{
   return m_piRefIdc[uiBufferNum];
}

/** Sorts the deltaPOC and Used by current values in the RPS based on the deltaPOC values.
 *  deltaPOC values are sorted with -ve values before the +ve values.  -ve values are in decreasing order.
 *  +ve values are in increasing order.
 * \returns Void
 */
Void TComReferencePictureSet::sortDeltaPOC()
{
  // sort in increasing order (smallest first)
  for(Int j=1; j < getNumberOfPictures(); j++)
  { 
    Int deltaPOC = getDeltaPOC(j);
    Bool bUsed = getUsed(j);
    for (Int k=j-1; k >= 0; k--)
    {
      Int temp = getDeltaPOC(k);
      if (deltaPOC < temp)
      {
        setDeltaPOC(k+1, temp);
        setUsed(k+1, getUsed(k));
        setDeltaPOC(k, deltaPOC);
        setUsed(k, bUsed);
      }
    }
  }
  // flip the negative values to largest first
  Int NumNegPics = getNumberOfNegativePictures();
  for(Int j=0, k=NumNegPics-1; j < NumNegPics>>1; j++, k--)
  { 
    Int deltaPOC = getDeltaPOC(j);
    Bool bUsed = getUsed(j);
    setDeltaPOC(j, getDeltaPOC(k));
    setUsed(j, getUsed(k));
    setDeltaPOC(k, deltaPOC);
    setUsed(k, bUsed);
  }
}

/** Prints the deltaPOC and RefIdc (if available) values in the RPS.
 *  A "*" is added to the deltaPOC value if it is Used bu current.
 * \returns Void
 */
Void TComReferencePictureSet::printDeltaPOC()
{
  printf("DeltaPOC = { ");
  for(Int j=0; j < getNumberOfPictures(); j++)
  {
    printf("%d%s ", getDeltaPOC(j), (getUsed(j)==1)?"*":"");
  } 
  if (getInterRPSPrediction()) 
  {
    printf("}, RefIdc = { ");
    for(Int j=0; j < getNumRefIdc(); j++)
    {
      printf("%d ", getRefIdc(j));
    } 
  }
  printf("}\n");
}

TComRPS::TComRPS()
{
}

TComRPS::~TComRPS()
{
}

Void TComRPS::create( UInt uiNumberOfReferencePictureSets)
{
  m_uiNumberOfReferencePictureSets = uiNumberOfReferencePictureSets;
  m_pReferencePictureSet = new TComReferencePictureSet[uiNumberOfReferencePictureSets];
}

Void TComRPS::destroy()
{
  delete [] m_pReferencePictureSet;     
  m_uiNumberOfReferencePictureSets = 0;
  m_pReferencePictureSet = NULL;
}



TComReferencePictureSet* TComRPS::getReferencePictureSet(UInt uiReferencePictureSetNum)
{
   return &m_pReferencePictureSet[uiReferencePictureSetNum];
}

UInt TComRPS::getNumberOfReferencePictureSets()
{
   return m_uiNumberOfReferencePictureSets;
}

Void TComRPS::setNumberOfReferencePictureSets(UInt uiNumberOfReferencePictureSets)
{
   m_uiNumberOfReferencePictureSets = uiNumberOfReferencePictureSets;
}

TComRefPicListModification::TComRefPicListModification()
: m_bRefPicListModificationFlagL0 (false)
, m_bRefPicListModificationFlagL1 (false)
, m_uiNumberOfRefPicListModificationsL0 (0)
, m_uiNumberOfRefPicListModificationsL1 (0)
{
  ::memset( m_ListIdcL0, 0, sizeof(m_ListIdcL0) );
  ::memset( m_RefPicSetIdxL0, 0, sizeof(m_RefPicSetIdxL0) );
  ::memset( m_ListIdcL1, 0, sizeof(m_ListIdcL1) );
  ::memset( m_RefPicSetIdxL1, 0, sizeof(m_RefPicSetIdxL1) );
}

TComRefPicListModification::~TComRefPicListModification()
{
}

TComAPS::TComAPS()
{
  m_apsID = 0;
  m_bAlfEnabled = false;
  m_bSaoEnabled = false;
  m_pSaoParam = NULL;
  m_pAlfParam = NULL;
#if !PARAMSET_VLC_CLEANUP
  m_bCABACForAPS = false;
  m_CABACinitIDC = -1;
  m_CABACinitQP = -1;
#endif
  m_scalingList = NULL;
  m_scalingListEnabled = false;
}

TComAPS::~TComAPS()
{

}

TComAPS& TComAPS::operator= (const TComAPS& src)
{
  m_apsID       = src.m_apsID;
  m_loopFilterOffsetInAPS = src.m_loopFilterOffsetInAPS;
  m_loopFilterDisable = src.m_loopFilterDisable;
  m_loopFilterBetaOffsetDiv2 = src.m_loopFilterBetaOffsetDiv2;
  m_loopFilterTcOffsetDiv2 = src.m_loopFilterTcOffsetDiv2;
  m_bAlfEnabled = src.m_bAlfEnabled;
  m_bSaoEnabled = src.m_bSaoEnabled;
  m_pSaoParam   = src.m_pSaoParam; 
  m_pAlfParam   = src.m_pAlfParam; 
#if !PARAMSET_VLC_CLEANUP
  m_bCABACForAPS= src.m_bCABACForAPS;
  m_CABACinitIDC= src.m_CABACinitIDC;
  m_CABACinitQP = src.m_CABACinitQP;
#endif
  m_scalingList = src.m_scalingList;
  m_scalingListEnabled = src.m_scalingListEnabled;

  return *this;
}

Void TComAPS::createSaoParam()
{
  m_pSaoParam = new SAOParam;
}

Void TComAPS::destroySaoParam()
{
  if(m_pSaoParam != NULL)
  {
    delete m_pSaoParam;
    m_pSaoParam = NULL;
  }
}

Void TComAPS::createAlfParam()
{
  m_pAlfParam = new ALFParam;
}
Void TComAPS::destroyAlfParam()
{
  if(m_pAlfParam != NULL)
  {
    delete m_pAlfParam;
    m_pAlfParam = NULL;
  }
}

Void TComAPS::createScalingList()
{
  m_scalingList = new TComScalingList;
}
Void TComAPS::destroyScalingList()
{
  delete m_scalingList;
}

TComScalingList::TComScalingList()
{
  init();
}
TComScalingList::~TComScalingList()
{
  destroy();
}

/** set default quantization matrix to array
*/
Void TComSlice::setDefaultScalingList()
{
  UInt i;
  Int *dst=0;
  Int *src=0;

  //4x4
  for(i=0;i<SCALING_LIST_NUM;i++)
  {
    src = (i<3) ? g_quantIntraDefault4x4 : g_quantInterDefault4x4;
    dst = getScalingList()->getScalingListOrgAddress(SCALING_LIST_4x4,i);
    ::memcpy(dst,src,sizeof(UInt)*16);
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_4x4,i);
    ::memcpy(dst,src,sizeof(UInt)*16);
  }
  //8x8
  for(i=0;i<SCALING_LIST_NUM;i++)
  {
    src = (i<3) ? g_quantIntraDefault8x8 : g_quantInterDefault8x8;
    dst = getScalingList()->getScalingListOrgAddress(SCALING_LIST_8x8,i);
    ::memcpy(dst,src,sizeof(UInt)*64);
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_8x8,i);
    ::memcpy(dst,src,sizeof(UInt)*64);
  }
  //16x16
  for(i=0;i<SCALING_LIST_NUM;i++)
  {
    src = (i<3) ? g_quantIntraDefault16x16 : g_quantInterDefault16x16;
    dst = getScalingList()->getScalingListOrgAddress(SCALING_LIST_16x16,i);
    ::memcpy(dst,src,sizeof(UInt)*256);
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_16x16,i);
    ::memcpy(dst,src,sizeof(UInt)*256);
  }
  //32x32
  for(i=0;i<SCALING_LIST_NUM_32x32;i++)
  {
    src = (i<1) ? g_quantIntraDefault32x32 : g_quantInterDefault32x32;
    dst = getScalingList()->getScalingListOrgAddress(SCALING_LIST_32x32,i);
    ::memcpy(dst,src,sizeof(UInt)*1024);
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_32x32,i);
    ::memcpy(dst,src,sizeof(UInt)*1024);
  }
}
/** check if use default quantization matrix
 * \returns true if use default quantization matrix in all size
*/
Bool TComSlice::checkDefaultScalingList()
{
  UInt i;
  Int *dst=0;
  Int *src=0;
  UInt defaultCounter=0;

  //4x4
  for(i=0;i<SCALING_LIST_NUM;i++)
  {
    src = (i<3) ? g_quantIntraDefault4x4 : g_quantInterDefault4x4;
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_4x4,i);
    if(::memcmp(dst,src,sizeof(UInt)*16) == 0) defaultCounter++;
  }

  //8x8
  for(i=0;i<SCALING_LIST_NUM;i++)
  {
    src = (i<3) ? g_quantIntraDefault8x8 : g_quantInterDefault8x8;
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_8x8,i);
    if(::memcmp(dst,src,sizeof(UInt)*64) == 0) defaultCounter++;
  }
  //16x16
  for(i=0;i<SCALING_LIST_NUM;i++)
  {
    src = (i<3) ? g_quantIntraDefault16x16 : g_quantInterDefault16x16;
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_16x16,i);
    if(::memcmp(dst,src,sizeof(UInt)*256) == 0) defaultCounter++;
  }
  //32x32
  for(i=0;i<SCALING_LIST_NUM_32x32;i++)
  {
    src = (i<1) ? g_quantIntraDefault32x32 : g_quantInterDefault32x32;
    dst = getScalingList()->getScalingListAddress(SCALING_LIST_32x32,i*3);
    if(::memcmp(dst,src,sizeof(UInt)*1024) == 0) defaultCounter++;
  }
  return (defaultCounter == (SCALING_LIST_NUM * SCALING_LIST_SIZE_NUM - 4)) ? true : false; // -4 for 32x32
}

/** get address of quantization matrix 
 * \param sizeIdc side index
 * \param listId list index
 * \returns pointer of quantization matrix
 */
Int* TComScalingList::getScalingListAddress(UInt sizeIdc, UInt listId)
{
  Int *src = 0;
  switch(sizeIdc)
  {
    case SCALING_LIST_4x4:
      src = m_scalingList4x4[listId];
      break;
    case SCALING_LIST_8x8:
      src = m_scalingList8x8[listId];
      break;
    case SCALING_LIST_16x16:
      src = m_scalingList16x16[listId];
      break;
    case SCALING_LIST_32x32:
      src = m_scalingList32x32[listId];
      break;
    default:
      assert(0);
      src = NULL;
      break;
  }
  return src;
}
/** get address of default quantization matrix 
 * \param sizeIdc size index
 * \param listId list index
 * \returns pointer of default quantization matrix
 */
Int* TComScalingList::getScalingListOrgAddress( UInt sizeIdc, UInt listId)
{
  Int *src = 0;
  switch(sizeIdc)
  {
    case SCALING_LIST_4x4:
      src = m_scalingList4x4_Org[listId];
      break;
    case SCALING_LIST_8x8:
      src = m_scalingList8x8_Org[listId];
      break;
    case SCALING_LIST_16x16:
      src = m_scalingList16x16_Org[listId];
      break;
    case SCALING_LIST_32x32:
      src = m_scalingList32x32_Org[listId];
      break;
    default:
      assert(0);
      src = NULL;
      break;
  }
  return src;
}
/** generate reference matrix for prediction
 * \param pcScalingListsrc parameter set of original matrix 
 * \param piDst reference matrix
 * \param uiDstSizeId size index of reference matrix
 * \param uiDstListId list index of reference matrix
 * \param uiSrcSizeIdc size index of original matrix
 * \param uiSrcMatrixId list index of original matrix
 */
Void TComScalingList::xPredScalingListMatrix( TComScalingList* scalingListsrc, Int* dst, UInt dstSizeId, UInt dstListId, UInt srcSizeIdc, UInt srcMatrixId)
{
  Int *src=0;
  UInt srcSize = g_scalingListSizeX[srcSizeIdc];

  src = scalingListsrc->getScalingListAddress(srcSizeIdc, srcMatrixId);

  ::memcpy(dst,src,sizeof(Int)*srcSize*srcSize);
}
/** mode decision of quantization matrix prediction
 */
Void TComScalingList::xScalingListMatrixModeDecision()
{
  Int sizeIdc,listIdc;
  Int *org = 0;
  Int *recon = 0;
  Int *bestRecon = new Int[1024];
  UInt bestBit;

  for(sizeIdc = 0; sizeIdc < SCALING_LIST_SIZE_NUM; sizeIdc++)
  {
    for(listIdc = 0; listIdc < g_auiScalingListNum[sizeIdc]; listIdc++)
    {
      org   = getScalingListOrgAddress(sizeIdc,listIdc );
      recon = getScalingListAddress(sizeIdc, listIdc);
      bestBit = MAX_UINT;
      xCalcBestBitCopyMode( org, recon, bestRecon, sizeIdc, listIdc, g_scalingListSize[sizeIdc], &bestBit);
      xCalcBestBitDPCMMode( org, recon, bestRecon, sizeIdc, listIdc, g_scalingListSize[sizeIdc], &bestBit);
      ::memcpy(recon,bestRecon,sizeof(Int)*g_scalingListSize[sizeIdc]);
    }
  }
  delete [] bestRecon;
}
/** calculate residual coefficients between original matrix and reconstructed matrix, and sad
 * \param piOrg pointer of original matrix
 * \param piRecon pointer of reconstructed matrix
 * \param piResidual pointer of residual coefficients between original matrix and reconstructed matrix
 * \param sizeIdc size index
 * \returns sad between original matirix and reconstructed matrix
 */
UInt TComScalingList::xMakeResidual(Int *piOrg,Int *recon, Int *residual, UInt sizeIdc)
{
  UInt i,size = g_scalingListSize[sizeIdc];
  UInt* scan  = g_auiFrameScanXY [ sizeIdc + 1 ];
  UInt uiSAD = 0;
  for(i=0;i<size;i++)
  {
    residual[scan[i]] = piOrg[scan[i]] - recon[scan[i]];
    uiSAD += abs(residual[scan[i]]);
  }
  return uiSAD;
}
/** change zigzag scan array into raster scan array
 * \param piSrc  zigzag scan array
 * \param piDist raster scan array
 * \param sizeIdc size index
 */
Void TComScalingList::xInvZigZag(Int *src,Int *dist, UInt sizeIdc)
{
  UInt i,size = g_scalingListSize[sizeIdc];
  UInt* scan  = g_auiFrameScanXY [ sizeIdc + 1 ];
  Int pcmCounter = 0;
  for(i=0;i<size;i++)
  {
    dist[scan[i]]  = src[pcmCounter];
    pcmCounter++;
  }
}
/** apply inverse DPCM to input matrix coefficient
 * \param piSrc  input matrix
 * \param piDist output matrix
 * \param sizeIdc size index
 * \param iStartValue start value of DPCM
 */
Void TComScalingList::xInvDPCM(Int *src,Int *dist, UInt sizeIdc, Int startValue)
{
  UInt i,size = g_scalingListSize[sizeIdc];
  Int  lastScale = startValue;
  for(i=0;i<size;i++)
  {
    dist[i] = (lastScale + src[i] + 256 ) % 256;
    lastScale = dist[i];
  }
}
/** calculate sum of syntax bits for matrix by applying copy mode and update best bit
 * \param piOrg original matrix
 * \param piRecon reconstructed matrix
 * \param piBestRecon best reconstructed matrix
 * \param iSizeIdc size index
 * \param iListIdc list index
 * \param uiSize number of matrix coefficients
 * \param uiBestBit best of sum of syntax bits for matrix
 */

Void TComScalingList::xCalcBestBitCopyMode( Int *org, Int *recon, Int * bestRecon, Int sizeIdc, Int listIdc, UInt uiSize, UInt *bestBit)
{
  Int *residual = new Int[1024];
  UInt sad=0;
  estScalingListStruct estScalingList;
  estScalingList.predMode = SCALING_LIST_PRED_COPY;
  for(estScalingList.predListIdx = listIdc -1 ; estScalingList.predListIdx >= 0; estScalingList.predListIdx--)
  {
    xPredScalingListMatrix(this, recon, sizeIdc, listIdc, sizeIdc, estScalingList.predListIdx);
    sad = xCalcResidual(org, recon, residual, sizeIdc, &estScalingList);
    if(sad == 0 && *bestBit != 0)
    {
      *bestBit = 0;
      xUpdateCondition(sizeIdc, listIdc, &estScalingList);
      ::memcpy(bestRecon,recon,sizeof(Int)*uiSize);
    }
  }
  delete [] residual;
}
/** calculate sum of syntax bits for matrix by applying DPCM mode and update best bit
 * \param piOrg original matrix
 * \param piRecon reconstructed matrix
 * \param piBestRecon best reconstructed matrix
 * \param iSizeIdc size index
 * \param iListIdc list index
 * \param uiSize number of matrix coefficients
 * \param uiBestBit best of sum of syntax bits for matrix
 */
Void TComScalingList::xCalcBestBitDPCMMode( Int *org, Int * recon, Int * bestRecon, Int sizeIdc, Int listIdc, UInt size, UInt *bestBit)
{
  UInt targetBit=0;
  estScalingListStruct estScalingList;

  estScalingList.predMode = SCALING_LIST_PRED_DPCM;
  targetBit = xPredDPCMScalingListMatrix(recon, org, sizeIdc, &estScalingList);

  if(targetBit < *bestBit)
  {
    *bestBit = targetBit;
    xUpdateCondition(sizeIdc, listIdc, &estScalingList);
    ::memcpy(bestRecon,recon,sizeof(Int)*size);
  }
}
/** calculate residual coefficients between original matrix and reconstructed matrix and decide coding mode of residual coefficients
 * \param piOrg original matrix
 * \param piRecon reconstructed matrix
 * \param piResidual residual matrix
 * \param sizeIdc size index
 * \param pestScalingList pointer of parameter sets of best coding mode
 * \returns best of sum of syntax bits
 */
UInt TComScalingList::xCalcResidual(Int *org, Int *recon, Int *residual, UInt sizeIdc, estScalingListStruct *pestScalingList)
{
  UInt sad = MAX_UINT;
  sad = xMakeResidual(org,recon,residual,sizeIdc);
  return sad;
}

/** apply DPCM to input matrix coefficient with quantization
 * \param piSrc input matrix
 * \param piDist output matrix
 * \param dpcm result of DPCM
 * \param sizeIdc size index
 */
Void TComScalingList::xMakeDPCM(Int* src, Int* dst, Int* dpcm, UInt sizeId)
{
  Int startValue = SCALING_LIST_START_VALUE;
  UInt i,size = g_scalingListSize[sizeId];
  UInt* scan    = g_auiFrameScanXY [ sizeId + 1 ];
  Int  lastScale = startValue;
  UInt dataCounter = 0;

  for(i=0;i<size;i++)
  {
    dpcm[dataCounter] = src[scan[i]] - lastScale;
    dst[scan[i]] = lastScale + dpcm[dataCounter];
    lastScale = dst[scan[i]];
    if(dpcm[dataCounter] > 127)
      dpcm[dataCounter]=dpcm[dataCounter]-256;
    if(dpcm[dataCounter] < -128)
      dpcm[dataCounter]=dpcm[dataCounter]+256;

    dataCounter++;
  }
}
/** estimate DPCM with ScalingListMatrix
 * \param piDst quantized matrix
 * \param piOrg original matrix
 * \param uiSizeId size index
 * \param pestScalingList pointer of parameter sets of best coding mode
 * \returns sum of syntax bits for matrix
 */
UInt TComScalingList::xPredDPCMScalingListMatrix(Int* dst, Int* org, UInt sizeId, estScalingListStruct *pestScalingList)
{
  Int dpcm[1024];

  xMakeDPCM(org, dst, dpcm, sizeId);
  return 1;
}

/** parse syntax infomation 
 *  \param pchFile syntax infomation
 *  \returns false if successful
 */
Bool TComScalingList::xParseScalingList(char* pchFile)
{
  FILE *fp;
  Char line[1024];
  UInt sizeIdc,listIdc;
  UInt i,size = 0;
  Int *src=0,data;
  Char *ret;
  UInt  retval;

  if((fp = fopen(pchFile,"r")) == (FILE*)NULL)
  {
    printf("can't open file %s :: set Default Matrix\n",pchFile);
    return true;
  }

  for(sizeIdc = 0; sizeIdc < SCALING_LIST_SIZE_NUM; sizeIdc++)
  {
    size = g_scalingListSize[sizeIdc];
    for(listIdc = 0; listIdc < g_auiScalingListNum[sizeIdc]; listIdc++)
    {
      src = getScalingListOrgAddress(sizeIdc, listIdc);

      fseek(fp,0,0);
      do 
      {
        ret = fgets(line, 1024, fp);
        if ((ret==NULL)||(strstr(line, MatrixType[sizeIdc][listIdc])==NULL && feof(fp)))
        {
          printf("Error: can't read Matrix :: set Default Matrix\n");
          return true;
        }
      }
      while (strstr(line, MatrixType[sizeIdc][listIdc]) == NULL);
      for (i=0; i<size; i++)
      {
        retval = fscanf(fp, "%d,", &data);
        if (retval!=1)
        {
          printf("Error: can't read Matrix :: set Default Matrix\n");
          return true;
        }
        src[i] = data;
      }
    }
  }
  fclose(fp);
  return false;
}

/** update condition
 * \param iSizeIdc size index
 * \param iListIdc list index
 * \param pestScalingList quantization matrix infomation
 */
Void TComScalingList::xUpdateCondition(UInt sizeIdc, UInt listIdc, estScalingListStruct *pestScalingList)
{
  setPredMatrixId(sizeIdc, listIdc, pestScalingList->predListIdx);
  setPredMode    (sizeIdc, listIdc, pestScalingList->predMode);
}
/** initialization process of quantization matrix array
 */
Void TComScalingList::init()
{
  for(UInt listId = 0; listId < SCALING_LIST_NUM; listId++)
  {
    m_scalingList4x4      [listId] = new Int [16];
    m_scalingList4x4_Org  [listId] = new Int [16];
    m_scalingList8x8      [listId] = new Int [64];
    m_scalingList8x8_Org  [listId] = new Int [64];
    m_scalingList16x16    [listId] = new Int [256];
    m_scalingList16x16_Org[listId] = new Int [256];
  }
  for(UInt listId = 0; listId < SCALING_LIST_NUM_32x32; listId++)
  {
    m_scalingList32x32    [listId] = new Int [1024];
    m_scalingList32x32_Org[listId] = new Int [1024];
  }
  m_scalingList32x32    [3] = m_scalingList32x32    [1]; // copy address for 32x32
  m_scalingList32x32_Org[3] = m_scalingList32x32_Org[1]; // copy address for 32x32 

}
/** destroy quantization matrix array
 */
Void TComScalingList::destroy()
{
  for(UInt listId = 0; listId < SCALING_LIST_NUM; listId++)
  {
    if(m_scalingList4x4      [listId]) delete [] m_scalingList4x4      [listId];
    if(m_scalingList4x4_Org  [listId]) delete [] m_scalingList4x4_Org  [listId];
    if(m_scalingList8x8      [listId]) delete [] m_scalingList8x8      [listId];
    if(m_scalingList8x8_Org  [listId]) delete [] m_scalingList8x8_Org  [listId];
    if(m_scalingList16x16    [listId]) delete [] m_scalingList16x16    [listId];
    if(m_scalingList16x16_Org[listId]) delete [] m_scalingList16x16_Org[listId];
  }
  for(UInt listId = 0; listId < SCALING_LIST_NUM_32x32; listId++)
  {
    if(m_scalingList32x32    [listId]) delete [] m_scalingList32x32    [listId];
    if(m_scalingList32x32_Org[listId]) delete [] m_scalingList32x32_Org[listId];
  }
}

#if PARAMSET_VLC_CLEANUP

ParameterSetManager::ParameterSetManager()
: m_spsMap(MAX_NUM_SPS)
, m_ppsMap(MAX_NUM_PPS)
, m_apsMap(MAX_NUM_APS)
{
}


ParameterSetManager::~ParameterSetManager()
{
}

#endif

//! \}
