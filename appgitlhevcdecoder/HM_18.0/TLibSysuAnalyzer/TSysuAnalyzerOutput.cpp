#include "TSysuAnalyzerOutput.h"

TSysuAnalyzerOutput* TSysuAnalyzerOutput::m_instance = NULL;

TSysuAnalyzerOutput::TSysuAnalyzerOutput()
{

  m_cSpsOut.open     ("decoder_sps.txt",  ios::out);
  m_cPredOutput.open ("decoder_pred.txt", ios::out);
  m_cCUPUOutput.open ("decoder_cupu.txt", ios::out);
  m_cMVOutput.open   ("decoder_mv.txt",   ios::out);
  m_cMergeOutput.open("decoder_merge.txt",ios::out);
  m_cIntraOutput.open("decoder_intra.txt",ios::out);
  m_cTUOutput.open   ("decoder_tu.txt",   ios::out);
  m_cMEOutput.open   ("encoder_me.txt",   ios::out);
  m_cBitOutputLCU.open  ("decoder_bit_lcu.txt",  ios::out);
  m_cBitOutputSCU.open  ("decoder_bit_scu.txt",  ios::out);

  m_cTileOutPut.open("decoder_tile.txt", ios::out);

  m_pcBitstream     = NULL;
  m_uiCtuStartByte  = 0;
  m_uiCUStartByte   = 0;
  m_bYuvFilesOpened = false;


}


//write out tile info
void TSysuAnalyzerOutput::writeOutTileInfo(TComPic * pcPic)
{
    int iPoc = pcPic ->getPOC();
    int iTileNumRows = pcPic->getPicSym()->getNumTileRowsMinus1()+1;
    int iTileNumCols = pcPic->getPicSym()->getNumTileColumnsMinus1()+1;


    for( int uiRowIdx=0; uiRowIdx < iTileNumRows; uiRowIdx++ )
           for( int uiColumnIdx=0; uiColumnIdx <iTileNumCols ; uiColumnIdx++ )
           {
             int uiTileIdx = uiRowIdx * (iTileNumCols) + uiColumnIdx;

             //information for each tile
             int uiTileWidth = 0;
             int uiTileHeight = 0;
             int uiFirstCUAddr = 0;

             uiTileWidth   = pcPic->getPicSym()->getTComTile(uiTileIdx)->getTileWidthInCtus();
             uiTileHeight  = pcPic->getPicSym()->getTComTile(uiTileIdx)->getTileHeightInCtus();
             uiFirstCUAddr = pcPic->getPicSym()->getTComTile(uiTileIdx)->getFirstCtuRsAddr();

             m_cTileOutPut << "<" << iPoc << "," << (iTileNumCols )* (iTileNumRows )<< ">"
                  << " " << uiFirstCUAddr << " " << uiTileWidth << " " << uiTileHeight << endl;


           }
}





void TSysuAnalyzerOutput::writeOutCUInfo   ( TComDataCU* pcCU )
{
  Int iPoc = pcCU->getSlice()->getPOC();
  Int iAddr = pcCU->getCtuRsAddr();
  Int iTotalNumPart = pcCU->getTotalNumPart();


  m_cPredOutput << "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out prediction info
  m_cCUPUOutput << "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out CU & PU splitting info
  m_cMVOutput   << "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out me info
  m_cMergeOutput<< "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out merge mode info
  m_cIntraOutput<< "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out intra mode info
  m_cTUOutput   << "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out TU mode info
  m_cBitOutputLCU << "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out bit info
  m_cBitOutputSCU << "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out bit info
  m_cMEOutput   << "<" << iPoc << "," << iAddr << ">" << " ";  ///< Write out ME info  

  xWriteOutCUInfo  ( pcCU, iTotalNumPart, 0, 0 );   ///< Recursive write Prediction, CU, PU, Merge, Intra, ME
  m_cBitOutputLCU << pcCU->getTotalBits(); ///< Bit info
  for(int i = 0; i < aiCUBits.size(); i++)
    m_cBitOutputSCU << aiCUBits.at(i) << " "; ///< Bit info

  m_cPredOutput << endl;
  m_cCUPUOutput << endl;
  m_cMVOutput   << endl;
  m_cMergeOutput<< endl;
  m_cIntraOutput<< endl;
  m_cTUOutput   << endl;
  m_cBitOutputLCU  << endl;
  m_cBitOutputSCU  << endl;
  m_cMEOutput   << endl;
}

