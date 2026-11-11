#pragma once
#include <fstream>
#include <string>
#include <vector>


#include "TLibCommon/TComDataCU.h"
#include "TLibCommon/TComSlice.h"
#include "TLibDecoder/TDecCu.h"

#include <map>
#include "TLibCommon/TComPic.h"
#include "TLibCommon/TComPicYuv.h"
#include "TLibCommon/TComBitStream.h"
#include "Utilities/TVideoIOYuv.h"



class TSysuAnalyzerOutput
{
public:
  TSysuAnalyzerOutput(void);
  ~TSysuAnalyzerOutput(void);

#if 0
  /// write out residual
  Void writeOutResiYUV (TDecCu* pcDecCU, TComDataCU* pcCU, UInt uiZorderIdx, UInt uiDepth);
#endif
 

  /// splitting mode
  Void writeOutCUInfo   ( TComDataCU* pcCU );
  Void xWriteOutCUInfo  ( TComDataCU* pcCU, Int iLength, Int iOffset, UInt iDepth);
  Void xWriteOutTUInfo  ( TComDataCU* pcCU, Int iLength, Int iOffset, UInt iDepth);
  
  /// Sequence parameter set output
  Void writeOutSps         ( TComSPS* pcSPS );


  ///write out tile info
  Void writeOutTileInfo(TComPic * pcPic);



  std::vector<int> aiCUBits;

  /// ---- HM-16+ hooks (HM-12 changed the TDecCu signatures instead) ----

  /// bit (byte) consumption per CTU / leaf CU, called from TDecSlice / TDecCu
  Void beginCtu     ( TComInputBitstream* pcBitstream );
  Void endCtu       ( TComDataCU* pcCtu );
  Void beginCU      ();
  Void endCU        ();
  Void addOutsideCU ();

  /// per-picture predicted & residual buffers (HM-16+ TComPic has none)
  TComPicYuv* getPredYuv ( TComPic* pcPic );
  TComPicYuv* getResiYuv ( TComPic* pcPic );
  /// store one intra prediction block before it is overwritten by the reconstruction
  Void storeIntraPred    ( TComDataCU* pcCU, ComponentID compID, UInt uiAbsPartIdx,
                           const Pel* piPred, UInt uiStride, UInt uiWidth, UInt uiHeight );

  /// pred_yuv.yuv / resi_yuv.yuv output, called from TAppDecTop next to the reconstruction output
  Void openYuvFiles  ( const Int outputBitDepth[MAX_NUM_CHANNEL_TYPE], const Int internalBitDepth[MAX_NUM_CHANNEL_TYPE] );
  Void writeYuvFiles ( TComPic* pcPic, const InputColourSpaceConversion ipCSC,
                       Int confLeft, Int confRight, Int confTop, Int confBottom );
  Void writeYuvFiles ( TComPic* pcPicTop, TComPic* pcPicBottom, const InputColourSpaceConversion ipCSC,
                       Int confLeft, Int confRight, Int confTop, Int confBottom, Bool isTff );
  Void closeYuvFiles ();
  /*
  Void setAllDepthTo       ( TComDataCU* pcCU, UInt uiDepth );
  Bool compareSplitMode    ( TComDataCU* pcRecursive, TComDataCU* pcFast );

  Void printCUModeForLCU   ( TComDataCU* pcCU, Char* phMessage );
  Void xPrintCUModeForLCU  ( UChar* pcCU, Int iLength, UInt iDepth );
  */



  /// SINGLETON 
  static TSysuAnalyzerOutput* getInstance() { if( m_instance == NULL ) m_instance = new TSysuAnalyzerOutput(); return m_instance;}

private:

  /// Decoder output ( extracted from bitstream )
  std::ofstream m_cSpsOut;          ///< SPS info
  std::ofstream m_cPredOutput;      ///< Prediction mode info output
  std::ofstream m_cCUPUOutput;      ///< CU info output
  std::ofstream m_cMVOutput;        ///< MV info output
  std::ofstream m_cMergeOutput;     ///< Merge info output
  std::ofstream m_cIntraOutput;     ///< Intra info output
  std::ofstream m_cTUOutput;        ///< TU info output
  std::ofstream m_cBitOutputLCU;    ///< LCU bit consumption info output
  std::ofstream m_cBitOutputSCU;    ///< SCU bit consumption info output
  std::ofstream m_cTileOutPut;

  /// Encoder output ( extracted in the encoding process
  std::ofstream m_cMEOutput;    ///< ME info (search point number, SAD, cost, etc)

  TComPicYuv* xGetPicBuffer ( TComPic* pcPic, Int iIdx );

  TComInputBitstream* m_pcBitstream;   ///< substream of the CTU being decoded
  UInt                m_uiCtuStartByte;
  UInt                m_uiCUStartByte;
  std::map< TComPic*, TComPicYuv* > m_acPicBuffers[2];  ///< [0] predicted, [1] residual
  TVideoIOYuv         m_cPredYuvFile;
  TVideoIOYuv         m_cResiYuvFile;
  Bool                m_bYuvFilesOpened;

  static TSysuAnalyzerOutput* m_instance;

};
