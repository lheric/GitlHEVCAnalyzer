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

Void TDecEntropy::decodeAux(ALFParam* pAlfParam)
{
  UInt uiSymbol;
#if STAR_CROSS_SHAPES_LUMA
  Int sqrFiltLengthTab[2] = {10, 9}; 
#else
  Int sqrFiltLengthTab[3] = {SQR_FILT_LENGTH_9SYM, SQR_FILT_LENGTH_7SYM, SQR_FILT_LENGTH_5SYM};
  Int FiltTab[3] = {9, 7, 5};
#endif
  
  pAlfParam->filters_per_group = 0;
  
  memset (pAlfParam->filterPattern, 0 , sizeof(Int)*NO_VAR_BINS);
#if ENABLE_FORCECOEFF0
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  if (!uiSymbol) pAlfParam->filtNo = -1;
  else pAlfParam->filtNo = uiSymbol; //nonZeroCoeffs
#else
  pAlfParam->filtNo = 1; //nonZeroCoeffs
#endif

#if MQT_BA_RA
  m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
  pAlfParam->alf_pcr_region_flag = uiSymbol;  
#endif

  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
#if STAR_CROSS_SHAPES_LUMA
  pAlfParam->realfiltNo = uiSymbol;
#else
  Int TabIdx = uiSymbol;
  pAlfParam->realfiltNo = 2-TabIdx;
  pAlfParam->tap = FiltTab[pAlfParam->realfiltNo];
#if TI_ALF_MAX_VSIZE_7
  pAlfParam->tapV = TComAdaptiveLoopFilter::ALFTapHToTapV(pAlfParam->tap);
#endif
#endif
  pAlfParam->num_coeff = sqrFiltLengthTab[pAlfParam->realfiltNo];
  
  if (pAlfParam->filtNo>=0)
  {
    if(pAlfParam->realfiltNo >= 0)
    {
      // filters_per_fr
      m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
      pAlfParam->noFilters = uiSymbol + 1;
      pAlfParam->filters_per_group = pAlfParam->noFilters; 

      if(pAlfParam->noFilters == 2)
      {
        m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
        pAlfParam->startSecondFilter = uiSymbol;
        pAlfParam->filterPattern [uiSymbol] = 1;
      }
      else if (pAlfParam->noFilters > 2)
      {
        pAlfParam->filters_per_group = 1;
        for (int i=1; i<NO_VAR_BINS; i++) 
        {
          m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
          pAlfParam->filterPattern[i] = uiSymbol;
          pAlfParam->filters_per_group += uiSymbol;
        }
      }
    }
  }
  else
  {
    memset (pAlfParam->filterPattern, 0, NO_VAR_BINS*sizeof(Int));
  }
  // Reconstruct varIndTab[]
  memset(pAlfParam->varIndTab, 0, NO_VAR_BINS * sizeof(int));
  if(pAlfParam->filtNo>=0)
  {
    for(Int i = 1; i < NO_VAR_BINS; ++i)
    {
      if(pAlfParam->filterPattern[i])
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i-1] + 1;
      else
        pAlfParam->varIndTab[i] = pAlfParam->varIndTab[i-1];
    }
  }
}