void TSysuAnalyzerOutput::xWriteOutCUInfo  ( TComDataCU* pcCU, Int iLength, Int iOffset, UInt iDepth )
{
  
  UChar* puhDepth    = pcCU->getDepth();
  SChar* puhPartSize = pcCU->getPartitionSize();

  TComMv rcMV;

  if( puhDepth[iOffset] <= iDepth )
  {
    ///< CU PU info
    ///< HM-16+ marks "no partition" with NUMBER_OF_PART_SIZES instead of SIZE_NONE (15)
    m_cCUPUOutput << ( puhPartSize[iOffset] == NUMBER_OF_PART_SIZES ? 15 : (Int)(puhPartSize[iOffset]) ) << " ";

    ///< TU info
    xWriteOutTUInfo  ( pcCU, iLength, iOffset, 0 );   ///< Recursive write TU
    

    /// PU number in this leaf CU
    int iNumPart = 0;           

    switch ( puhPartSize[iOffset] )
    {
    case SIZE_2Nx2N:    iNumPart = 1; break;
    case SIZE_2NxN:     iNumPart = 2; break;
    case SIZE_Nx2N:     iNumPart = 2; break;
    case SIZE_NxN:      iNumPart = 4; break;
    case SIZE_2NxnU:    iNumPart = 2; break;
    case SIZE_2NxnD:    iNumPart = 2; break;
    case SIZE_nLx2N:    iNumPart = 2; break;
    case SIZE_nRx2N:    iNumPart = 2; break;
    default:    iNumPart = 0;  /*assert(0);*/  break;  ///< out of boundery
    }

    /// Traverse every PU
    int iPartAddOffset = 0;   ///< PU offset
    for( int i = 0; i < iNumPart; i++ )
    {

      switch ( puhPartSize[iOffset] )
      {
      case SIZE_2NxN:
        iPartAddOffset = ( i == 0 )? 0 : iLength >> 1;
        break;
      case SIZE_Nx2N:
        iPartAddOffset = ( i == 0 )? 0 : iLength >> 2;
        break;
      case SIZE_NxN:
        iPartAddOffset = ( iLength >> 2 ) * i;
        break;
      case SIZE_2NxnU:    
        iPartAddOffset = ( i == 0 ) ? 0 : iLength >> 3;
        break;
      case SIZE_2NxnD:    
        iPartAddOffset = ( i == 0 ) ? 0 : (iLength >> 1) + (iLength >> 3);
        break;
      case SIZE_nLx2N:    
        iPartAddOffset = ( i == 0 ) ? 0 : iLength >> 4;
        break;
      case SIZE_nRx2N:   
        iPartAddOffset = ( i == 0 ) ? 0 : (iLength >> 2) + (iLength >> 4);
        break;
      default:
        assert ( puhPartSize[iOffset] == SIZE_2Nx2N );
        iPartAddOffset = 0;
        break;
      }

      /// Write prediction info (for historical reason, MODE_SKIP = 0, MODE_INTER = 1 ....) (SKIP mode removed after HM-8.0)
      PredMode ePred = pcCU->getPredictionMode(iOffset+iPartAddOffset);
      Int iPred = ePred; 
      if(ePred == MODE_INTER )
        iPred = 1;
      else if(ePred == MODE_INTRA)
        iPred = 2;
      else if(ePred == NUMBER_OF_PREDICTION_MODES)   ///< HM-16+ has no MODE_NONE
        iPred = 15;
      m_cPredOutput << iPred << " ";

      /// Write merge info
      Bool bMergeFlag = pcCU->getMergeFlag(iOffset+iPartAddOffset);      
      Int iMergeIndex = pcCU->getMergeIndex(iOffset+iPartAddOffset);
      if(bMergeFlag)
        m_cMergeOutput << iMergeIndex << " ";
      else
        m_cMergeOutput << -1 << " ";

      /// Write MV info
      Int iInterDir = pcCU->getInterDir(iOffset+iPartAddOffset);   ///< Inter direction: 0--Invalid, 1--List 0 only, 2--List 1 only, 3--List 0&1(bi-direction)
      int iRefIdx = -1;
      m_cMVOutput << iInterDir << " ";
      if( iInterDir == 0 )
      {
        // do nothing
      }
      else if ( iInterDir == 1 )
      {
        rcMV= pcCU->getCUMvField(REF_PIC_LIST_0)->getMv(iOffset+iPartAddOffset);	
        iRefIdx = pcCU->getCUMvField(REF_PIC_LIST_0)->getRefIdx(iOffset+iPartAddOffset);
        m_cMVOutput << pcCU->getSlice()->getRefPOC(REF_PIC_LIST_0, iRefIdx) << " " << rcMV.getHor() << " " << rcMV.getVer() << " ";
      }
      else if ( iInterDir == 2 )
      {
        rcMV= pcCU->getCUMvField(REF_PIC_LIST_1)->getMv(iOffset+iPartAddOffset);
        iRefIdx = pcCU->getCUMvField(REF_PIC_LIST_1)->getRefIdx(iOffset+iPartAddOffset);
        m_cMVOutput << pcCU->getSlice()->getRefPOC(REF_PIC_LIST_1, iRefIdx) << " " << rcMV.getHor() << " " << rcMV.getVer() << " ";
      }
      else if ( iInterDir == 3 )
      {
        rcMV= pcCU->getCUMvField(REF_PIC_LIST_0)->getMv(iOffset+iPartAddOffset);	
        iRefIdx = pcCU->getCUMvField(REF_PIC_LIST_0)->getRefIdx(iOffset+iPartAddOffset);
        m_cMVOutput << pcCU->getSlice()->getRefPOC(REF_PIC_LIST_0, iRefIdx) << " " << rcMV.getHor() << " " << rcMV.getVer() << " ";
        rcMV= pcCU->getCUMvField(REF_PIC_LIST_1)->getMv(iOffset+iPartAddOffset);
        iRefIdx = pcCU->getCUMvField(REF_PIC_LIST_1)->getRefIdx(iOffset+iPartAddOffset);
        m_cMVOutput << pcCU->getSlice()->getRefPOC(REF_PIC_LIST_1, iRefIdx) << " " << rcMV.getHor() << " " << rcMV.getVer() << " ";
      }    

      /// Write Intra info
      Int iLumaIntraDir   = pcCU->getIntraDir(CHANNEL_TYPE_LUMA,   iOffset+iPartAddOffset);
      Int iChromaIntraDir = pcCU->getIntraDir(CHANNEL_TYPE_CHROMA, iOffset+iPartAddOffset);
      m_cIntraOutput << iLumaIntraDir << " " << iChromaIntraDir << " ";




    } /// PU end
  }
  else
  {
    m_cCUPUOutput << "99" << " ";     ///< CU info
    for( UInt i = 0; i < 4; i++ )
    {
      xWriteOutCUInfo  ( pcCU, iLength/4, iOffset+iLength/4*i, iDepth+1);
    }
  }
}


