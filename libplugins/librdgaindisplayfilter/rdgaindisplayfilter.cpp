#include "rdgaindisplayfilter.h"

RDGainDisplayFilter::RDGainDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("R-D Gain Display");
}

bool RDGainDisplayFilter::config   (FilterContext* pcContext)
{

    return true;
}

bool RDGainDisplayFilter::drawPU  (FilterContext* pcContext, QPainter* pcPainter,
                               ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{

    return true;

}