Void TDecEntropy::readFilterCodingParams(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  int ind, scanPos;
  int golombIndexBit;
  int kMin;
  int maxScanVal;
  int *pDepthInt;
#if STAR_CROSS_SHAPES_LUMA
  // Determine maxScanVal
  maxScanVal = 0;
  pDepthInt = pDepthIntTabShapes[pAlfParam->realfiltNo];
#else
  int fl;
  
  // Determine fl
  if(pAlfParam->num_coeff == SQR_FILT_LENGTH_9SYM)
    fl = 4;
  else if(pAlfParam->num_coeff == SQR_FILT_LENGTH_7SYM)
    fl = 3;
  else
    fl = 2;
  
  // Determine maxScanVal
  maxScanVal = 0;
  pDepthInt = pDepthIntTab[fl - 2];
#endif
  for(ind = 0; ind < pAlfParam->num_coeff; ind++)
    maxScanVal = max(maxScanVal, pDepthInt[ind]);
  
  // Golomb parameters
  m_pcEntropyDecoderIf->parseAlfUvlc(uiSymbol);
  pAlfParam->minKStart = 1 + uiSymbol;
  
  kMin = pAlfParam->minKStart;
  for(scanPos = 0; scanPos < maxScanVal; scanPos++)
  {
    m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
    golombIndexBit = uiSymbol;
    if(golombIndexBit)
      pAlfParam->kMinTab[scanPos] = kMin + 1;
    else
      pAlfParam->kMinTab[scanPos] = kMin;
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
      nr += 1 << a;
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
#if STAR_CROSS_SHAPES_LUMA
  pDepthInt = pDepthIntTabShapes[pAlfParam->realfiltNo];
#else
  int fl;
  
  if(pAlfParam->num_coeff == SQR_FILT_LENGTH_9SYM)
    fl = 4;
  else if(pAlfParam->num_coeff == SQR_FILT_LENGTH_7SYM)
    fl = 3;
  else
    fl = 2;
  
  pDepthInt = pDepthIntTab[fl - 2];
#endif
  
  for(ind = 0; ind < pAlfParam->filters_per_group_diff; ++ind)
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
  
  if (pAlfParam->filtNo >= 0)
  {
    pAlfParam->filters_per_group_diff = pAlfParam->filters_per_group;
    if (pAlfParam->filters_per_group > 1)
    {
#if ENABLE_FORCECOEFF0
      m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
      pAlfParam->forceCoeff0 = uiSymbol;

      if (pAlfParam->forceCoeff0)
      {
        pAlfParam->filters_per_group_diff = 0;
        for (int i=0; i<pAlfParam->filters_per_group; i++)
        {
          m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
          pAlfParam->codedVarBins[i] = uiSymbol;
          pAlfParam->filters_per_group_diff += uiSymbol;
        }
      }
      else
#else
      pAlfParam->forceCoeff0 = 0;
#endif
      {
        for (int i=0; i<NO_VAR_BINS; i++)
          pAlfParam->codedVarBins[i] = 1;

      }
      m_pcEntropyDecoderIf->parseAlfFlag (uiSymbol);
      pAlfParam->predMethod = uiSymbol;
    }
    else
    {
      pAlfParam->forceCoeff0 = 0;
      pAlfParam->predMethod = 0;
    }

    decodeFilterCoeff (pAlfParam);
  }
}

Void TDecEntropy::decodeAlfParam(ALFParam* pAlfParam)
{
  UInt uiSymbol;
  Int iSymbol;
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->alf_flag = uiSymbol;
  
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
#if ALF_CHROMA_NEW_SHAPES
    Int sqrFiltLengthTab[2] = {10, 9};
    pAlfParam->realfiltNo_chroma = uiSymbol;
    pAlfParam->num_coeff_chroma = sqrFiltLengthTab[pAlfParam->realfiltNo_chroma];
#else
    pAlfParam->tap_chroma = (uiSymbol<<1) + 5;
    pAlfParam->num_coeff_chroma = ((pAlfParam->tap_chroma*pAlfParam->tap_chroma+1)>>1) + 1;
#endif
    
    // filter coefficients for chroma
    for(pos=0; pos<pAlfParam->num_coeff_chroma; pos++)
    {
      m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);
      pAlfParam->coeff_chroma[pos] = iSymbol;
    }
  }
#if !E045_SLICE_COMMON_INFO_SHARING    
  // region control parameters for luma
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->cu_control_flag = uiSymbol;
  if (pAlfParam->cu_control_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(true);
    m_pcEntropyDecoderIf->parseAlfCtrlDepth(uiSymbol);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(uiSymbol);
    pAlfParam->alf_max_depth = uiSymbol;
    decodeAlfCtrlParam(pAlfParam);
  }
  else
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(0); //unncessary
  }
#endif
}

/** decode ALF CU control parameters
 * \param pAlfParam ALF paramters
 */
#if E045_SLICE_COMMON_INFO_SHARING
Void TDecEntropy::decodeAlfCtrlParam( ALFParam* pAlfParam , Bool bFirstSliceInPic)
#else
Void TDecEntropy::decodeAlfCtrlParam( ALFParam* pAlfParam )
#endif
{
  UInt uiSymbol;

#if E045_SLICE_COMMON_INFO_SHARING
  if(pAlfParam->alf_flag ==0)
  {
    return;
  }
  m_pcEntropyDecoderIf->parseAlfFlag(uiSymbol);
  pAlfParam->cu_control_flag = uiSymbol;
  if (pAlfParam->cu_control_flag)
  {
    m_pcEntropyDecoderIf->setAlfCtrl(true);
    m_pcEntropyDecoderIf->parseAlfCtrlDepth(uiSymbol);
    m_pcEntropyDecoderIf->setMaxAlfCtrlDepth(uiSymbol);
    pAlfParam->alf_max_depth = uiSymbol;
  }
  else
  {
    m_pcEntropyDecoderIf->setAlfCtrl(false);
    return;
  }
#endif

#if E045_SLICE_COMMON_INFO_SHARING
  Int iSymbol;
  m_pcEntropyDecoderIf->parseAlfSvlc(iSymbol);

  uiSymbol = (UInt)(iSymbol + (Int)pAlfParam->num_cus_in_frame);
#else

#if MTK_NONCROSS_INLOOP_FILTER
  Int  iDepth         = pAlfParam->alf_max_depth;
#if FINE_GRANULARITY_SLICES
  Int  iGranularity   = m_pcEntropyDecoderIf->getSliceGranularity();
  if(iGranularity > iDepth)
  {
    iDepth = iGranularity;
  }
#endif
  m_pcEntropyDecoderIf->parseAlfFlagNum( uiSymbol, pAlfParam->num_cus_in_frame, iDepth );
#else
  m_pcEntropyDecoderIf->parseAlfFlagNum( uiSymbol, pAlfParam->num_cus_in_frame, pAlfParam->alf_max_depth );
#endif

#endif


#if E045_SLICE_COMMON_INFO_SHARING
  if(bFirstSliceInPic)
  {
    pAlfParam->num_alf_cu_flag = 0;
  }
  UInt uiSliceNumAlfFlags = uiSymbol;
  UInt uiSliceAlfFlagsPos = pAlfParam->num_alf_cu_flag;

  pAlfParam->num_alf_cu_flag += uiSliceNumAlfFlags;
#else
  pAlfParam->num_alf_cu_flag = uiSymbol;
#endif

#if E045_SLICE_COMMON_INFO_SHARING
  for(UInt i=uiSliceAlfFlagsPos; i<pAlfParam->num_alf_cu_flag; i++)
#else
  for(UInt i=0; i<pAlfParam->num_alf_cu_flag; i++)
#endif
  {
    m_pcEntropyDecoderIf->parseAlfCtrlFlag( pAlfParam->alf_cu_flag[i] );
  }
}

