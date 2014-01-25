#include "bitdisplayfilter.h"
#include "model/modellocator.h"
#include <QDebug>
BitDisplayFilter::BitDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("Bit Heatmap Display");
}

bool BitDisplayFilter::init(FilterContext* pcContext)
{
    m_iLCUAvgBit = 0;
    ComSequence* pcSeq = pcContext->pcSequenceManager->getCurrentSequence();
    if(pcSeq == NULL)
        return true;

    foreach( ComFrame* pcFrame, pcSeq->getFrames())
        foreach( ComCU* pcCU, pcFrame->getLCUs() )
            m_iLCUAvgBit += pcCU->getBitCount();
    m_iLCUAvgBit /= pcSeq->getFrames().size()*(pcSeq->getFrames().at(0)->getLCUs().size());

    return true;

}

bool BitDisplayFilter::drawCTU  (FilterContext *pcContext, QPainter *pcPainter,
                                ComCU *pcCTU, double dScale, QRect *pcScaledArea)
{
    ///TODO fix m_iLCUAvgBit should be used here
    int iClip = VALUE_CLIP(240,359,pcCTU->getBitCount()/(m_iLCUAvgBit*5.0)*(359-240)+240);

    QColor cFill;
    double dHue = iClip/360.0;
    cFill.setHsvF(dHue, 1.0, 1.0, 0.6);
    pcPainter->setBrush(QBrush(cFill));
    pcPainter->setPen(Qt::NoPen);
    pcPainter->drawRect(*pcScaledArea);
    return true;
}
