#include "cupuparser.h"

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

            ///
            QString strCUInfo = cMatchTarget.cap(3);
            cCUInfoStream.setString( &strCUInfo, QIODevice::ReadOnly );
            QVector<int>& piCUMode = pcLCU->getCUPUMode();
            xReadInCUMode( &cCUInfoStream, piCUMode );
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



bool CUPUParser::xReadInCUMode(QTextStream* pcCUInfoStream, QVector<int>& piCUMode)
{
    int iCUMode;
    *pcCUInfoStream >> iCUMode;
    piCUMode.push_back(iCUMode);
    if( iCUMode == CU_SLIPT_FLAG )
    {

        for(int i = 0; i < 4; i++)
        {
            xReadInCUMode(pcCUInfoStream, piCUMode);
        }
    }
    return true;
}
