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

/** \file     TDecEntropy.cpp
    \brief    entropy decoder class
*/

#include "TDecEntropy.h"

//! \ingroup TLibDecoder
//! \{

Void TDecEntropy::setEntropyDecoder         ( TDecEntropyIf* p )
{
  m_pcEntropyDecoderIf = p;
}

#include "TLibCommon/TComAdaptiveLoopFilter.h"
#include "TLibCommon/TComSampleAdaptiveOffset.h"

#if !PARAMSET_VLC_CLEANUP
Void TDecEntropy::decodeAux(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  Int sqrFiltLengthTab[2] = { 9, 9}; 

  pAlfParam->filters_per_group = 0;
  
  memset (pAlfParam->filterPattern, 0 , sizeof(Int)*NO_VAR_BINS);
  m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
  pAlfParam->alf_pcr_region_flag = uiSymbol;  

  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);

  pAlfParam->filter_shape = uiSymbol;
  pAlfParam->num_coeff = sqrFiltLengthTab[pAlfParam->filter_shape];
  // filters_per_fr
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->filters_per_group = uiSymbol + 1;

  if(uiSymbol == 1) // filters_per_group == 2
  {
    m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
    pAlfParam->startSecondFilter = uiSymbol;
    pAlfParam->filterPattern [uiSymbol] = 1;
  }
  else if (uiSymbol > 1) // filters_per_group > 2
  {
    pAlfParam->filters_per_group = 1;
    Int numMergeFlags = pAlfParam->alf_pcr_region_flag ? 16 : 15;
    for (Int i=1; i<numMergeFlags; i++) 
    {
      m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
      pAlfParam->filterPattern[i] = uiSymbol;
      pAlfParam->filters_per_group += uiSymbol;
    }
  }
}
#endif

#if !PARAMSET_VLC_CLEANUP
Void TDecEntropy::readFilterCodingParams(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  int ind, scanPos;
  int golombIndexBit;
  int kMin;
  int maxScanVal;
  int *pDepthInt;
  int minScanVal = (pAlfParam->filter_shape == ALF_STAR5x5) ? 0: MIN_SCAN_POS_CROSS;
  // Determine maxScanVal
  maxScanVal = 0;
  pDepthInt = pDepthIntTabShapes[pAlfParam->filter_shape];
  for(ind = 0; ind < pAlfParam->num_coeff; ind++)
  {
    maxScanVal = max(maxScanVal, pDepthInt[ind]);
  }
  
  // Golomb parameters
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->minKStart = 1 + uiSymbol;
  
  kMin = pAlfParam->minKStart;

  for(scanPos = minScanVal; scanPos < maxScanVal; scanPos++)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    golombIndexBit = uiSymbol;
    if(golombIndexBit)
    {
      pAlfParam->kMinTab[scanPos] = kMin + 1;
    }
    else
    {
      pAlfParam->kMinTab[scanPos] = kMin;
    }
    kMin = pAlfParam->kMinTab[scanPos];
  }
}

Int TDecEntropy::golombDecode(Int k)
{
  UInt uiSymbol;
  Int q = -1;
  Int nr = 0;
  Int a;
  
  uiSymbol = 1;
  while (uiSymbol)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    q++;
  }
  for(a = 0; a < k; ++a)          // read out the sequential log2(M) bits
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    if(uiSymbol)
    {
      nr += 1 << a;
    }
  }
  nr += q << k;
  if(nr != 0)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    nr = (uiSymbol)? nr: -nr;
  }
  return nr;
}



Void TDecEntropy::readFilterCoeffs(ALFParam* pAlfParam)
{
  int ind, scanPos, i;
  int *pDepthInt;
  pDepthInt = pDepthIntTabShapes[pAlfParam->filter_shape];
  for(ind = 0; ind < pAlfParam->filters_per_group; ++ind)
  {
    for(i = 0; i < pAlfParam->num_coeff; i++)
    {
      scanPos = pDepthInt[i] - 1;
      pAlfParam->coeffmulti[ind][i] = golombDecode(pAlfParam->kMinTab[scanPos]);
    }
  }
}

Void TDecEntropy::decodeFilterCoeff (ALFParam* pAlfParam)
{
  readFilterCodingParams (pAlfParam);
  readFilterCoeffs (pAlfParam);
}



