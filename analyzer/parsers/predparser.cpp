#include "predparser.h"

PredParser::PredParser(QObject *parent) :
    QObject(parent)
{
}

bool PredParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <1,1> 0 0 1 1 0
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComLCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*) ");
    QTextStream cPredInfoStream;
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
            QString strCUInfo = cMatchTarget.cap(3);
            cPredInfoStream.setString(&strCUInfo, QIODevice::ReadOnly );

            QVector<int>& piPredType = pcLCU->getPredType();

            while( !cPredInfoStream.atEnd() )
            {
                int iPredType;
                cPredInfoStream >> iPredType;
                piPredType.push_back(iPredType);
            }

        }

    }
    return true;
}