Void TDecEntropy::decodeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  m_pcEntropyDecoderIf->parseAlfCtrlFlag( pcCU, uiAbsPartIdx, uiDepth );
}

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
#if !CHANGE_GET_MERGE_CANDIDATE && !MRG_AMVP_FIXED_IDX_F470
  UInt uiNumCand = 0;
  for(UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
  {
    if( pcSubCU->getNeighbourCandIdx( uiIter, uiAbsPartIdx ) == ( uiIter + 1 ) )
    {
      uiNumCand++;
      break;
    }
  }
  if( uiNumCand )
  {
#endif
    // at least one merge candidate exists
    m_pcEntropyDecoderIf->parseMergeFlag( pcSubCU, uiAbsPartIdx, uiDepth, uiPUIdx );
#if !CHANGE_GET_MERGE_CANDIDATE && !MRG_AMVP_FIXED_IDX_F470
  }
  else
  {
    assert( !pcSubCU->getMergeFlag( uiAbsPartIdx ) );
  }
#endif
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
#if MRG_AMVP_FIXED_IDX_F470
  UInt uiMergeIndex = 0;
  m_pcEntropyDecoderIf->parseMergeIndex( pcCU, uiMergeIndex, uiAbsPartIdx, uiDepth );
  pcCU->setMergeIndexSubParts( uiMergeIndex, uiAbsPartIdx, uiPartIdx, uiDepth );
#else
  UInt uiNumCand = 0;
  for(UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
  {
    if( pcCU->getNeighbourCandIdx( uiIter, uiAbsPartIdx ) == ( uiIter + 1 ) )
    {
      uiNumCand++;
    }
  }
  // Merge to left or above depending on uiMergeIndex
  UInt uiMergeIndex = 0;
  if ( uiNumCand > 1 )
  {
    // two different motion parameter sets exist
    // parse merge index.
    m_pcEntropyDecoderIf->parseMergeIndex( pcCU, uiMergeIndex, uiAbsPartIdx, uiDepth );
  }
  else
  {
    if( pcCU->getNeighbourCandIdx( 0, uiAbsPartIdx ) == 1 )
    {
      uiMergeIndex = 0;
    }
    else if( pcCU->getNeighbourCandIdx( 1, uiAbsPartIdx ) == 2 )
    {
      uiMergeIndex = 1;
    }
    else if( pcCU->getNeighbourCandIdx( 2, uiAbsPartIdx ) == 3 )
    {
      uiMergeIndex = 2;
    }
    else if( pcCU->getNeighbourCandIdx( 3, uiAbsPartIdx ) == 4 )
    {
      uiMergeIndex = 3;
    }
    else if( pcCU->getNeighbourCandIdx( 4, uiAbsPartIdx ) == 5 )
    {
      uiMergeIndex = 4;
    }
  }
  pcCU->setMergeIndexSubParts( uiMergeIndex, uiAbsPartIdx, uiPartIdx, uiDepth );
  pcCU->setInterDirSubParts( puhInterDirNeighbours[uiMergeIndex], uiAbsPartIdx, uiPartIdx, uiDepth );

  TComMv cTmpMv( 0, 0 );
  if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0  ) //if ( ref. frame list0 has at least 1 entry )
  {
    pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_0, uiAbsPartIdx, uiPartIdx, uiDepth);
    pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_0, uiAbsPartIdx, uiPartIdx, uiDepth);
#if AMP
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cTmpMv, eCUMode, uiAbsPartIdx, uiDepth, uiPartIdx );
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( pcMvFieldNeighbours[ 2*uiMergeIndex ], eCUMode, uiAbsPartIdx, uiDepth, uiPartIdx );
#else
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cTmpMv, eCUMode, uiAbsPartIdx, uiDepth );
    pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( pcMvFieldNeighbours[ 2*uiMergeIndex ], eCUMode, uiAbsPartIdx, uiDepth );
#endif
  }
  if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
  {
    pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_1, uiAbsPartIdx, uiPartIdx, uiDepth);
    pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_1, uiAbsPartIdx, uiPartIdx, uiDepth);
#if AMP
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cTmpMv, eCUMode, uiAbsPartIdx, uiDepth, uiPartIdx );
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( pcMvFieldNeighbours[ 2*uiMergeIndex + 1 ], eCUMode, uiAbsPartIdx, uiDepth, uiPartIdx );
#else
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cTmpMv, eCUMode, uiAbsPartIdx, uiDepth );
    pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( pcMvFieldNeighbours[ 2*uiMergeIndex + 1 ], eCUMode, uiAbsPartIdx, uiDepth );