Void TDecEntropy::decodeFilt(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  if (pAlfParam->filters_per_group > 1)
  {
    m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
    pAlfParam->predMethod = uiSymbol;
  }
  for(Int ind = 0; ind < pAlfParam->filters_per_group; ++ind)
  {
    m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
    pAlfParam->nbSPred[ind] = uiSymbol;
  }
  decodeFilterCoeff (pAlfParam);
}

Void TDecEntropy::decodeAlfParam(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  Int iSymbol;
  if (!pAlfParam->alf_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
    return;
  }
  
  Int pos;
  decodeAux(pAlfParam);
  decodeFilt(pAlfParam);
  // filter parameters for chroma
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->chroma_idc = uiSymbol;
  
  if(pAlfParam->chroma_idc)
  {
    m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
    Int sqrFiltLengthTab[2] = { 9, 9};
    pAlfParam->filter_shape_chroma = uiSymbol;
    pAlfParam->num_coeff_chroma = sqrFiltLengthTab[pAlfParam->filter_shape_chroma];
    // filter coefficients for chroma
    for(pos=0; pos<pAlfParam->num_coeff_chroma; pos++)
    {
      m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);
      pAlfParam->coeff_chroma[pos] = iSymbol;
    }
  }
}

/** decode ALF CU control parameters for one slice
 * \param [in,out] cAlfParam ALF CU control parameters
 * \param [in] iNumCUsinPic number of LCUs in picture for ALF
 */

Void TDecEntropy::decodeAlfCtrlParam(AlfCUCtrlInfo& cAlfParam, Int iNumCUsInPic)
{
  UInt uiSymbol;
  Int iSymbol;

  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  cAlfParam.cu_control_flag = uiSymbol;
  if (cAlfParam.cu_control_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(true);
    m_pcEntropyDecoderIf->parseAlfCtrlDepth(uiSymbol);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(uiSymbol);
    cAlfParam.alf_max_depth = uiSymbol;
  }
  else
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    return;
  }

  m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);
  cAlfParam.num_alf_cu_flag = (UInt)(iSymbol + iNumCUsInPic);
  cAlfParam.alf_cu_flag.resize(cAlfParam.num_alf_cu_flag);

  for(UInt i=0; i< cAlfParam.num_alf_cu_flag; i++)
  {
    m_pcEntropyDecoderIf->parseAlfCtrlFlag( cAlfParam.alf_cu_flag[i] );
  }
}
#endif

Void TDecEntropy::decodeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSkipFlag( pcCU, uiAbsPartIdx, uiDepth );
}

/** decode merge flag
 * \param pcSubCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPUIdx 
 * \returns Void
 */
Void TDecEntropy::decodeMergeFlag( TComDataCU* pcSubCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPUIdx )
{ 
  // at least one merge candidate exists
  m_pcEntropyDecoderIf->parseMergeFlag( pcSubCU, uiAbsPartIdx, uiDepth, uiPUIdx );
}

/** decode merge index
 * \param pcCU
 * \param uiPartIdx 
 * \param uiAbsPartIdx 
 * \param puhInterDirNeighbours pointer to list of inter direction from the casual neighbours
 * \param pcMvFieldNeighbours pointer to list of motion vector field from the casual neighbours
 * \param uiDepth
 * \returns Void
 */
Void TDecEntropy::decodeMergeIndex( TComDataCU* pcCU, UInt uiPartIdx, UInt uiAbsPartIdx, PartSize eCUMode, UChar* puhInterDirNeighbours, TComMvField* pcMvFieldNeighbours, UInt uiDepth )
{
  UInt uiMergeIndex = 0;
  m_pcEntropyDecoderIf->parseMergeIndex( pcCU, uiMergeIndex, uiAbsPartIdx, uiDepth );
  pcCU->setMergeIndexSubParts( uiMergeIndex, uiAbsPartIdx, uiPartIdx, uiDepth );
}

