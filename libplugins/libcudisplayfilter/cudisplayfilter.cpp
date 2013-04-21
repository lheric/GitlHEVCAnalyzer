#include "cudisplayfilter.h"

CUDisplayFilter::CUDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("CU Structure");
}

bool CUDisplayFilter::drawCU   (FilterContext* pcContext, QPainter* pcPainter,
                                ComCU *pcCU, double dScale,  QRect* pcScaledArea)
{
    /// Scaled CU Area
//    QPoint cLeftTop(iCUX*dScale+0.5, iCUY*dScale+0.5);
//    QPoint cBottomRight((iCUX+iCUSize)*dScale-0.5, (iCUY+iCUSize)*dScale-0.5);
//    QRect cCUArea(cLeftTop, cBottomRight);

    /// Draw CU Rect
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,255,255,128));
    pcPainter->drawRect(*pcScaledArea);
    return true;
}