#endif 
  }
#endif
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

#if E057_INTRA_PCM
/** Parse I_PCM information. 
 * \param pcCU  pointer to CUpointer to CU
 * \param uiAbsPartIdx CU index
 * \param uiDepth CU depth
 * \returns Void
 */
Void TDecEntropy::decodeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  if(pcCU->getWidth(uiAbsPartIdx) < (1<<pcCU->getSlice()->getSPS()->getPCMLog2MinSize()))
    return;

  m_pcEntropyDecoderIf->parseIPCMInfo( pcCU, uiAbsPartIdx, uiDepth );
}
#endif 

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
#if MRG_AMVP_FIXED_IDX_F470
    Int numValidMergeCand = 0;
#else
    UInt uiNeighbourCandIdx[MRG_MAX_NUM_CANDS]; //MVs with same idx => same cand
#endif
    for( UInt ui = 0; ui < MRG_MAX_NUM_CANDS; ++ui )
    {
      uhInterDirNeighbours[ui] = 0;
#if !MRG_AMVP_FIXED_IDX_F470
      uiNeighbourCandIdx[ui] = 0;
#endif
    }
    if ( pcCU->getSlice()->getSPS()->getUseMRG() )
    {
#if !CHANGE_GET_MERGE_CANDIDATE && !MRG_AMVP_FIXED_IDX_F470
      pcSubCU->getInterMergeCandidates( uiSubPartIdx-uiAbsPartIdx, uiPartIdx, uiDepth, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourCandIdx );
      for(UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
      {
        pcCU->setNeighbourCandIdxSubParts( uiIter, uiNeighbourCandIdx[uiIter], uiSubPartIdx, uiPartIdx, uiDepth );
      }
#endif
#if PART_MRG
      if (pcCU->getWidth( uiAbsPartIdx ) > 8 && uiNumPU == 2 && uiPartIdx == 0)
      {
        pcCU->setMergeFlagSubParts( true, uiSubPartIdx, uiPartIdx, uiDepth );
      }
      else
  #endif
      decodeMergeFlag( pcCU, uiSubPartIdx, uiDepth, uiPartIdx );
    }
    if ( pcCU->getMergeFlag( uiSubPartIdx ) )
    {
#if MRG_AMVP_FIXED_IDX_F470
      decodeMergeIndex( pcCU, uiPartIdx, uiSubPartIdx, ePartSize, uhInterDirNeighbours, cMvFieldNeighbours, uiDepth );
      pcSubCU->getInterMergeCandidates( uiSubPartIdx-uiAbsPartIdx, uiPartIdx, uiDepth, cMvFieldNeighbours, uhInterDirNeighbours, numValidMergeCand );
      UInt uiMergeIndex = pcCU->getMergeIndex(uiSubPartIdx);
      pcCU->setInterDirSubParts( uhInterDirNeighbours[uiMergeIndex], uiSubPartIdx, uiPartIdx, uiDepth );

      TComMv cTmpMv( 0, 0 );
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_0 ) > 0  ) //if ( ref. frame list0 has at least 1 entry )
      {
        pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_0, uiSubPartIdx, uiPartIdx, uiDepth);
        pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_0, uiSubPartIdx, uiPartIdx, uiDepth);
#if AMP
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cTmpMv, ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ], ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
#else
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvd( cTmpMv, ePartSize, uiSubPartIdx, uiDepth );
        pcCU->getCUMvField( REF_PIC_LIST_0 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex ], ePartSize, uiSubPartIdx, uiDepth );
#endif
      }
      if ( pcCU->getSlice()->getNumRefIdx( REF_PIC_LIST_1 ) > 0 ) //if ( ref. frame list1 has at least 1 entry )
      {
        pcCU->setMVPIdxSubParts( 0, REF_PIC_LIST_1, uiSubPartIdx, uiPartIdx, uiDepth);
        pcCU->setMVPNumSubParts( 0, REF_PIC_LIST_1, uiSubPartIdx, uiPartIdx, uiDepth);
#if AMP
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cTmpMv, ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ], ePartSize, uiSubPartIdx, uiDepth, uiPartIdx );
#else
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvd( cTmpMv, ePartSize, uiSubPartIdx, uiDepth );
        pcCU->getCUMvField( REF_PIC_LIST_1 )->setAllMvField( cMvFieldNeighbours[ 2*uiMergeIndex + 1 ], ePartSize, uiSubPartIdx, uiDepth );
#endif
      }
#else
#if CHANGE_GET_MERGE_CANDIDATE
      pcSubCU->getInterMergeCandidates( uiSubPartIdx-uiAbsPartIdx, uiPartIdx, uiDepth, cMvFieldNeighbours, uhInterDirNeighbours, uiNeighbourCandIdx );
      for(UInt uiIter = 0; uiIter < MRG_MAX_NUM_CANDS; uiIter++ )
      {
        pcCU->setNeighbourCandIdxSubParts( uiIter, uiNeighbourCandIdx[uiIter], uiSubPartIdx, uiPartIdx, uiDepth );
      }
