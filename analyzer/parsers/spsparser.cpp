#include "spsparser.h"

SpsParser::SpsParser(QObject *parent) :
    QObject(parent)
{
}

/** Resolution:176x144
  * Max CU Size:64
  * Max CU Depth:4
  * Min TU Depth:0
  * Max TU Depth:1
  *
  */
bool SpsParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{

    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    // Resolution:176x144
    cMatchTarget.setPattern("Resolution:([0-9]+)x([0-9]+)");
    while( !pcInputStream->atEnd() ) {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iWidth  = cMatchTarget.cap(1).toInt();
            int iHeight = cMatchTarget.cap(2).toInt();
            pcSequence->setWidth(iWidth);
            pcSequence->setHeight(iHeight);
            break;
        }
    }



    // Max CU Size:64
    cMatchTarget.setPattern("Max CU Size:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iMaxCUSize = cMatchTarget.cap(1).toInt();
            pcSequence->setMaxCUSize(iMaxCUSize);
            break;
        }
    }

    // Max CU Depth:4
    cMatchTarget.setPattern("Max CU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iMaxCUDepth = cMatchTarget.cap(1).toInt();
            pcSequence->setMaxCUDepth(iMaxCUDepth);
            break;
        }
    }

    // Min TU Depth:0
    cMatchTarget.setPattern("Min TU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iMinTUDepth = cMatchTarget.cap(1).toInt();
            pcSequence->setMinTUDepth(iMinTUDepth);
            break;
        }
    }

    // Max TU Depth:1
    cMatchTarget.setPattern("Max TU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iMaxTUDepth = cMatchTarget.cap(1).toInt();
            pcSequence->setMaxTUDepth(iMaxTUDepth);
            break;
        }
    }


    return true;
}
