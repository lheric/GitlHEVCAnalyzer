#include "decodergeneralparser.h"
#include <QRegExp>
#include <QTextStream>
#include <QDebug>

/// rearrange frames into POC ascending order
static bool xFrameSortingOrder(ComFrame* pcFrameFirst, ComFrame* pcFrameSecond)
{
    return (pcFrameFirst->getPOC() < pcFrameSecond->getPOC());
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


    // POC   18 TId: 0 ( B-SLICE, QP 33 ) [DT  0.005] [L0 17 16 15 14 ] [L1 17 16 15 14 ] [MD5:f2984e1c87633f1c2476eb2ddb844a53,(OK)]
    // read one frame

    ComFrame *pcFrame = NULL;
    cMatchTarget.setPattern("POC *(-?[0-9]+).*\\[DT *([0-9.]+) *\\] \\[L0(( -?[0-9]+){0,}) \\] \\[L1(( -?[0-9]+){0,}) \\]");
    /// no line is skipped here: HM-18 prints the first POC line right after the
    /// version line (HM-12 printed a blank line first); the loop below scans for
    /// POC lines and ignores everything else
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
            pcFrame->setPOC(cMatchTarget.cap(1).toInt());
            pcFrame->setTotalDecTime(cMatchTarget.cap(2).toDouble());

            /// L0 L1
            QString strL0, strL1;
            strL0 = cMatchTarget.cap(3); strL1 = cMatchTarget.cap(5);
            readIntArray(&pcFrame->getL0List(), &strL0);
            readIntArray(&pcFrame->getL1List(), &strL1);

            pcSequence->getFramesInDecOrder().push_back(pcFrame);
            pcSequence->getFramesInDisOrder().push_back(pcFrame);


        }
        else    // frames finish
        {

            pcSequence->setTotalFrames(pcSequence->getFramesInDisOrder().size());

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

    /// sort & calculate frame displaying order according to POC
    xSortByFrameCount(pcSequence);



    return true;
}

void DecoderGeneralParser::xSortByFrameCount( ComSequence* pcSequence )
{
    QVector<ComFrame*>& rcFrames = pcSequence->getFramesInDisOrder();
    if( rcFrames.isEmpty() )
        return;

    /// Get IDR groups: frames are in decoding order here; a group starts at the
    /// first frame and at every later frame whose POC is reset to 0 (IDR). The
    /// first frame need not be POC 0 (e.g. a stream starting with a CRA).
    QList<QVector<ComFrame*>::iterator> cIDRGroup;
    QVector<ComFrame*>::iterator cIter = rcFrames.begin();
    cIDRGroup.push_back(cIter);
    for( ++cIter; cIter != rcFrames.end(); ++cIter )
    {
        if ((*cIter)->getPOC() == 0)
        {
            cIDRGroup.push_back(cIter);
        }
    }
    cIDRGroup.push_back(rcFrames.end());

    /// sort each IDRGroup
    for(int i = 0; i < cIDRGroup.size()-1; i++)
        qSort(cIDRGroup[i], cIDRGroup[i+1], xFrameSortingOrder);

    /// assign frame num
    for(int i = 0; i < pcSequence->getFramesInDisOrder().size(); i++)
    {
        pcSequence->getFramesInDisOrder().at(i)->setFrameCount(i);
    }
}