Void TSysuAnalyzerOutput::xWriteOutTUInfo  ( TComDataCU* pcCU, Int iLength, Int iOffset, UInt iDepth)
{
  UChar* puhTranIdx  = pcCU->getTransformIdx();
  if( puhTranIdx[iOffset] <= iDepth )
  {
    /// Write TU info
    m_cTUOutput << (Int)(puhTranIdx[iOffset]) << " ";
  }
  else
  {
    m_cTUOutput << "99" << " ";     
    for( UInt i = 0; i < 4; i++ )
    {
      xWriteOutTUInfo  ( pcCU, iLength/4, iOffset+iLength/4*i, iDepth+1);
    }
  }
}

Void TSysuAnalyzerOutput::writeOutSps   ( TComSPS* pcSPS )
{
  m_cSpsOut << "Resolution:"   << pcSPS->getPicWidthInLumaSamples() << "x" << pcSPS->getPicHeightInLumaSamples() << endl;

  m_cSpsOut << "Max CU Size:"  << pcSPS->getMaxCUHeight() << endl;
  m_cSpsOut << "Max CU Depth:" << pcSPS->getMaxTotalCUDepth() << endl;
  m_cSpsOut << "Max Inter TU Depth:" << pcSPS->getQuadtreeTUMaxDepthInter() << endl;
  m_cSpsOut << "Max Intra TU Depth:" << pcSPS->getQuadtreeTUMaxDepthIntra() << endl;

  int iInputBitDepth = pcSPS->getBitDepth(CHANNEL_TYPE_LUMA);

  m_cSpsOut << "Input Bit Depth:"  << iInputBitDepth  << endl;

  /// conformance window in luma samples (the YUV files are NOT cropped to it)
  const Window& rcConf = pcSPS->getConformanceWindow();
  m_cSpsOut << "Conformance Window:" << rcConf.getWindowLeftOffset() << " " << rcConf.getWindowRightOffset()
            << " " << rcConf.getWindowTopOffset() << " " << rcConf.getWindowBottomOffset() << endl;

}


