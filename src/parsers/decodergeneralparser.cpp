#include "decodergeneralparser.h"
#include <QRegExp>
#include <QTextStream>
#include <QDebug>

/// for frame sorting in POC ascending order
static bool xFrameSortingOrder(const ComFrame* pcFrameFirst, const ComFrame* pcFrameSecond)
{
    return (*pcFrameFirst < *pcFrameSecond);
}


DecoderGeneralParser::DecoderGeneralParser(QObject *parent) :
    QObject(parent)
{
}

static void readIntArray(QVector<int>* paiArr, QString* pstrArrStr)
{
    QString strTrimmed = pstrArrStr->trimmed();
    QTextStream strTextStream(&strTrimmed);
    int iPOC;
    while(!strTextStream.atEnd())
    {
        strTextStream >> iPOC;
        paiArr->push_back(iPOC);
    }
}

bool DecoderGeneralParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    //HM software: Decoder Version [5.2][Windows][VS 1500][32 bit]
    cMatchTarget.setPattern("HM software: Decoder Version \\[([0-9.]+)\\].*");
    while( !pcInputStream->atEnd() ) {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            QString strEncoderVersion = cMatchTarget.cap(1);
            pcSequence->setEncoderVersion(strEncoderVersion);
            break;
        }
    }


    // POC   18 TId: 0 ( B-SLICE, QP 33 ) [DT  0.005] [L0 17 16 15 14 ] [L1 17 16 15 14 ] [LC 17 16 15 14 ] [MD5:f2984e1c87633f1c2476eb2ddb844a53,(OK)]
    // read one frame

    ComFrame *pcFrame = NULL;
    cMatchTarget.setPattern("POC *([0-9]+).*\\[DT *([0-9.]+) *\\] \\[L0(( [0-9]+){0,}) \\] \\[L1(( [0-9]+){0,}) \\] (\\[LC(( [0-9]+){0,}) \\])?");
    pcInputStream->readLine();///< Skip a empty line
    while( !pcInputStream->atEnd() )
    {

        while( !pcInputStream->atEnd() )
        {
            strOneLine = pcInputStream->readLine();
            if(cMatchTarget.indexIn(strOneLine) != -1)
                break;
        }
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            pcFrame = new ComFrame(pcSequence);

            /// POC & Decoding time
            pcFrame->setPoc(cMatchTarget.cap(1).toInt());
            pcFrame->setTotalDecTime(cMatchTarget.cap(2).toDouble());

            /// L0 L1 LC
            QString strL0, strL1, strLC;
            strL0 = cMatchTarget.cap(3); strL1 = cMatchTarget.cap(5); strLC = cMatchTarget.cap(7);
            readIntArray(&pcFrame->getL0List(), &strL0);
            readIntArray(&pcFrame->getL1List(), &strL1);
            readIntArray(&pcFrame->getLCList(), &strLC);

            pcSequence->getFrames().push_back(pcFrame);


        }
        else    // frames finish
        {

            pcSequence->setTotalFrames(pcSequence->getFrames().size());

            // Total Time:     2070.335 sec.

            cMatchTarget.setPattern(" Total Time: *([0-9.]+).*");

            {
                strOneLine = pcInputStream->readLine();
                if( cMatchTarget.indexIn(strOneLine) != -1 ) {
                    pcSequence->setTotalDecTime(cMatchTarget.cap(1).toDouble());
                    break;
                }
            }
        }

    }


    /// sort frames in POC ascending order
    qSort(pcSequence->getFrames().begin(), pcSequence->getFrames().end(), xFrameSortingOrder);


    return true;
}
