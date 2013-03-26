#include "mergeparser.h"

MergeParser::MergeParser(QObject *parent) :
    QObject(parent)
{
}
bool MergeParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <1,0> -1 0 0 -1 1
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComLCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*) ");
    QTextStream cMergeInfoStream;
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
            QString strMergeInfo = cMatchTarget.cap(3);
            cMergeInfoStream.setString( &strMergeInfo, QIODevice::ReadOnly );


            QVector<int>& piMergeIndex = pcLCU->getMergeIndex();

            while( !cMergeInfoStream.atEnd() )
            {
                int iMergeIndex;
                cMergeInfoStream >> iMergeIndex;
                piMergeIndex.push_back(iMergeIndex);
            }

        }

    }
    return true;
}