// ---------------------------------------------------------------------------
// HM-16+ hooks
// ---------------------------------------------------------------------------

/// Called by TDecSlice right before decodeCtu(): remember the substream and
/// the CTU start position (same byte-based measure as the HM-12 version).
Void TSysuAnalyzerOutput::beginCtu( TComInputBitstream* pcBitstream )
{
  m_pcBitstream    = pcBitstream;
  m_uiCtuStartByte = pcBitstream->getByteLocation();
  m_uiCUStartByte  = m_uiCtuStartByte;
  aiCUBits.clear();
}

/// Called by TDecSlice right after decodeCtu()
Void TSysuAnalyzerOutput::endCtu( TComDataCU* pcCtu )
{
  pcCtu->getTotalBits() = m_pcBitstream->getByteLocation() - m_uiCtuStartByte;
}

/// Called by TDecCu::xDecodeCU when a leaf CU starts
Void TSysuAnalyzerOutput::beginCU()
{
  m_uiCUStartByte = m_pcBitstream->getByteLocation();
}

/// Called at the end of TDecCu::xFinishDecodeCU (every leaf CU ends there)
Void TSysuAnalyzerOutput::endCU()
{
  aiCUBits.push_back( m_pcBitstream->getByteLocation() - m_uiCUStartByte );
}

/// CU outside of the picture: 0 bit
Void TSysuAnalyzerOutput::addOutsideCU()
{
  aiCUBits.push_back( 0 );
}

/// Picture-sized buffer that lives as long as the TComPic it belongs to
/// (TComPic objects are recycled by the decoder, and so are these buffers).
TComPicYuv* TSysuAnalyzerOutput::xGetPicBuffer( TComPic* pcPic, Int iIdx )
{
  TComPicYuv*  pcRec  = pcPic->getPicYuvRec();
  TComPicYuv*& rpcBuf = m_acPicBuffers[iIdx][pcPic];

  if( rpcBuf != NULL &&
      ( rpcBuf->getWidth(COMPONENT_Y)  != pcRec->getWidth(COMPONENT_Y)  ||
        rpcBuf->getHeight(COMPONENT_Y) != pcRec->getHeight(COMPONENT_Y) ||
        rpcBuf->getChromaFormat()      != pcRec->getChromaFormat() ) )
  {
    rpcBuf->destroy();
    delete rpcBuf;
    rpcBuf = NULL;
  }

  if( rpcBuf == NULL )
  {
    const TComSPS& rcSPS = pcPic->getPicSym()->getSPS();
    rpcBuf = new TComPicYuv;
    rpcBuf->create( pcRec->getWidth(COMPONENT_Y), pcRec->getHeight(COMPONENT_Y), pcRec->getChromaFormat(),
                    rcSPS.getMaxCUWidth(), rcSPS.getMaxCUHeight(), rcSPS.getMaxTotalCUDepth(), true );

    for( UInt comp = 0; comp < rpcBuf->getNumberValidComponents(); comp++ )
    {
      const ComponentID compID = ComponentID(comp);
      Pel* piDst = rpcBuf->getAddr(compID);
      for( Int y = 0; y < rpcBuf->getHeight(compID); y++, piDst += rpcBuf->getStride(compID) )
      {
        for( Int x = 0; x < rpcBuf->getWidth(compID); x++ )
        {
          piDst[x] = 0;
        }
      }
    }
  }
  return rpcBuf;
}