Void TDecEntropy::decodeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseSplitFlag( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePredMode( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parsePartSize( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodePredInfo    ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  PartSize eMode = pcCU->getPartitionSize( uiAbsPartIdx );
  
  if( pcCU->isIntra( uiAbsPartIdx ) )                                 // If it is Intra mode, encode intra prediction mode.
  {
    if( eMode == SIZE_NxN )                                         // if it is NxN size, encode 4 intra directions.
    {
      UInt uiPartOffset = ( pcCU->getPic()->getNumPartInCU() >> ( pcCU->getDepth(uiAbsPartIdx) << 1 ) ) >> 2;
      // if it is NxN size, this size might be the smallest partition size.                                                         // if it is NxN size, this size might be the smallest partition size.
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx,                  uiDepth+1 );
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset,   uiDepth+1 );
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*2, uiDepth+1 );
      decodeIntraDirModeLuma( pcCU, uiAbsPartIdx + uiPartOffset*3, uiDepth+1 );
      decodeIntraDirModeChroma( pcCU, uiAbsPartIdx, uiDepth );
    }
    else                                                                // if it is not NxN size, encode 1 intra directions
    {
      decodeIntraDirModeLuma  ( pcCU, uiAbsPartIdx, uiDepth );
      decodeIntraDirModeChroma( pcCU, uiAbsPartIdx, uiDepth );
    }
  }
  else                                                                // if it is Inter mode, encode motion vector and reference index
  {
    decodePUWise( pcCU, uiAbsPartIdx, uiDepth, pcSubCU );
  }
}

/** Parse I_PCM information. 
 * \param pcCU  pointer to CUpointer to CU
 * \param uiAbsPartIdx CU index
 * \param uiDepth CU depth
 * \returns Void
 */
Void TDecEntropy::decodeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if(!pcCU->getSlice()->getSPS()->getUsePCM()
    || pcCU->getWidth(uiAbsPartIdx) > (1<<pcCU->getSlice()->getSPS()->getPCMLog2MaxSize())
    || pcCU->getWidth(uiAbsPartIdx) < (1<<pcCU->getSlice()->getSPS()->getPCMLog2MinSize()) )
  {
    return;
  }
  
  m_pcEntropyDecoderIf->parseIPCMInfo( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeIntraDirModeLuma  ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirLumaAng( pcCU, uiAbsPartIdx, uiDepth );
}

Void TDecEntropy::decodeIntraDirModeChroma( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseIntraDirChroma( pcCU, uiAbsPartIdx, uiDepth );
}

/** decode motion information for every PU block.
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param pcSubCU
 * \returns Void
 */
Void TDecEntropy::decodePUWise( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, TComDataCU* pcSubCU )
{
  PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
  UInt uiNumPU = ( ePartSize == SIZE_2Nx2N ? 1 : ( ePartSize == SIZE_NxN ? 4 : 2 ) );
  UInt uiPUOffset = ( g_auiPUOffset[UInt( ePartSize )] << ( ( pcCU->getSlice()->getSPS()->getMaxCUDepth() - uiDepth ) << 1 ) ) >> 4;

  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_0 );
  pcSubCU->copyInterPredInfoFrom( pcCU, uiAbsPartIdx, REF_PIC_LIST_1 );
  for ( UInt uiPartIdx = 0, uiSubPartIdx = uiAbsPartIdx; uiPartIdx < uiNumPU; uiPartIdx++, uiSubPartIdx += uiPUOffset )
  {
    TComMvField cMvFieldNeighbours[MRG_MAX_NUM_CANDS << 1]; // double length for mv of both lists
    UChar uhInterDirNeighbours[MRG_MAX_NUM_CANDS];
    Int numValidMergeCand = 0;
    for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ++ui )
    {
      uhInterDirNeighbours[ui] = 0;
    }
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
      decodeMergeFlag( pcCU, uiSubPartIdx, uiDepth, uiPartIdx );
    }
    if ( pcCU->getMergeFlag( uiSubPartIdx ) )
    {
      decodeMergeIndex( pcCU, uiPartIdx, uiSubPartIdx, ePartSize, uhInterDirNeighbours, cMvFieldNeighbours, uiDepth );
      pcSubCU->getInterMergeCandidates( uiSubPartIdx-uiAbsPartIdx, uiPartIdx, uiDepth, cMvFieldNeighbours, uhInterDirNeighbours, numValidMergeCand );
      UInt uiMergeIndex = pcCU->getMergeIndex(uiSubPartIdx);
      pcCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeIndex], uiSubPartIdx, uiPartIdx, uiDepth );

      TComMv cTmpMv( 0, 0 );
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0  ) //if ( ref. frame list0 has at least 1 entry )
      {
        pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_0, uiSubPartIdx, uiPartIdx, uiDepth);
        pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_0, uiSubPartIdx, uiPartIdx, uiDepth);
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cTmpMv, ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ], ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
      }
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
      {
        pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_1, uiSubPartIdx, uiPartIdx, uiDepth);
        pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_1, uiSubPartIdx, uiPartIdx, uiDepth);
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cTmpMv, ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ], ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
      }
    }
    else
    {
      decodeInterDirPU( pcCU, uiSubPartIdx, uiDepth, uiPartIdx );
      for ( UInt uiRefListIdx = 0; uiRefListIdx < 2; uiRefListIdx++ )
      {        
        if ( pcCU->getSlice()->getNumRefIdx( RefPicList( uiRefListIdx ) ) > 0 )
        {
          decodeRefFrmIdxPU( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
          decodeMvdPU      ( pcCU,    uiSubPartIdx,              uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
          decodeMVPIdxPU   ( pcSubCU, uiSubPartIdx-uiAbsPartIdx, uiDepth, uiPartIdx, RefPicList( uiRefListIdx ) );
        }
      }
    }
  }
  return;
}

