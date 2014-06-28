#include "tileparser.h"
#include <QRegExp>


TileParser::TileParser(QObject *parent) :
    QObject(parent)
{

}

bool TileParser::parseFile(QTextStream *pcInputStream, ComSequence *pcSequence)
{
    Q_ASSERT(pcSequence != NULL);

    QString strOneLine;
    QRegExp cMatchTarget;

    ///<1,1> 0 4 4
    ///read one tile
    ComFrame * pcFrame = NULL;

    cMatchTarget.setPattern("^<(-?[0-9]+),([0-9]+)> (.*)");

    QTextStream cTileInfoStream;
    int iDecOrder = -1;
    int iLastPOC = -1;


    while(!pcInputStream->atEnd())
    {
        strOneLine = pcInputStream->readLine();
        if(cMatchTarget.indexIn(strOneLine) != -1)
        {
            ///poc addr and iTileNum
            int iPoc = cMatchTarget.cap(1).toInt();
            iDecOrder += (iLastPOC != iPoc);
            iLastPOC = iPoc;

            pcFrame = pcSequence->getFramesInDecOrder().at(iDecOrder);

            int iTileNum = cMatchTarget.cap(2).toInt();
            //pcFrame->m_iTileNum = iTileNum;

            QString strTileInfo = cMatchTarget.cap(3);
            cTileInfoStream.setString( &strTileInfo, QIODevice::ReadOnly);

            xReadTile(&cTileInfoStream, pcFrame);
        }

    }

    return true;
}



bool TileParser::xReadTile(QTextStream *pcTileInfoStream, ComFrame *pcFrame)
{

    int iTempVal = 0;
    ComTile * pcTile = NULL;
    pcTile = new ComTile(pcFrame);

    Q_ASSERT( !pcTileInfoStream->atEnd() );
    *pcTileInfoStream >> iTempVal;
    pcTile->setFirstCUAddr(iTempVal);

    Q_ASSERT( !pcTileInfoStream->atEnd() );
    *pcTileInfoStream >> iTempVal;
    pcTile->setWidth(iTempVal);

    Q_ASSERT( !pcTileInfoStream ->atEnd() );
    *pcTileInfoStream >> iTempVal;
    pcTile->setHeight(iTempVal);

    pcFrame->getTiles().push_back(pcTile);

    return true;

}

