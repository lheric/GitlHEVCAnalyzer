#include "mvparser.h"

MVParser::MVParser(QObject *parent) :
    QObject(parent)
{
}

bool MVParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <1,1> 1 -3 0 1 -3 0 1 -3 0 1 1 0 1 -3 0
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*) ");
    QTextStream cMVInfoStream;
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
            QString strMVInfo = cMatchTarget.cap(3);
            cMVInfoStream.setString( &strMVInfo, QIODevice::ReadOnly );

            QVector<ComMV*>& pcMVs = pcLCU->getMVs();
            QVector<int>& pcInterDir = pcLCU->getInterDir();

            while( !cMVInfoStream.atEnd() )
            {
                ComMV* pcReadMV = NULL;
                int iInterDir, iHor, iVer;
                cMVInfoStream >> iInterDir;
                pcInterDir.push_back(iInterDir);

                if( iInterDir == 1 || iInterDir == 2)   //uni-prediction, 1 MV
                {
                    cMVInfoStream >> iHor >> iVer;
                    pcReadMV = new ComMV();
                    pcReadMV->setHor(iHor);
                    pcReadMV->setVer(iVer);
                    pcMVs.push_back(pcReadMV);
                }
                else if( iInterDir == 3 )               //bi-prediction, 2 MVs
                {
                    cMVInfoStream >> iHor >> iVer;
                    pcReadMV = new ComMV();
                    pcReadMV->setHor(iHor);
                    pcReadMV->setVer(iVer);
                    pcMVs.push_back(pcReadMV);

                    cMVInfoStream >> iHor >> iVer;
                    pcReadMV = new ComMV();
                    pcReadMV->setHor(iHor);
                    pcReadMV->setVer(iVer);
                    pcMVs.push_back(pcReadMV);
                }


            }

        }

    }
    return true;
}