/** decode inter direction for a PU block
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPartIdx 
 * \returns Void
 */
Void TDecEntropy::decodeInterDirPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx )
{
  UInt uiInterDir;

  if ( pcCU->getSlice()->isInterP() )
  {
    uiInterDir = 1;
  }
  else
  {
    m_pcEntropyDecoderIf->parseInterDir( pcCU, uiInterDir, uiAbsPartIdx, uiDepth );
  }

  pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiPartIdx, uiDepth );
}

Void TDecEntropy::decodeRefFrmIdxPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  if(pcCU->getSlice()->getNumRefIdx(REF_PIC_LIST_C) > 0 && pcCU->getInterDir( uiAbsPartIdx ) != 3)
  {
    if(eRefList == REF_PIC_LIST_1)
    {
      return;
    }

    Int iRefFrmIdx = 0;
    Int iRefFrmIdxTemp;
    UInt uiInterDir;
    RefPicList eRefListTemp;

    PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );

    if ( pcCU->getSlice()->getNumRefIdx ( REF_PIC_LIST_C ) > 1 )
    {
      m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, REF_PIC_LIST_C );
    }
    else
    {
      iRefFrmIdx=0;
    }
    uiInterDir = pcCU->getSlice()->getListIdFromIdxOfLC(iRefFrmIdx) + 1;
    iRefFrmIdxTemp = pcCU->getSlice()->getRefIdxFromIdxOfLC(iRefFrmIdx);
    eRefListTemp = (RefPicList)pcCU->getSlice()->getListIdFromIdxOfLC(iRefFrmIdx);

    pcCU->getCUMvField( eRefListTemp )->setAllRefIdx( iRefFrmIdxTemp, ePartSize, uiAbsPartIdx, uiDepth, uiPartIdx );

    pcCU->setInterDirSubParts( uiInterDir, uiAbsPartIdx, uiPartIdx, uiDepth );
  }
  else
  {
    Int iRefFrmIdx = 0;
    Int iParseRefFrmIdx = pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList );

    if ( pcCU->getSlice()->getNumRefIdx( eRefList ) > 1 && iParseRefFrmIdx )
    {
      m_pcEntropyDecoderIf->parseRefFrmIdx( pcCU, iRefFrmIdx, uiAbsPartIdx, uiDepth, eRefList );
    }
    else if ( !iParseRefFrmIdx )
    {
      iRefFrmIdx = NOT_VALID;
    }
    else
    {
      iRefFrmIdx = 0;
    }

    PartSize ePartSize = pcCU->getPartitionSize( uiAbsPartIdx );
    pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, ePartSize, uiAbsPartIdx, uiDepth, uiPartIdx );
  }
}

/** decode motion vector difference for a PU block
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiPartIdx
 * \param eRefList 
 * \returns Void
 */
Void TDecEntropy::decodeMvdPU( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  if ( pcCU->getInterDir( uiAbsPartIdx ) & ( 1 << eRefList ) )
  {
    m_pcEntropyDecoderIf->parseMvd( pcCU, uiAbsPartIdx, uiPartIdx, uiDepth, eRefList );
  }
}

Void TDecEntropy::decodeMVPIdxPU( TComDataCU* pcSubCU, UInt uiPartAddr, UInt uiDepth, UInt uiPartIdx, RefPicList eRefList )
{
  Int iMVPIdx = -1;

  TComMv cZeroMv( 0, 0 );
  TComMv cMv     = cZeroMv;
  Int    iRefIdx = -1;

  TComCUMvField* pcSubCUMvField = pcSubCU->getCUMvField( eRefList );
  AMVPInfo* pAMVPInfo = pcSubCUMvField->getAMVPInfo();

  iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
  cMv = cZeroMv;

  if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    m_pcEntropyDecoderIf->parseMVPIdx( iMVPIdx );
  }
  pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
  pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
  pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
  if ( iRefIdx >= 0 )
  {
    m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
    cMv += pcSubCUMvField->getMvd( uiPartAddr );
  }

  PartSize ePartSize = pcSubCU->getPartitionSize( uiPartAddr );
  pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 0, uiPartIdx);
}