#endif
      decodeMergeIndex( pcCU, uiPartIdx, uiSubPartIdx, ePartSize, uhInterDirNeighbours, cMvFieldNeighbours, uiDepth );
#endif
    }
    else
    {
      if ( pcCU->getSlice()->getSymbolMode() == 0 )
      {
#if AVOID_NEIGHBOR_REF_F470
        if ( !pcCU->getSlice()->isInterB() )
        {
          pcCU->getSlice()->setRefIdxCombineCoding( false );
        }
        else
        {
          pcCU->getSlice()->setRefIdxCombineCoding( true );
        }
#else
        if ( pcCU->isSuroundingRefIdxException( uiAbsPartIdx ) )
        {
          pcCU->getSlice()->setRefIdxCombineCoding( false );
        }
        else
        {
          pcCU->getSlice()->setRefIdxCombineCoding( true );
        }
#endif
      }
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

#if AMP
    pcCU->getCUMvField( eRefListTemp )->setAllRefIdx( iRefFrmIdxTemp, ePartSize, uiAbsPartIdx, uiDepth, uiPartIdx );
#else
    pcCU->getCUMvField( eRefListTemp )->setAllRefIdx( iRefFrmIdxTemp, ePartSize, uiAbsPartIdx, uiDepth );
#endif

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
#if AMP
    pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, ePartSize, uiAbsPartIdx, uiDepth, uiPartIdx );
#else
    pcCU->getCUMvField( eRefList )->setAllRefIdx( iRefFrmIdx, ePartSize, uiAbsPartIdx, uiDepth );
#endif
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
  Int iMVPIdx;

  TComMv cZeroMv( 0, 0 );
  TComMv cMv     = cZeroMv;
  Int    iRefIdx = -1;

  TComCUMvField* pcSubCUMvField = pcSubCU->getCUMvField( eRefList );
  AMVPInfo* pAMVPInfo = pcSubCUMvField->getAMVPInfo();

  iRefIdx = pcSubCUMvField->getRefIdx(uiPartAddr);
  iMVPIdx = -1;
  cMv = cZeroMv;

#if MRG_AMVP_FIXED_IDX_F470
  if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
  }
  pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
  pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
#else
  pcSubCU->fillMvpCand(uiPartIdx, uiPartAddr, eRefList, iRefIdx, pAMVPInfo);
  pcSubCU->setMVPNumSubParts(pAMVPInfo->iN, eRefList, uiPartAddr, uiPartIdx, uiDepth);
  if ( (pcSubCU->getInterDir(uiPartAddr) & ( 1 << eRefList )) && (pAMVPInfo->iN > 1) && (pcSubCU->getAMVPMode(uiPartAddr) == AM_EXPL) )
  {
    m_pcEntropyDecoderIf->parseMVPIdx( pcSubCU, iMVPIdx, pAMVPInfo->iN, uiPartAddr, uiDepth, eRefList );
  }
#endif
  pcSubCU->setMVPIdxSubParts( iMVPIdx, eRefList, uiPartAddr, uiPartIdx, uiDepth );
  if ( iRefIdx >= 0 )
  {
    m_pcPrediction->getMvPredAMVP( pcSubCU, uiPartIdx, uiPartAddr, eRefList, iRefIdx, cMv);
    cMv += pcSubCUMvField->getMvd( uiPartAddr );
  }

  PartSize ePartSize = pcSubCU->getPartitionSize( uiPartAddr );
#if AMP
  pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 0, uiPartIdx);
#else
  pcSubCU->getCUMvField( eRefList )->setAllMv(cMv, ePartSize, uiPartAddr, 0);
#endif
}

Void TDecEntropy::xDecodeTransformSubdiv( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, UInt uiInnerQuadIdx, UInt& uiYCbfFront3, UInt& uiUCbfFront3, UInt& uiVCbfFront3 )
{
  UInt uiSubdiv;
  const UInt uiLog2TrafoSize = g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMaxCUWidth()]+2 - uiDepth;

#if CAVLC_RQT_CBP
  if(pcCU->getSlice()->getSymbolMode()==0)
  {
    // code CBP and transform split flag jointly
    UInt uiTrDepth =  uiDepth - pcCU->getDepth( uiAbsPartIdx );
    m_pcEntropyDecoderIf->parseCbfTrdiv( pcCU, uiAbsPartIdx, uiTrDepth, uiDepth, uiSubdiv );
  }
  else
  {//CABAC
#endif
  if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA && pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_NxN && uiDepth == pcCU->getDepth(uiAbsPartIdx) )
  {
    uiSubdiv = 1;
  }
#ifdef MOT_TUPU_MAXDEPTH1
  else if( (pcCU->getSlice()->getSPS()->getQuadtreeTUMaxDepthInter() == 1) && (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER) && ( pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2NxN || pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_Nx2N) && (uiDepth == pcCU->getDepth(uiAbsPartIdx)) )
  {
    uiSubdiv = (uiLog2TrafoSize > pcCU->getQuadtreeTULog2MinSizeInCU(uiAbsPartIdx));
  }
