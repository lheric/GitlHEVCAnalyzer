#include "mccdisplayfilter.h"
#include <QRect>
#include <QColor>
#include <QBrush>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QString>
#include <QDebug>
#include <numeric>
#include "model/modellocator.h"

MCCDisplayFilter::MCCDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("MCC Display Filter");
    m_eMCCType = PREDICTED;
}

bool MCCDisplayFilter::init(FilterContext *pcContext)
{

    ComSequence* pcSequence = pcContext->pcSequenceManager->getCurrentSequence();
    if(pcSequence == NULL)
        return true;

    m_iSeqWidth  = pcSequence->getWidth();
    m_iSeqHeight = pcSequence->getHeight();
    m_iSeqMaxCUSize = pcSequence->getMaxCUSize();

    m_iLCUInRow = (m_iSeqWidth+m_iSeqMaxCUSize-1)/m_iSeqMaxCUSize;
    m_iLCUInCol = (m_iSeqHeight+m_iSeqMaxCUSize-1)/m_iSeqMaxCUSize;

    m_cMCCFromL0.clear();
    m_cMCCFromL0.fill(LCUMCC(), m_iLCUInCol*m_iLCUInRow);
    m_cMCCFromL1.clear();
    m_cMCCFromL1.fill(LCUMCC(), m_iLCUInCol*m_iLCUInRow);

    m_cMCCReal.clear();
    m_cMCCReal.fill(LCUMCC(), m_iLCUInCol*m_iLCUInRow);

    return true;
}

bool MCCDisplayFilter::uninit(FilterContext* pcContext)
{
    return true;
}

bool MCCDisplayFilter::drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                                 ComCU *pcCTU, double dScale, QRect* pcScaledArea)
{
    /// calcuate MCC for this frame when entering
    if( pcCTU->getAddr() == 0 )
    {

        QVector<int> aiL0 = pcCTU->getFrame()->getL0List();
        if(aiL0.empty())        /// TODO may not be suitable for bi-direction prediction
        {
            qDebug() << QString("No reference frame for POC %1, no mcc.").arg(pcCTU->getFrame()->getPoc());
        }
        else
        {
            ComSequence* pcCurSequence = pcContext->pcSequenceManager->getCurrentSequence();
            /// L0 direction MCC
            int iRefPOCL0 = aiL0.at(0);
            ComFrame* pcRefFrame = pcCurSequence->getFrames().at(iRefPOCL0);
            xCalMCCFromRef(pcRefFrame, &m_cMCCFromL0);
            /// TODO L1 direction MCC

            /// Calculate Real MCC
            xCalRealMCC(pcCTU->getFrame(), &m_cMCCReal);
        }
    }


    /// display mcc for this LCU
    int iAddr = pcCTU->getAddr();
    int iPredMCC = m_cMCCFromL0.at(iAddr).getMCC();    ///L0 direction MCC
    int iRealMCC = m_cMCCReal.at(iAddr).getMCC();

    /// draw text
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,0,0));
    QFont cFont = pcPainter->font();
    pcPainter->setPen(QColor(255,0,0));
    cFont.setPointSize(18);
    pcPainter->setFont(cFont);
    pcPainter->drawText(*pcScaledArea,Qt::AlignCenter, QString("%1/%2\n%3/%4").arg(iPredMCC).arg(iRealMCC).arg(iPredMCC-iRealMCC).arg(QString::number( qAbs(iRealMCC-iPredMCC)/double(iRealMCC), 'g', 2 )));

    /// draw rect
    int iAlpha = 255-iPredMCC*15;
    iAlpha=((iAlpha<0)?(0):(iAlpha));
    iAlpha=((iAlpha>175)?(175):(iAlpha));
    if( (iPredMCC > 30) && (iRealMCC > 100) && (qAbs(iRealMCC-iPredMCC)/double(iRealMCC) > 1.8) )
        pcPainter->setPen(QPen(Qt::red,3));
    else
        pcPainter->setPen(Qt::NoPen);
    //pcPainter->setBrush(QColor(0,0,0,iAlpha));
    pcPainter->drawRect(*pcScaledArea);
    return true;
}


bool MCCDisplayFilter::drawFrame(FilterContext* pcContext, QPainter* pcPainter,
                                ComFrame *pcFrame, double dScale, QRect* pcScaledArea)
{
    int iLCUNum = m_cMCCFromL0.size();
    m_cMCCFromL0.clear();
    m_cMCCFromL0.fill(LCUMCC(), iLCUNum);
    m_cMCCFromL1.clear();
    m_cMCCFromL1.fill(LCUMCC(), iLCUNum);
    m_cMCCReal.clear();
    m_cMCCReal.fill(LCUMCC(), iLCUNum);

    return true;
}

bool MCCDisplayFilter::config   (FilterContext* pcContext)
{
    m_eMCCType = (m_eMCCType==PREDICTED)?REAL:PREDICTED;
    qDebug() << "MCC Type is " << ((m_eMCCType==PREDICTED)?"PREDICTED":"REAL");
    return true;
}


void MCCDisplayFilter::xCalMCCFromRef(ComFrame* pcRefFrame, QVector<LCUMCC> *pcMCCTable)
{

    for(int i = 0; i < pcRefFrame->getLCUs().size(); i++)
    {
        ComCU* pcLCU = pcRefFrame->getLCUs().at(i);
        xCalMCCFromRefHelper(pcLCU, pcMCCTable);
    }
}

