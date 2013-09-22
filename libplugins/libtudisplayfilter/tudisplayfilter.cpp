#include "tudisplayfilter.h"

TUDisplayFilter::TUDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("TU Structure");
}


bool TUDisplayFilter::drawTU   (FilterContext* pcContext, QPainter* pcPainter,
                                ComTU *pcTU, double dScale,  QRect* pcScaledArea)
{

    /// Draw TU Rect
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,0,0,128));
    pcPainter->drawRect(*pcScaledArea);

    return true;
}
