#include "cupuparser.h"
#include "io/analyzermsgsender.h"
#include <QTextStream>
#include <QRegExp>
#include <QtAlgorithms>

#define CU_SLIPT_FLAG 99      ///< CU splitting flag in file

/// for CU sorting in Addr ascending order
static bool xCUSortingOrder(const ComCU* pcCUFirst, const ComCU* pcCUSecond)
{
    return (*pcCUFirst < *pcCUSecond);
}

CUPUParser::CUPUParser(QObject *parent) :
    QObject(parent)
{
}


bool CUPUParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    ///
    int iSeqWidth = pcSequence->getWidth();
    int iMaxCUSize = pcSequence->getMaxCUSize();
    int iCUOneRow = (iSeqWidth+iMaxCUSize-1)/iMaxCUSize;

    ////
    QString strOneLine;
    QRegExp cMatchTarget;


    /// <1,1> 99 0 0 5 0
    /// read one LCU
    ComFrame* pcFrame = NULL;
    ComCU* pcLCU = NULL;
    int iLCUSize = pcSequence->getMaxCUSize();
    cMatchTarget.setPattern("^<([0-9]+),([0-9]+)> (.*) ");
    QTextStream cCUInfoStream;
    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            pcFrame = pcSequence->getFrames().at(iPoc);
            int iAddr = cMatchTarget.cap(2).toInt();
            pcLCU = new ComCU(pcFrame);
            pcLCU->setAddr(iAddr);
            pcLCU->setFrame(pcFrame);
            pcLCU->setDepth(0);
            pcLCU->setZorder(0);
            pcLCU->setSize(iLCUSize);
            int iPixelX = (pcLCU->getAddr()%iCUOneRow)*iMaxCUSize;
            int iPixelY = (pcLCU->getAddr()/iCUOneRow)*iMaxCUSize;
            pcLCU->setX(iPixelX);
            pcLCU->setY(iPixelY);

            /// recursively parse the CU&PU quard-tree structure
            QString strCUInfo = cMatchTarget.cap(3);
            cCUInfoStream.setString( &strCUInfo, QIODevice::ReadOnly );
            if( xReadInCUMode( &cCUInfoStream, pcLCU ) == false )
                return false;
            pcFrame->getLCUs().push_back(pcLCU);
        }

    }

    qSort(pcFrame->getLCUs().begin(), pcFrame->getLCUs().end(), xCUSortingOrder);

    return true;
}



bool CUPUParser::xReadInCUMode(QTextStream* pcCUInfoStream, ComCU* pcCU)
{
    int iCUMode;
    if( pcCUInfoStream->atEnd() )
    {
        qCritical() << "CUPUParser Error! Illegal CU/PU Mode!";
        return false;
    }
    *pcCUInfoStream >> iCUMode;

    if( iCUMode == CU_SLIPT_FLAG )
    {
        int iMaxDepth = pcCU->getFrame()->getSequence()->getMaxCUDepth();
        int iTotalNumPart = 1 << ( (iMaxDepth-pcCU->getDepth()) << 1 );
        /// non-leaf node : add 4 children CUs
        for(int i = 0; i < 4; i++)
        {
            ComCU* pcChildNode = new ComCU(pcCU->getFrame());
            pcChildNode->setAddr(pcCU->getAddr());
            pcChildNode->setDepth(pcCU->getDepth()+1);
            pcChildNode->setZorder( pcCU->getZorder() + (iTotalNumPart/4)*i );
            pcChildNode->setSize(pcCU->getSize()/2);
            int iSubCUX = pcCU->getX() + i%2 * (pcCU->getSize()/2);
            int iSubCUY = pcCU->getY() + i/2 * (pcCU->getSize()/2);
            pcChildNode->setX(iSubCUX);
            pcChildNode->setY(iSubCUY);
            pcCU->getSCUs().push_back(pcChildNode);
            xReadInCUMode(pcCUInfoStream, pcChildNode);
        }
    }
    else
    {
        /// leaf node : create PUs and write the PU Mode for it
        pcCU->setPartSize((PartSize)iCUMode);

        int iPUCount = ComCU::getPUNum((PartSize)iCUMode);
        for(int i = 0; i < iPUCount; i++)
        {
            ComPU* pcPU = new ComPU(pcCU);
            int iPUOffsetX, iPUOffsetY, iPUWidth, iPUHeight;
            ComCU::getPUOffsetAndSize(pcCU->getSize(), (PartSize)iCUMode, i, iPUOffsetX, iPUOffsetY, iPUWidth, iPUHeight);
            int iPUX = pcCU->getX() + iPUOffsetX;
            int iPUY = pcCU->getY() + iPUOffsetY;
            pcPU->setX(iPUX);
            pcPU->setY(iPUY);
            pcPU->setWidth(iPUWidth);
            pcPU->setHeight(iPUHeight);
            pcCU->getPUs().push_back(pcPU);
        }

    }
    return true;
}
