#include "TComSysuCuMDTools.h"

TComSysuCuMDTools* TComSysuCuMDTools::m_instance = NULL;

TComSysuCuMDTools::TComSysuCuMDTools( std::string cInFilename, std::string cOutFilename )
{
    m_cOutputFile.open(cOutFilename.c_str(), ios::app);
    m_cInputFile.open(cInFilename.c_str(), ios::in);

    m_uiDepthDiff = 0;
    m_uiSame = 0;
    m_uiTotal = 0;
}


void TComSysuCuMDTools::writeOutSplitMode   ( TComDataCU* pcCU )
{
  Int iTotalNumPart = pcCU->getTotalNumPart();
  UChar* puhDepth = pcCU->getDepth();
  xWriteOutSplitMode  ( puhDepth, iTotalNumPart, 0 );
  m_cOutputFile << endl;
}

void TComSysuCuMDTools::xWriteOutSplitMode  ( UChar* pcCU, Int iLength, UInt iDepth )
{
  if( pcCU[0] == iDepth )
  {
    m_cOutputFile << "0" << " ";
  }
  else
  {
    m_cOutputFile << "1" << " ";
    for( UInt i = 0; i < 4; i++ )
    {
      xWriteOutSplitMode  ( pcCU+iLength/4*i, iLength/4, iDepth+1 );
    }
  }
}

void TComSysuCuMDTools::setAllDepthTo ( TComDataCU* pcCU, UInt uiDepth )
{
    for( UInt i = 0; i < pcCU->getTotalNumPart(); i++ )
    {
        *(pcCU->getDepth()+i) = uiDepth;
    }
}

Bool TComSysuCuMDTools::compareSplitMode ( TComDataCU* pcRecursive, TComDataCU* pcFast )
{

    UChar* puhFirCU = pcRecursive->getDepth();
//    Double dFirCost = pcRecursive->getTotalCost();
    UChar* puhSecCU = pcFast->getDepth();
//    Double dSecCost = pcFast->getTotalCost();
    UInt uiTotalNumPart = pcFast->getTotalNumPart();

//    Int iPoc = pcRecursive->getSlice()->getPOC();
//    Int iCuAddr = pcRecursive->getAddr();

    Bool bIsSame = true;
    
    UInt64 uiDepthDiffOld = m_uiDepthDiff;
    for( UInt i = 0; i < uiTotalNumPart; i++ )
    {
        m_uiDepthDiff += abs( puhFirCU[i] - puhSecCU[i] );        
    }
    
    bIsSame = ( uiDepthDiffOld == m_uiDepthDiff );
    
    ++m_uiTotal;
    if(bIsSame)
    {
        ++m_uiSame;
    }

    return bIsSame;
}

Void TComSysuCuMDTools::printCUModeForLCU   ( TComDataCU* pcCU, Char* phMessage, bool bCost )
{
  if( bCost ) 
    cout << phMessage << "Cost: " << pcCU->getTotalCost() << endl;
  UInt uiTotalNumPart = pcCU->getTotalNumPart();
  UChar* puhDepth = pcCU->getDepth();
  xPrintCUModeForLCU  ( puhDepth, uiTotalNumPart, 0 );
  cout << endl;
}

Void TComSysuCuMDTools::xPrintCUModeForLCU   ( UChar* pcCU, Int iLength, UInt iDepth )
{

  if( pcCU[0] == iDepth )
  {
    cout << "0" << " ";
  }
  else
  {
    cout << "1" << " ";
    for( UInt i = 0; i < 4; i++ )
    {
      xPrintCUModeForLCU  ( pcCU+iLength/4*i, iLength/4, iDepth+1 );
    }
  }
}

Void TComSysuCuMDTools::printCUStatistic    ()
{
    cout << "Same CU Mode Percentage : " << (Double)m_uiSame/m_uiTotal << endl;
    cout << "Mean CU Depth Error : " << (Double)m_uiDepthDiff/m_uiTotal << endl;
}

void TComSysuCuMDTools::readInSplitMode( TComDataCU* pcCU )
{
  Int iTotalNumPart = pcCU->getTotalNumPart();
  UChar* puhDepth = pcCU->getDepth();
  xReadInSplitMode( puhDepth, iTotalNumPart, 0 );
  m_cOutputFile << endl;

}
void TComSysuCuMDTools::xReadInSplitMode  ( UChar* pcCU, Int iLength, UInt iDepth )
{
  Int iReadDepth;
  m_cInputFile >> iReadDepth;
  if( iReadDepth == 0 )
  {
    memset( pcCU, iDepth, iLength );
  }
  else
  {
    for( UInt i = 0; i < 4; i++ )
    {
      xReadInSplitMode  ( pcCU+iLength/4*i, iLength/4, iDepth+1 );
    }
  }
}

TComSysuCuMDTools::~TComSysuCuMDTools()
{
    m_cOutputFile.close();
    m_cInputFile.close();
}