Void TDecEntropy::xDecodeTransformSubdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3 )
{
  UInt uiSubdiv;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;

  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    uiSubdiv = 1;
  }
#if G519_TU_AMP_NSQT_HARMONIZATION
  else if( (pcCU->getSlice()->getSPS()->getQuadtreeTUMaxDepthInter() == 1) && (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER) && ( pcCU->getPartitionSize(uiAbsPartIdx) != SIZE_2Nx2N ) && (uiDepth == pcCU->getDepth(uiAbsPartIdx)) )
#else
  else if( (pcCU->getSlice()->getSPS()->getQuadtreeTUMaxDepthInter() == 1) && (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER) && ( pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_Nx2N) && (uiDepth == pcCU->getDepth(uiAbsPartIdx)) )
#endif
  {
    uiSubdiv = (uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx));
  }
  else if( uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    uiSubdiv = 1;
  }
  else if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
  {
    uiSubdiv = 0;
  }
  else if( uiLog2TrafoSize == pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) )
  {
    uiSubdiv = 0;
  }
  else
  {
    assert( uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx) );
    m_pcEntropyDecoderIf->parseTransformSubdivFlag( uiSubdiv, uiDepth );
  }
  
  const UInt uiTrDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
  
  if( uiLog2TrafoSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
  {
    const Bool bFirstCbfOfCU = uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() || uiTrDepth == 0;
    if( bFirstCbfOfCU )
    {
      pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
      pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
    }
    if( bFirstCbfOfCU || uiLog2TrafoSize > 2 )
    {
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) )
      {
        if ( uiInnerQuadIdx == 3 && uiUCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
        {
          uiUCbfFront3++;
          pcCU->setCbfSubParts( 1 << uiTrDepth, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
          //printf( " \nsave bits, U Cbf");
        }
        else
        {
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth, uiDepth );
          uiUCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth );
        }
      }
      if( bFirstCbfOfCU || pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) )
      {
        if ( uiInnerQuadIdx == 3 && uiVCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
        {
          uiVCbfFront3++;
          pcCU->setCbfSubParts( 1 << uiTrDepth, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
          //printf( " \nsave bits, V Cbf");
        }
        else
        {
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth, uiDepth );
          uiVCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth );
        }
      }
    }
    else
    {
      pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
      pcCU->setCbfSubParts( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth - 1 ) << uiTrDepth, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
      if ( uiLog2TrafoSize == 2 )
      {
        uiUCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrDepth );
        uiVCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrDepth );
      }
    }
  }
  
  if( uiSubdiv )
  {
    ++uiDepth;
    const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
    const UInt uiStartAbsPartIdx = uiAbsPartIdx;
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiStartAbsPartIdx, uiTrDepth+1, uiLumaTrMode, uiChromaTrMode );
    UInt uiYCbf = 0;
    UInt uiUCbf = 0;
    UInt uiVCbf = 0;
    
    UInt uiCurrentCbfY = 0;
    UInt uiCurrentCbfU = 0;
    UInt uiCurrentCbfV = 0;
    
    for( Int i = 0; i < 4; i++ )
    {
      xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, i, uiCurrentCbfY, uiCurrentCbfU, uiCurrentCbfV );
      uiYCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
      uiUCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiChromaTrMode );
      uiVCbf |= pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiChromaTrMode );
      uiAbsPartIdx += uiQPartNum;
    }
    
    uiYCbfFront3 += uiCurrentCbfY;
    uiUCbfFront3 += uiCurrentCbfU;
    uiVCbfFront3 += uiCurrentCbfV;
    
    pcCU->convertTransIdx( uiStartAbsPartIdx, uiTrDepth, uiLumaTrMode, uiChromaTrMode );
    for( UInt ui = 0; ui < 4 * uiQPartNum; ++ui )
    {
      pcCU->getCbf( TEXT_LUMA     )[uiStartAbsPartIdx + ui] |= uiYCbf << uiLumaTrMode;
      pcCU->getCbf( TEXT_CHROMA_U )[uiStartAbsPartIdx + ui] |= uiUCbf << uiChromaTrMode;
      pcCU->getCbf( TEXT_CHROMA_V )[uiStartAbsPartIdx + ui] |= uiVCbf << uiChromaTrMode;
    }
  }
  else
  {
    assert( uiDepth >= pcCU->getDepth( uiAbsPartIdx ) );
    pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiDepth );
    
    {
      DTRACE_CABAC_VL( g_nSymbolCounter++ );
      DTRACE_CABAC_T( "\tTrIdx: abspart=" );
      DTRACE_CABAC_V( uiAbsPartIdx );
      DTRACE_CABAC_T( "\tdepth=" );
      DTRACE_CABAC_V( uiDepth );
      DTRACE_CABAC_T( "\ttrdepth=" );
      DTRACE_CABAC_V( uiTrDepth );
      DTRACE_CABAC_T( "\n" );
    }
    
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, uiTrDepth, uiLumaTrMode, uiChromaTrMode );
    pcCU->setCbfSubParts ( 0, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiDepth == pcCU->getDepth( uiAbsPartIdx ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, 0 ) && !pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, 0 ) )
    {
      pcCU->setCbfSubParts( 1 << uiLumaTrMode, TEXT_LUMA, uiAbsPartIdx, uiDepth );
    }
    else
    {
      const UInt uiLog2CUSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()] + 2 - pcCU->getDepth( uiAbsPartIdx );
      if ( pcCU->getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && uiInnerQuadIdx == 3 && uiYCbfFront3 == 0 && uiUCbfFront3 == 0 && uiVCbfFront3 == 0
          && ( uiLog2CUSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() + 1 || uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() ) )
      {
        pcCU->setCbfSubParts( 1 << uiLumaTrMode, TEXT_LUMA, uiAbsPartIdx, uiDepth );
        //printf( " \nsave bits, Y Cbf");
        uiYCbfFront3++;    
      }
      else
      {
        m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode, uiDepth );
        uiYCbfFront3 += pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiLumaTrMode );
      }
    }
    
  }
}

