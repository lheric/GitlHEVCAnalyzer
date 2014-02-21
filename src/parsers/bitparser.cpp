#include "bitparser.h"
#include <QRegExp>

BitParser::BitParser(QObject *parent) :
    QObject(parent)
{
}


bool BitParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
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
