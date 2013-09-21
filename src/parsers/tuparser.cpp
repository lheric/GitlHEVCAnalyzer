#include "tuparser.h"
#include <QRegExp>
#include <QDebug>

#define TU_SLIPT_FLAG 99

TUParser::TUParser(QObject *parent):
    QObject(parent)
{
}


bool TUParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <1,1> 1 -3 0 1 -3 0 1 -3 0 1 1 0 1 -3 0
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*)");
    QTextStream cTUInfoStream;
    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            pcFrame = pcSequence->getFrames().at(iPoc);
            int iAddr = cMatchTarget.cap(2).toInt();
            pcLCU = pcFrame->getLCUs().at(iAddr);


            ///
            QString strTUInfo = cMatchTarget.cap(3);
            cTUInfoStream.setString( &strTUInfo, QIODevice::ReadOnly );


            xReadTU(&cTUInfoStream, pcLCU);

        }

    }
    return true;
}


bool TUParser::xReadTU(QTextStream* pcTUInfoStream, ComCU* pcCU)
{
    if( !pcCU->getSCUs().empty() )
    {
        /// non-leaf CU node : continue to leaf CU
        xReadTU(pcTUInfoStream, pcCU->getSCUs().at(0));
        xReadTU(pcTUInfoStream, pcCU->getSCUs().at(1));
        xReadTU(pcTUInfoStream, pcCU->getSCUs().at(2));
        xReadTU(pcTUInfoStream, pcCU->getSCUs().at(3));
    }
    else
    {
        /// leaf CU node : read TU
        ComTU* pcTURoot = &pcCU->getTURoot();
        pcTURoot->setX(pcCU->getX());
        pcTURoot->setY(pcCU->getY());
        pcTURoot->setSize(pcCU->getSize());
        xReadTUHelper(pcTUInfoStream, &(pcCU->getTURoot()));
    }
    return true;
}


bool TUParser::xReadTUHelper(QTextStream* pcCUInfoStream, ComTU* pcTU)
{
    int iTUMode;
    if( pcCUInfoStream->atEnd() )
    {
        qCritical() << "TUParser Error! Illegal TU Mode!";
        return false;
    }
    *pcCUInfoStream >> iTUMode;

    if( iTUMode == TU_SLIPT_FLAG )
    {
        /// non-leaf node : add 4 children CUs
        for(int i = 0; i < 4; i++)
        {
            ComTU* pcChildNode = new ComTU();
            pcChildNode->setSize(pcTU->getSize()/2);
            int iSubCUX = pcTU->getX() + i%2 * (pcTU->getSize()/2);
            int iSubCUY = pcTU->getY() + i/2 * (pcTU->getSize()/2);
            pcChildNode->setX(iSubCUX);
            pcChildNode->setY(iSubCUY);
            pcTU->getTUs().push_back(pcChildNode);
            xReadTUHelper(pcCUInfoStream, pcChildNode);
        }
    }
    else
    {
        /// leaf TU node : DO NOTHING
    }
    return true;
}
