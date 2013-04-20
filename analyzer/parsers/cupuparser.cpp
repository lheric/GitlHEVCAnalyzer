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
            pcLCU = new ComCU();
            pcLCU->setAddr(iAddr);

            /// recursively parse the CU&PU quard-tree structure
            QString strCUInfo = cMatchTarget.cap(3);
            cCUInfoStream.setString( &strCUInfo, QIODevice::ReadOnly );
            if( xReadInCUMode( &cCUInfoStream, pcLCU ) == false )
                return false;
            pcFrame->getLCUs().push_back(pcLCU);

            //debug code
            /*int ss[256];
            for(int i = 0; i < 256; i++) {
                ss[i] = pcLCU->getRecrMode()[i];
                cout << pcLCU->getRecrMode()[i] << " ";
            }
            cout << endl;*/

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
        /// non-leaf node : add 4 children CUs
        for(int i = 0; i < 4; i++)
        {
            ComCU* pcChildNode = new ComCU();
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