#endif
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
#if CAVLC_RQT_CBP
  }
#endif
  
  const UInt uiTrDepth = uiDepth - pcCU->getDepth( uiAbsPartIdx );
  
  if(pcCU->getSlice()->getSymbolMode()==0)
  {
    if( uiSubdiv )
    {
      ++uiDepth;
      const UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      
      for( Int i = 0; i < 4; i++ )
      {
        UInt uiDummyCbfY = 0;
        UInt uiDummyCbfU = 0;
        UInt uiDummyCbfV = 0;
        xDecodeTransformSubdiv( pcCU, uiAbsPartIdx, uiDepth, i, uiDummyCbfY, uiDummyCbfU, uiDummyCbfV );
        uiAbsPartIdx += uiQPartNum;
      }
    }
    else
    {
      assert( uiDepth >= pcCU->getDepth( uiAbsPartIdx ) );
      pcCU->setTrIdxSubParts( uiTrDepth, uiAbsPartIdx, uiDepth );
    }
  }
  else
  {
#if DNB_CHROMA_CBF_FLAGS
    if( uiLog2TrafoSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
#else
    if( pcCU->getPredictionMode(uiAbsPartIdx) != MODE_INTRA && uiLog2TrafoSize <= pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
#endif
    {
      const Bool bFirstCbfOfCU = uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() || uiTrDepth == 0;
      if( bFirstCbfOfCU )
      {
        pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepth );
        pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepth );
      }
      if( bFirstCbfOfCU || uiLog2TrafoSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
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
        
        if ( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
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
        if ( pcCU->getPredictionMode( uiAbsPartIdx ) != MODE_INTRA && uiInnerQuadIdx == 3 && uiYCbfFront3 == 0 && uiUCbfFront3 == 0 && uiVCbfFront3 == 0 && uiLog2TrafoSize < pcCU->getSlice()->getSPS()->getQuadtreeTULog2MaxSize() )
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
      
#if !DNB_CHROMA_CBF_FLAGS
      if( pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTRA )
      {
        Bool bCodeChroma   = true;
        UInt uiDepthChroma = uiDepth;
        if( uiLog2TrafoSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
        {
          UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
          bCodeChroma  = ( ( uiAbsPartIdx % uiQPDiv ) == 0 );
          uiDepthChroma--;
        }
        if( bCodeChroma )
        {
          pcCU->setCbfSubParts( 0, TEXT_CHROMA_U, uiAbsPartIdx, uiDepthChroma );
          pcCU->setCbfSubParts( 0, TEXT_CHROMA_V, uiAbsPartIdx, uiDepthChroma );
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_U, uiChromaTrMode, uiDepthChroma );
          m_pcEntropyDecoderIf->parseQtCbf( pcCU, uiAbsPartIdx, TEXT_CHROMA_V, uiChromaTrMode, uiDepthChroma );
        }
      }
#endif
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
  if ( pcCU->getSlice()->getSPS()->getUseDQP() )
  {
    m_pcEntropyDecoderIf->parseDeltaQP( pcCU, uiAbsPartIdx, uiDepth );
  }
}


Void TDecEntropy::xDecodeCoeff( TComDataCU* pcCU, TCoeff* pcCoeff, UInt uiAbsPartIdx, UInt uiDepth, UInt uiWidth, UInt uiHeight, UInt uiTrIdx, UInt uiCurrTrIdx, TextType eType, Bool& bCodeDQP )
{
  if ( pcCU->getCbf( uiAbsPartIdx, eType, uiTrIdx ) )
  {
    // dQP: only for LCU
    if ( pcCU->getSlice()->getSPS()->getUseDQP() )
    {
      if ( bCodeDQP )
      {
#if SUB_LCU_DQP
        decodeQP( pcCU, uiAbsPartIdx, uiDepth);
#else
        decodeQP( pcCU, 0, 0 );
#endif
        bCodeDQP = false;
      }
    }   
    UInt uiLumaTrMode, uiChromaTrMode;
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx( uiAbsPartIdx ), uiLumaTrMode, uiChromaTrMode );
    const UInt uiStopTrMode = eType == TEXT_LUMA ? uiLumaTrMode : uiChromaTrMode;
    
    if( uiTrIdx == uiStopTrMode )
    {
      UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
      if( eType != TEXT_LUMA && uiLog2TrSize == pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
      {
        UInt uiQPDiv = pcCU->getPic()->getNumPartInCU() >> ( ( uiDepth - 1 ) << 1 );
        if( ( uiAbsPartIdx % uiQPDiv ) != 0 )
        {
          return;
        }
        uiWidth  <<= 1;
        uiHeight <<= 1;
      }
      m_pcEntropyDecoderIf->parseCoeffNxN( pcCU, pcCoeff, uiAbsPartIdx, uiWidth, uiHeight, uiDepth, eType );
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
        assert(1);
      UInt uiSize;
      uiWidth  >>= 1;
      uiHeight >>= 1;
      uiSize = uiWidth*uiHeight;
      uiDepth++;
      uiTrIdx++;
      
      UInt uiQPartNum = pcCU->getPic()->getNumPartInCU() >> (uiDepth << 1);
      UInt uiIdx      = uiAbsPartIdx;
      
#if !CAVLC_RQT_CBP
      if(pcCU->getSlice()->getSymbolMode() == 0)
      {
        UInt uiLog2TrSize = g_aucConvertToBit[ pcCU->getSlice()->getSPS()->getMaxCUWidth() >> uiDepth ] + 2;
        if( eType == TEXT_LUMA || uiLog2TrSize > pcCU->getSlice()->getSPS()->getQuadtreeTULog2MinSize() )
          m_pcEntropyDecoderIf->parseBlockCbf(pcCU, uiIdx, eType, uiTrIdx, uiDepth, uiQPartNum);
        else
        {
          UInt uiCbf = pcCU->getCbf( uiIdx, eType );
          pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx, uiDepth ); 
          uiCbf = pcCU->getCbf( uiIdx + uiQPartNum, eType );
          pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx + uiQPartNum, uiDepth ); 
          uiCbf = pcCU->getCbf( uiIdx + 2*uiQPartNum, eType );
          pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx + 2*uiQPartNum, uiDepth ); 
          uiCbf = pcCU->getCbf( uiIdx + 3*uiQPartNum, eType );
          pcCU->setCbfSubParts( uiCbf | ( 0x01 << uiTrIdx ), eType, uiIdx + 3*uiQPartNum, uiDepth ); 
        }
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP );
      }
      else
      {
#endif
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP ); pcCoeff += uiSize; uiIdx += uiQPartNum;
        xDecodeCoeff( pcCU, pcCoeff, uiIdx, uiDepth, uiWidth, uiHeight, uiTrIdx, uiCurrTrIdx, eType, bCodeDQP );
#if !CAVLC_RQT_CBP
      }
#endif
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
    
    if (pcCU->getSlice()->getSymbolMode() == 0)
    {
#if !CAVLC_RQT_CBP
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
#endif
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
        return;
    }
  }
  else
  {
    if (pcCU->getSlice()->getSymbolMode()==0)
    {
#if !CAVLC_RQT_CBP
      m_pcEntropyDecoderIf->parseCbf( pcCU, uiAbsPartIdx, TEXT_ALL, 0, uiDepth );
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
        return;
#endif
    }
    else
    {
      UInt uiQtRootCbf = 1;
#if HHI_MRG_SKIP
      if( !( pcCU->getPartitionSize( uiAbsPartIdx) == SIZE_2Nx2N && pcCU->getMergeFlag( uiAbsPartIdx ) ) )
      {
        m_pcEntropyDecoderIf->parseQtRootCbf( pcCU, uiAbsPartIdx, uiDepth, uiQtRootCbf );
      }
#else
      m_pcEntropyDecoderIf->parseQtRootCbf( pcCU, uiAbsPartIdx, uiDepth, uiQtRootCbf );
#endif
      if ( !uiQtRootCbf )
      {
        pcCU->setCbfSubParts( 0, 0, 0, uiAbsPartIdx, uiDepth );
        pcCU->setTrIdxSubParts( 0 , uiAbsPartIdx, uiDepth );
        return;
      }
    }
    
    decodeTransformIdx( pcCU, uiAbsPartIdx, pcCU->getDepth(uiAbsPartIdx) );
    
#if CAVLC_RQT_CBP
    if (pcCU->getSlice()->getSymbolMode() == 0)
    {
      if(pcCU->getCbf(uiAbsPartIdx, TEXT_LUMA, 0)==0 && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_U, 0)==0
         && pcCU->getCbf(uiAbsPartIdx, TEXT_CHROMA_V, 0)==0)
         return;
    }
