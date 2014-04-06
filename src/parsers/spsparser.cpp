#include "spsparser.h"
#include <QRegExp>

SpsParser::SpsParser(QObject *parent) :
    QObject(parent)
{
}

/** --- SAMPLE TEXT ---
  * Resolution:176x144
  * Max CU Size:64
  * Max CU Depth:4
  * Min TU Depth:0
  * Max TU Depth:1
  * Input Bit Depth:8
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

    // Max Inter TU Depth:3
    cMatchTarget.setPattern("Max Inter TU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iMinTUDepth = cMatchTarget.cap(1).toInt();
            pcSequence->setMaxInterTUDepth(iMinTUDepth);
            break;
        }
    }

    // Max Intra TU Depth:3
    cMatchTarget.setPattern("Max Intra TU Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iMaxTUDepth = cMatchTarget.cap(1).toInt();
            pcSequence->setMaxIntraTUDepth(iMaxTUDepth);
            break;
        }
    }

    // Input Bit Depth:8
    cMatchTarget.setPattern("Input Bit Depth:([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iInputBitDepth = cMatchTarget.cap(1).toInt();
            pcSequence->setInputBitDepth(iInputBitDepth);
            break;
        }
    }

    return true;
}
