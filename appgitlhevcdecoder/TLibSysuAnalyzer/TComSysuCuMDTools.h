#ifndef _TCOMSYSUCUMDTOOLS__
#define _TCOMSYSUCUMDTOOLS__

#include <fstream>
#include <string>

#include "TLibCommon/TComDataCU.h"

class TComSysuCuMDTools
{
public:
    TComSysuCuMDTools( std::string cInFilename, std::string cOutFilename );
    ~TComSysuCuMDTools();
    // read & write splitting mode
    Void setInputFileName    ( std::string cFilename )  { m_cInputFilename = cFilename; }
    Void setOutputFileName   ( std::string cFilename )  { m_cOutputFilename = cFilename; }
    Void writeOutSplitMode   ( TComDataCU* pcCU );
    Void xWriteOutSplitMode  ( UChar* pcCU, Int iLength, UInt iDepth );
    
    Void readInSplitMode     ( TComDataCU* pcCU );
    Void xReadInSplitMode    ( UChar* pcCU, Int iLength, UInt iDepth );
    
    Void setAllDepthTo       ( TComDataCU* pcCU, UInt uiDepth );
    Bool compareSplitMode    ( TComDataCU* pcRecursive, TComDataCU* pcFast );

    Void printCUModeForLCU   ( TComDataCU* pcCU, Char* phMessage, bool bCost = true);
    Void xPrintCUModeForLCU  ( UChar* pcCU, Int iLength, UInt iDepth );

    Void printCUStatistic    ();
   
private:
    std::string m_cInputFilename;
    std::string m_cOutputFilename;
    std::ifstream m_cInputFile;
    std::ofstream m_cOutputFile;
    static TComSysuCuMDTools* m_instance;

    UInt64 m_uiDepthDiff;
    UInt64 m_uiSame;
    UInt64 m_uiTotal;
};

#endif