#endif
    pcCU->convertTransIdx( uiAbsPartIdx, pcCU->getTransformIdx(uiAbsPartIdx), uiLumaTrMode, uiChromaTrMode );
  }
  
  xDecodeCoeff( pcCU, pcCU->getCoeffY()  + uiLumaOffset,   uiAbsPartIdx, uiDepth, uiWidth,    uiHeight,    0, uiLumaTrMode,   TEXT_LUMA,     bCodeDQP );
  xDecodeCoeff( pcCU, pcCU->getCoeffCb() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_U, bCodeDQP );
  xDecodeCoeff( pcCU, pcCU->getCoeffCr() + uiChromaOffset, uiAbsPartIdx, uiDepth, uiWidth>>1, uiHeight>>1, 0, uiChromaTrMode, TEXT_CHROMA_V, bCodeDQP );
}

#if MTK_SAO
/** decodeQAO One Part
 * \param  pQaoParam, iPartIdx
 */
#if MTK_SAO_CHROMA
Void TDecEntropy::decodeQAOOnePart(SAOParam* pQaoParam, Int iPartIdx, Int iYCbCr)
#else
Void TDecEntropy::decodeQAOOnePart(SAOParam* pQaoParam, Int iPartIdx)
#endif
{
  UInt uiSymbol;
  Int iSymbol;  
#if MTK_SAO_CHROMA
  SAOQTPart*  pAlfPart = NULL;
  if (iYCbCr == 0)
  {
    pAlfPart = &(pQaoParam->psSaoPart[iPartIdx]);
  }
  else if (iYCbCr == 1)
  {
    pAlfPart = &(pQaoParam->psSaoPartCb[iPartIdx]);
  }
  else 
  {
    pAlfPart = &(pQaoParam->psSaoPartCr[iPartIdx]);
  }
#else
  SAOQTPart*  pAlfPart = &(pQaoParam->psSaoPart[iPartIdx]);
#endif

  static Int iTypeLength[MAX_NUM_SAO_TYPE] = {
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_EO_LEN,
    SAO_BO_LEN,
    SAO_BO_LEN
  };  
  if(!pAlfPart->bSplit)
  {

    m_pcEntropyDecoderIf->parseAoUvlc(uiSymbol);

    if (uiSymbol)
    {
      pAlfPart->iBestType = uiSymbol-1;
      pAlfPart->bEnableFlag = true;
    }
    else
    {
      pAlfPart->iBestType = -1;
      pAlfPart->bEnableFlag = false;
    }

    if (pAlfPart->bEnableFlag)
    {

      pAlfPart->iLength = iTypeLength[pAlfPart->iBestType];

      for(Int i=0; i< pAlfPart->iLength; i++)
      {
        m_pcEntropyDecoderIf->parseAoSvlc(iSymbol);
        pAlfPart->iOffset[i] = iSymbol;
      }

    }
    return;
  }

  //split
  if (pAlfPart->PartLevel < pQaoParam->iMaxSplitLevel)
  {
    for(Int i=0;i<NUM_DOWN_PART;i++)
    {
#if MTK_SAO_CHROMA
      decodeQAOOnePart(pQaoParam, pAlfPart->DownPartsIdx[i], iYCbCr);
#else
      decodeQAOOnePart(pQaoParam, pAlfPart->DownPartsIdx[i]);
#endif
    }
  }
}
/** decode QuadTree Split Flag
 * \param  pQaoParam, iPartIdx
 */
