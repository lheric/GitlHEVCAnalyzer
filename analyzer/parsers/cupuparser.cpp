#include "cupuparser.h"
#include "io/analyzermsgsender.h"
#include <QTextStream>
#include <iostream>
using namespace std;

CUPUParser::CUPUParser(QObject *parent) :
    QObject(parent)
{
}


bool CUPUParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <1,1> 99 0 0 5 0
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*) ");
    QTextStream cCUInfoStream;
    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            pcFrame = pcSequence->getFrames().at(iPoc);
            int iAddr = cMatchTarget.cap(2).toInt();
            pcLCU = new ComCU(pcFrame);
            pcLCU->setAddr(iAddr);
            pcLCU->setFrame(pcFrame);
            pcLCU->setDepth(0);
            pcLCU->setZorder(0);

            /// recursively parse the CU&PU quard-tree structure
            QString strCUInfo = cMatchTarget.cap(3);
            cCUInfoStream.setString( &strCUInfo, QIODevice::ReadOnly );
            if( xReadInCUMode( &cCUInfoStream, pcLCU ) == false )
                return false;
            pcFrame->getLCUs().push_back(pcLCU);
        }

    }
    return true;
}



bool CUPUParser::xReadInCUMode(QTextStream* pcCUInfoStream, ComCU* pcCU)
{
    int iCUMode;
    if( pcCUInfoStream->atEnd() )
    {
        AnalyzerMsgSender::getInstance()->msgOut("CUPUParser Error! Illegal CU/PU Mode!", GITL_MSG_FATAL);
        return false;
    }
    *pcCUInfoStream >> iCUMode;

    if( iCUMode == CU_SLIPT_FLAG )
    {
        int iTotalNumPart = 1 << ((pcCU->getFrame()->getSequence()->getMaxCUDepth()-pcCU->getDepth()) << 1);
        /// non-leaf node : add 4 children CUs
        for(int i = 0; i < 4; i++)
        {
            ComCU* pcChildNode = new ComCU(pcCU->getFrame());
            pcChildNode->setAddr(pcCU->getAddr());
            pcChildNode->setDepth(pcCU->getDepth()+1);
            pcChildNode->setZorder( pcCU->getZorder() + (iTotalNumPart/4)*i );
            pcCU->getSCUs().push_back(pcChildNode);
            xReadInCUMode(pcCUInfoStream, pcChildNode);
        }
    }
    else
    {
        /// leaf node : create PUs and write the PU Mode for it
        pcCU->setPartSize((PartSize)iCUMode);

        int iPUCount = ComCU::getPUNum((PartSize)iCUMode);
        for(int i = 0; i < iPUCount; i++)
        {
            pcCU->getPUs().push_back(new ComPU());
        }

    }
    return true;
}
