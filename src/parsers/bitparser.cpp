#include "bitparser.h"
#include <QRegExp>

BitParser::BitParser(QObject *parent) :
    QObject(parent)
{
}


bool BitParser::parseLCUBitFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <0,0> 95
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<(-?[0-9]+),([0-9]+)> (.*)");

    int iDecOrder = -1;
    int iLastPOC  = -1;

    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            iDecOrder += (iLastPOC != iPoc);
            iLastPOC = iPoc;

            pcFrame = pcSequence->getFramesInDecOrder().at(iDecOrder);
            int iAddr = cMatchTarget.cap(2).toInt();
            pcLCU = pcFrame->getLCUs().at(iAddr);


            ///
            QString strBitInfo = cMatchTarget.cap(3);
            int iLCUBit = strBitInfo.toInt();
            pcLCU->setBitCount(iLCUBit);
            pcFrame->getBitCount() += iLCUBit;

        }

    }
    return true;
}



bool BitParser::parseSCUBitFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <0,8> 8 36 31 36 30 36 31 36 0 36 2 36 1 36 0 36 1 36
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<(-?[0-9]+),([0-9]+)> (.*)");
    QTextStream cSCUBitInfoStream;
    int iDecOrder = -1;
    int iLastPOC  = -1;
    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {
            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            iDecOrder += (iLastPOC != iPoc);
            iLastPOC = iPoc;

            pcFrame = pcSequence->getFramesInDecOrder().at(iDecOrder);
            int iAddr = cMatchTarget.cap(2).toInt();
            pcLCU = pcFrame->getLCUs().at(iAddr);


            ///
            QString strSCUBitInfo = cMatchTarget.cap(3);
            cSCUBitInfoStream.setString( &strSCUBitInfo, QIODevice::ReadOnly );
            xParseSCUBitFile(&cSCUBitInfoStream, pcLCU);

        }

    }
    return true;
}


bool BitParser::xParseSCUBitFile(QTextStream* pcSCUBitInfoStream, ComCU* pcCU)
{
    if( !pcCU->getSCUs().empty() )
    {
        /// non-leaf node : recursive reading for children
        xParseSCUBitFile(pcSCUBitInfoStream, pcCU->getSCUs().at(0));
        xParseSCUBitFile(pcSCUBitInfoStream, pcCU->getSCUs().at(1));
        xParseSCUBitFile(pcSCUBitInfoStream, pcCU->getSCUs().at(2));
        xParseSCUBitFile(pcSCUBitInfoStream, pcCU->getSCUs().at(3));
    }
    else
    {
        /// leaf node : read data
        Q_ASSERT(pcSCUBitInfoStream->atEnd()==false);
        int iSCUBit;
        *pcSCUBitInfoStream >> iSCUBit;
        pcCU->setBitCount(iSCUBit);
    }
    return true;
}