Void TDecEntropy::decodeTransformIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tdecodeTransformIdx()\tCUDepth=" )
  DTRACE_CABAC_V( uiDepth )
  DTRACE_CABAC_T( "\n" )
  UInt temp = 0;
  UInt temp1 = 0;
  UInt temp2 = 0;
  xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, 0, temp, temp1, temp2 );
}

Void TDecEntropy::decodeQP          ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if ( pcCU->getSlice()->getPPS()->getUseDQP() )
  {
    m_pcEntropyDecoderIf->parseDeltaQP( pcCU, uiAbsPartIdx, uiDepth );
  }
}

Void TDecEntropy::xDecodeCoeff( TComDataCU* pcCU, UInt uiLumaOffset, UInt uiChromaOffset, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, Bool& bCodeDQP )
{
  UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
  UInt uiCbfY = pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiTrIdx );
  UInt uiCbfU = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
  UInt uiCbfV = pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );
  if( uiLog2TrSize == 2 )
  {
    UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
    if( ( uiAbsPartIdx % uiQPDiv ) == 0 )
    {
      m_uiBakAbsPartIdx   = uiAbsPartIdx;
      m_uiBakChromaOffset = uiChromaOffset;
    }
    else if( ( uiAbsPartIdx % uiQPDiv ) == (uiQPDiv - 1) )
    {
      uiCbfU = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_U, uiTrIdx );
      uiCbfV = pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_V, uiTrIdx );
    }
  }

  if ( uiCbfY || uiCbfU || uiCbfV )
  {
    // dQP: only for LCU
    if ( pcCU->getSlice()->getPPS()->getUseDQP() )
    {
      if ( bCodeDQP )
      {
        decodeQP( pcCU, uiAbsPartIdx, uiDepth);
        bCodeDQP = false;
      }
    }   
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
    const UInt uiStopTrMode = uiLumaTrMode;
    
    if( uiTrIdx == uiStopTrMode )
    {
      if( pcCU->getCbf( uiAbsPartIdx, TEXT_LUMA, uiTrIdx ) )
      {
        Int trWidth = uiWidth;
        Int trHeight = uiHeight;
        pcCU->getNSQTSize( uiTrIdx, uiAbsPartIdx, trWidth, trHeight );
        m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffY()+uiLumaOffset), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_LUMA );
      }
      
      uiWidth  >>= 1;
      uiHeight >>= 1;

      if( uiLog2TrSize == 2 )
      {
        UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
        if( ( uiAbsPartIdx % uiQPDiv ) == (uiQPDiv - 1) )
        {
          uiWidth  <<= 1;
          uiHeight <<= 1;
          Int trWidth = uiWidth;
          Int trHeight = uiHeight;
          pcCU->getNSQTSize( uiTrIdx-1, uiAbsPartIdx, trWidth, trHeight );
          if( pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_U, uiTrIdx ) )
          {
            m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCb()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
          }
          if( pcCU->getCbf( m_uiBakAbsPartIdx, TEXT_CHROMA_V, uiTrIdx ) )
          {
            m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCr()+m_uiBakChromaOffset), m_uiBakAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
          }
        }
      }
      else
      {
        Int trWidth = uiWidth;
        Int trHeight = uiHeight;
        pcCU->getNSQTSize( uiTrIdx, uiAbsPartIdx, trWidth, trHeight );
        if( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_U, uiTrIdx ) )
        {
          m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCb()+uiChromaOffset), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_U );
        }
        if( pcCU->getCbf( uiAbsPartIdx, TEXT_CHROMA_V, uiTrIdx ) )
        {
          m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, (pcCU->getCoeffCr()+uiChromaOffset), uiAbsPartIdx, trWidth, trHeight, uiDepth, TEXT_CHROMA_V );
        }
      }
    }
    else
    {
      {
        DTRACE_CABAC_VL( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing down\tdepth=" );
        DTRACE_CABAC_V( uiDepth );
        DTRACE_CABAC_T( "\ttridx=" );
        DTRACE_CABAC_V( uiTrIdx );
        DTRACE_CABAC_T( "\n" );
      }
      if( uiCurrTrIdx <= uiTrIdx )
      {
        assert(1);
      }
      UInt uiSize;
      uiWidth  >>= 1;
      uiHeight >>= 1;
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;
      
      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;
      
      xDecodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, bCodeDQP );
      uiLumaOffset += uiSize;  uiChromaOffset += (uiSize>>2);  uiIdx += uiQPartNum;
      
      xDecodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, bCodeDQP );
      uiLumaOffset += uiSize;  uiChromaOffset += (uiSize>>2);  uiIdx += uiQPartNum;
      
      xDecodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, bCodeDQP );
      uiLumaOffset += uiSize;  uiChromaOffset += (uiSize>>2);  uiIdx += uiQPartNum;
      
      xDecodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, bCodeDQP );
      {
        DTRACE_CABAC_VL( g_nSymbolCounter++ );
        DTRACE_CABAC_T( "\tgoing up\n" );
      }
    }
  }
}