TComPicYuv* TSysuAnalyzerOutput::getPredYuv( TComPic* pcPic )
{
  return xGetPicBuffer( pcPic, 0 );
}

TComPicYuv* TSysuAnalyzerOutput::getResiYuv( TComPic* pcPic )
{
  return xGetPicBuffer( pcPic, 1 );
}

Void TSysuAnalyzerOutput::storeIntraPred( TComDataCU* pcCU, ComponentID compID, UInt uiAbsPartIdx,
                                          const Pel* piPred, UInt uiStride, UInt uiWidth, UInt uiHeight )
{
  TComPicYuv* pcPredYuv  = getPredYuv( pcCU->getPic() );
  Pel*        piDst      = pcPredYuv->getAddr( compID, pcCU->getCtuRsAddr(), pcCU->getZorderIdxInCtu() + uiAbsPartIdx );
  const Int   iDstStride = pcPredYuv->getStride( compID );

  for( UInt y = 0; y < uiHeight; y++, piDst += iDstStride, piPred += uiStride )
  {
    for( UInt x = 0; x < uiWidth; x++ )
    {
      piDst[x] = piPred[x];
    }
  }
}

Void TSysuAnalyzerOutput::openYuvFiles( const Int outputBitDepth[MAX_NUM_CHANNEL_TYPE], const Int internalBitDepth[MAX_NUM_CHANNEL_TYPE] )
{
  /// residual: signed 16-bit samples, written without any scaling (as in HM-12)
  Int aiResiBitDepth[MAX_NUM_CHANNEL_TYPE];
  for( UInt ch = 0; ch < MAX_NUM_CHANNEL_TYPE; ch++ )
  {
    aiResiBitDepth[ch] = 16;
  }

  /// prediction: same format as the reconstruction file
  m_cPredYuvFile.open( "pred_yuv.yuv", true, outputBitDepth, outputBitDepth, internalBitDepth );
  m_cResiYuvFile.open( "resi_yuv.yuv", true, aiResiBitDepth, aiResiBitDepth, aiResiBitDepth );
  m_bYuvFilesOpened = true;
}

Void TSysuAnalyzerOutput::writeYuvFiles( TComPic* pcPic, const InputColourSpaceConversion ipCSC,
                                         Int confLeft, Int confRight, Int confTop, Int confBottom )
{
  if( !m_bYuvFilesOpened )
  {
    return;
  }
  m_cPredYuvFile.write( getPredYuv(pcPic), ipCSC, confLeft, confRight, confTop, confBottom );
  m_cResiYuvFile.write( getResiYuv(pcPic), IPCOLOURSPACE_UNCHANGED, confLeft, confRight, confTop, confBottom );
}

Void TSysuAnalyzerOutput::writeYuvFiles( TComPic* pcPicTop, TComPic* pcPicBottom, const InputColourSpaceConversion ipCSC,
                                         Int confLeft, Int confRight, Int confTop, Int confBottom, Bool isTff )
{
  if( !m_bYuvFilesOpened )
  {
    return;
  }
  m_cPredYuvFile.write( getPredYuv(pcPicTop), getPredYuv(pcPicBottom), ipCSC,
                        confLeft, confRight, confTop, confBottom, NUM_CHROMA_FORMAT, isTff );
  m_cResiYuvFile.write( getResiYuv(pcPicTop), getResiYuv(pcPicBottom), IPCOLOURSPACE_UNCHANGED,
                        confLeft, confRight, confTop, confBottom, NUM_CHROMA_FORMAT, isTff );
}

Void TSysuAnalyzerOutput::closeYuvFiles()
{
  if( m_bYuvFilesOpened )
  {
    m_cPredYuvFile.close();
    m_cResiYuvFile.close();
    m_bYuvFilesOpened = false;
  }
}

TSysuAnalyzerOutput::~TSysuAnalyzerOutput()
{
  m_cCUPUOutput.close();
  m_cMVOutput.close();
  m_cPredOutput.close();
  m_cSpsOut.close();
  m_cIntraOutput.close();
  m_cTUOutput.close();
  m_cBitOutputLCU.close();
  m_cBitOutputSCU.close();
  m_cMEOutput.close();

   m_cTileOutPut.close();
}
