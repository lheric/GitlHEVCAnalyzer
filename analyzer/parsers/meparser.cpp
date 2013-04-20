#include "meparser.h"
#include "../io/analyzermsgsender.h"


MEParser::MEParser(QObject *parent) :
    QObject(parent)
{
}

bool MEParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    /*Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    /// <1,1> 0 8694 0 8694 0 8227 0 2459 0 8694
    /// read SP and SAD for one PU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    ComMV* pcMV = NULL;
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*) ");
    QTextStream cMEInfoStream;
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

            QVector<ComMV*>& cMVs = pcLCU->getMVs();
            int iMVNum = cMVs.size();
            int iMVIndex = 0;

            ///
            QString strMEInfo = cMatchTarget.cap(3);
            cMEInfoStream.setString( &strMEInfo, QIODevice::ReadOnly );
            while( !cMEInfoStream.atEnd() )
            {
                Q_ASSERT(iMVIndex < iMVNum);
                pcMV = cMVs.at(iMVIndex);
                int iFMESP, iSAD;     ///< Fractional Motion Estimation Search Point number & SAD
                cMEInfoStream >> iFMESP >> iSAD;
                // TODO
//                pcMV->setFMESP(iFMESP);
//                pcMV->setSAD(iSAD);
                iMVIndex++;
            }
            if(iMVIndex != iMVNum)
                AnalyzerMsgSender::getInstance()->msgOut("MEParser: ME info length not match", GITL_MSG_ERROR);


        }

    }*/
    return true;
}