/** decode coefficients
 * \param pcCU
 * \param uiAbsPartIdx 
 * \param uiDepth
 * \param uiWidth
 * \param uiHeight 
 * \returns Void
 */
Void TDecEntropy::decodeCoeff( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, Bool& bCodeDQP )
{
  UInt uiMinCoeffSize = pcCU->getPic()->getMinCUWidth()*pcCU->getPic()->getMinCUHeight();
  UInt uiLumaOffset   = uiMinCoeffSize*uiAbsPartIdx;
  UInt uiChromaOffset = uiLumaOffset>>2;
  UInt uiLumaTrMode, uiChromaTrMode;
  
  if( pcCU->isIntra(uiAbsPartIdx) )
  {
    decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );
    
  }
  else
  {
    UInt uiQtRootCbf = 1;
    if( !( pcCU->getPartitionSize( uiAbsPartIdx) == SIZE_2Nx2N && pcCU->getMergeFlag( uiAbsPartIdx ) ) )
    {
      m_pcEntropyDecoderIf->parseQtRootCbf( pcCU, uiAbsPartIdx, uiDepth, uiQtRootCbf );
    }
    if ( !uiQtRootCbf )
    {
      pcCU->setCbfSubParts( 0, 0, 0, uiAbsPartIdx, uiDepth );
      pcCU->setTrIdxSubParts( 0 , uiAbsPartIdx, uiDepth );
      return;
    }
    
    decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );
  }
  
  xDecodeCoeff( pcCU, uiLumaOffset, uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth, uiHeight, 0, uiLumaTrMode, bCodeDQP );
}

#if !PARAMSET_VLC_CLEANUP
/** Decode SAO for one partition
 * \param  pSaoParam, iPartIdx
 */