void MCCDisplayFilter::xCalMCCFromRefHelper(ComCU *pcCU, QVector<LCUMCC> *pcMCCTable)
{
    foreach(ComCU* pcSubCU, pcCU->getSCUs())
    {
        if( pcSubCU->getSCUs().empty() )
        {
            /// leaf
            foreach( ComPU* pcPU, pcSubCU->getPUs() )
            {
                if(pcPU->getPredMode() == MODE_INTRA)
                    continue;
//                if(pcPU->getMergeIndex() != -1 )
//                    continue;   ///ignore merge

                 QPoint cCenter(pcPU->getX()+pcPU->getWidth()/2,
                               pcPU->getY()+pcPU->getHeight()/2);

                foreach(ComMV* pcMV, pcPU->getMVs())
                {
                    int iPredX = cCenter.x()-pcMV->getHor()/4;
                    int iPredY = cCenter.y()-pcMV->getVer()/4;

                    /// TODO Fix out of boundary
                    iPredX = VALUE_CLIP(0, m_iSeqWidth-1,  iPredX);
                    iPredY = VALUE_CLIP(0, m_iSeqHeight-1, iPredY);

                    int iLCUPosX = iPredX/m_iSeqMaxCUSize;
                    int iLCUPosY = iPredY/m_iSeqMaxCUSize;

                    int iSmallestCUSize = m_iSeqMaxCUSize/8;
                    int iLCURaster = iLCUPosY*m_iLCUInRow+iLCUPosX;


                    LCUMCC cMCC= pcMCCTable->at(iLCURaster);
                    int iXinLCU = iLCUPosX%m_iSeqMaxCUSize;
                    int iYinLCU = iLCUPosY%m_iSeqMaxCUSize;
                    cMCC.aaiMCC[iXinLCU/iSmallestCUSize][iYinLCU/iSmallestCUSize] += xGetMVMCCWeight(pcMV)*xGetAreaMCCWeight(pcPU);
                    (*pcMCCTable)[iLCURaster] = cMCC;
                }
            }
        }
        else
        {
            xCalMCCFromRefHelper(pcSubCU, pcMCCTable);
        }

    }
}


void MCCDisplayFilter::xCalRealMCC(ComFrame *pcFrame, QVector<LCUMCC> *pcMCCTable)
{
    for(int i = 0; i < pcFrame->getLCUs().size(); i++)
    {
        ComCU* pcLCU = pcFrame->getLCUs().at(i);
        xCalRealMCCHelper(pcLCU, pcMCCTable);
    }
}

void MCCDisplayFilter::xCalRealMCCHelper(ComCU* pcCU, QVector<LCUMCC>* pcMCCTable)
{
    foreach(ComCU* pcSubCU, pcCU->getSCUs())
    {
        if( pcSubCU->getSCUs().empty() )
        {
            /// leaf
            foreach( ComPU* pcPU, pcSubCU->getPUs() )
            {
                if(pcPU->getPredMode() == MODE_INTRA)
                    continue;
//                if(pcPU->getMergeIndex() != -1 )
//                    continue;   ///ignore merge

                 QPoint cCenter(pcPU->getX()+pcPU->getWidth()/2,
                               pcPU->getY()+pcPU->getHeight()/2);

                foreach(ComMV* pcMV, pcPU->getMVs())
                {
                    int iPredX = cCenter.x();
                    int iPredY = cCenter.y();

                    /// TODO Fix out of boundary
                    iPredX = VALUE_CLIP(0, m_iSeqWidth-1,  iPredX);
                    iPredY = VALUE_CLIP(0, m_iSeqHeight-1, iPredY);

                    int iLCUPosX = iPredX/m_iSeqMaxCUSize;
                    int iLCUPosY = iPredY/m_iSeqMaxCUSize;

                    int iSmallestCUSize = m_iSeqMaxCUSize/8;
                    int iLCURaster = iLCUPosY*m_iLCUInRow+iLCUPosX;


                    LCUMCC cMCC= pcMCCTable->at(iLCURaster);
                    int iXinLCU = iLCUPosX%m_iSeqMaxCUSize;
                    int iYinLCU = iLCUPosY%m_iSeqMaxCUSize;
                    cMCC.aaiMCC[iXinLCU/iSmallestCUSize][iYinLCU/iSmallestCUSize] += xGetMVMCCWeight(pcMV)*xGetAreaMCCWeight(pcPU);
                    (*pcMCCTable)[iLCURaster] = cMCC;
                }
            }
        }
        else
        {
            xCalRealMCCHelper(pcSubCU, pcMCCTable);
        }

    }
}

double MCCDisplayFilter::xGetMVMCCWeight(ComMV *pcMV)
{
    double dWeight = qSqrt(qPow(pcMV->getHor()/4.0,2)+qPow(pcMV->getVer()/4.0,2));
    if(dWeight > 16.0)
        dWeight = 0.0;
    return dWeight;
}

double MCCDisplayFilter::xGetAreaMCCWeight(ComPU *pcPU)
{
    double dWeight = (pcPU->getWidth()*pcPU->getHeight())/16;
    return dWeight;
}