#if MTK_SAO_CHROMA
Void TDecEntropy::decodeQuadTreeSplitFlag(SAOParam* pQaoParam, Int iPartIdx, Int iYCbCr)
#else
Void TDecEntropy::decodeQuadTreeSplitFlag(SAOParam* pQaoParam, Int iPartIdx)
#endif
{
  UInt uiSymbol;
#if MTK_SAO_CHROMA
  SAOQTPart*  pAlfPart = NULL;
  if (iYCbCr == 0)
  {
    pAlfPart= &(pQaoParam->psSaoPart[iPartIdx]);
  }
  else if ( iYCbCr == 1 )
  {
    pAlfPart= &(pQaoParam->psSaoPartCb[iPartIdx]);
  }
  else 
  {
    pAlfPart= &(pQaoParam->psSaoPartCr[iPartIdx]);
  }
#else
  SAOQTPart*  pAlfPart = &(pQaoParam->psSaoPart[iPartIdx]);
#endif

  if(pAlfPart->PartLevel < pQaoParam->iMaxSplitLevel)
  {

    //send one flag
    m_pcEntropyDecoderIf->parseAoFlag(uiSymbol); 
    pAlfPart->bSplit = uiSymbol? true:false; 

    if(pAlfPart->bSplit)
    {
      for (Int i=0;i<NUM_DOWN_PART;i++)
      {
#if MTK_SAO_CHROMA
        decodeQuadTreeSplitFlag(pQaoParam, pAlfPart->DownPartsIdx[i], iYCbCr);
#else
        decodeQuadTreeSplitFlag(pQaoParam, pAlfPart->DownPartsIdx[i]);
#endif
      }
    }
  }

}
/** decode Sao Param
 * \param  pQaoParam
 */
Void TDecEntropy::decodeSaoParam(SAOParam* pQaoParam)
{
  UInt uiSymbol;

  m_pcEntropyDecoderIf->parseAoFlag(uiSymbol);

  if (uiSymbol)
  {
    pQaoParam->bSaoFlag = true;
  }
  else
  {
    pQaoParam->bSaoFlag = false;
  }

  if (pQaoParam->bSaoFlag)
  {
#if MTK_SAO_CHROMA
    decodeQuadTreeSplitFlag(pQaoParam, 0, 0);
    decodeQAOOnePart(pQaoParam, 0, 0);
    m_pcEntropyDecoderIf->parseAoFlag(uiSymbol);

    pQaoParam->bSaoFlagCb = uiSymbol? true:false;
    if (pQaoParam->bSaoFlagCb)
    {
      decodeQuadTreeSplitFlag(pQaoParam, 0, 1);
      decodeQAOOnePart(pQaoParam, 0, 1);
    }
    m_pcEntropyDecoderIf->parseAoFlag(uiSymbol);
    pQaoParam->bSaoFlagCr = uiSymbol? true:false;
    if (pQaoParam->bSaoFlagCr)
    {
      decodeQuadTreeSplitFlag(pQaoParam, 0, 2);
      decodeQAOOnePart(pQaoParam, 0, 2);
    }
#else
    decodeQuadTreeSplitFlag(pQaoParam, 0);
    decodeQAOOnePart(pQaoParam, 0);
#endif
  }

}

#endif
//! \}
