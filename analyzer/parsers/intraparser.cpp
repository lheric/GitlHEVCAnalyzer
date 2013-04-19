#include "intraparser.h"

IntraParser::IntraParser(QObject *parent) :
    QObject(parent)
{
}
bool IntraParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <0,8> 8 36 31 36 30 36 31 36 0 36 2 36 1 36 0 36 1 36
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*) ");
    QTextStream cIntraDirInfoStream;
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
            QString strIntraDir = cMatchTarget.cap(3);
            cIntraDirInfoStream.setString( &strIntraDir, QIODevice::ReadOnly );


            QVector<int>& piIntraDir = pcLCU->getIntraDir();

            while( !cIntraDirInfoStream.atEnd() )
            {
                int iIntraDir;
                cIntraDirInfoStream >> iIntraDir;
                piIntraDir.push_back(iIntraDir);
            }

        }

    }
    return true;
}
