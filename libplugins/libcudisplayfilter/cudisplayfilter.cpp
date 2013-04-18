#include "cudisplayfilter.h"

CUDisplayFilter::CUDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("CU Structure");
}

bool CUDisplayFilter::drawCU   (QPainter* pcPainter,
                                FilterContext* pcContext,
                                ComSequence* pcSequence,
                                int iPoc, int iAddr,
                                int iZOrder, int iDepth,
                                int iCUX, int iCUY,
                                int iCUSize, double dScale)
{
    /// Scaled CU Area
    QPoint cLeftTop(iCUX*dScale+0.5, iCUY*dScale+0.5);
    QPoint cBottomRight((iCUX+iCUSize)*dScale-0.5, (iCUY+iCUSize)*dScale-0.5);
    QRect cCUArea(cLeftTop, cBottomRight);

    /// Draw CU Rect
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,255,255,128));
    pcPainter->drawRect(cCUArea);
    return true;
}