Void TDecEntropy::decodeSaoOnePart(SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr)
{
  UInt uiSymbol;
  Int iSymbol;  
  SAOQTPart*  pSaoPart = NULL;
  pSaoPart = &(pSaoParam->psSaoPart[iYCbCr][iPartIdx]);

  static Int iTypeLength[MAX_NUM_SAO_TYPE] =
  {
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_BO_LEN,
    SAO_BO_LEN
  };  
  if(!pSaoPart->bSplit)
  {
    m_pcEntropyDecoderIf->parseSaoUvlc(uiSymbol);
    if (uiSymbol)
    {
      pSaoPart->iBestType = uiSymbol-1;
      pSaoPart->bEnableFlag = true;
    }
    else
    {
      pSaoPart->iBestType = -1;
      pSaoPart->bEnableFlag = false;
    }

    if (pSaoPart->bEnableFlag)
    {
      pSaoPart->iLength = iTypeLength[pSaoPart->iBestType];
      for(Int i=0; i< pSaoPart->iLength; i++)
      {
        m_pcEntropyDecoderIf->parseSaoSvlc(iSymbol);
        pSaoPart->iOffset[i] = iSymbol;
      }
    }
    return;
  }

  //split
  if (pSaoPart->PartLevel < pSaoParam->iMaxSplitLevel)
  {
    for(Int i=0;i<NUM_DOWN_PART;i++)
    {
      decodeSaoOnePart(pSaoParam, pSaoPart->DownPartsIdx[i], iYCbCr);
    }
  }
}

/** Decode quadtree split flag
 * \param  pSaoParam, iPartIdx
 */
Void TDecEntropy::decodeQuadTreeSplitFlag(SAOParam* pSaoParam, Int iPartIdx, Int iYCbCr)
{
  UInt uiSymbol;
  SAOQTPart*  pSaoPart = NULL;
  pSaoPart= &(pSaoParam->psSaoPart[iYCbCr][iPartIdx]);

  if(pSaoPart->PartLevel < pSaoParam->iMaxSplitLevel)
  {
    m_pcEntropyDecoderIf->parseSaoFlag(uiSymbol); 
    pSaoPart->bSplit = uiSymbol? true:false; 
    if(pSaoPart->bSplit)
    {
      for (Int i=0;i<NUM_DOWN_PART;i++)
      {
        decodeQuadTreeSplitFlag(pSaoParam, pSaoPart->DownPartsIdx[i], iYCbCr);
      }
    }
  }
  else
  {
    pSaoPart->bSplit = false; 
  }
}

/** Decode SAO parameters
 * \param  pSaoParam
 */
Void TDecEntropy::decodeSaoParam(SAOParam* pSaoParam)
{
  UInt uiSymbol;

  if (pSaoParam->bSaoFlag[0])
  {
    decodeQuadTreeSplitFlag(pSaoParam, 0, 0);
    decodeSaoOnePart(pSaoParam, 0, 0);
    m_pcEntropyDecoderIf->parseSaoFlag(uiSymbol);
    pSaoParam->bSaoFlag[1] = uiSymbol? true:false;
    if (pSaoParam->bSaoFlag[1])
    {
      decodeQuadTreeSplitFlag(pSaoParam, 0, 1);
      decodeSaoOnePart(pSaoParam, 0, 1);
    }

    m_pcEntropyDecoderIf->parseSaoFlag(uiSymbol);
    pSaoParam->bSaoFlag[2] = uiSymbol? true:false;
    if (pSaoParam->bSaoFlag[2])
    {
      decodeQuadTreeSplitFlag(pSaoParam, 0, 2);
      decodeSaoOnePart(pSaoParam, 0, 2);
    }
  }

}

Void TDecEntropy::decodeDFParams(TComAPS *pcAPS)
{
  UInt uiSymbol;
  Int iSymbol;

  m_pcEntropyDecoderIf->parseDFFlag(uiSymbol, "loop_filter_disable");
  pcAPS->setLoopFilterDisable(uiSymbol?true:false);

  if (!pcAPS->getLoopFilterDisable())
  {
    m_pcEntropyDecoderIf->parseDFSvlc(iSymbol, "beta_offset_div2");
    pcAPS->setLoopFilterBetaOffset(iSymbol);
    m_pcEntropyDecoderIf->parseDFSvlc(iSymbol, "tc_offset_div2");
    pcAPS->setLoopFilterTcOffset(iSymbol);
  }
}

#endif
//! \}
