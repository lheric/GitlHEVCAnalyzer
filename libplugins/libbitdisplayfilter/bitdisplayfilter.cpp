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
        return false;

    foreach( ComFrame* pcFrame, pcSeq->getFrames())
        foreach( ComCU* pcCU, pcFrame->getLCUs() )
            m_iLCUAvgBit += pcCU->getBitCount();

    return true;

}

bool BitDisplayFilter::drawCTU  (FilterContext *pcContext, QPainter *pcPainter,
                                ComCU *pcCTU, double dScale, QRect *pcScaledArea)
{
    int iClip = VALUE_CLIP(0,240,pcCTU->getBitCount());

    QColor cFill;
    double dHue = (240-iClip)/360.0;
    cFill.setHsvF(dHue, 1.0, 1.0, 0.6);
    pcPainter->setBrush(QBrush(cFill));
    pcPainter->setPen(Qt::NoPen);
    pcPainter->drawRect(*pcScaledArea);
    return true;
}
