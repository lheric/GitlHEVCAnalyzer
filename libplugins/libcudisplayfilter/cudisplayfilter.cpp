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
                                        int iCUSize)
{
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,255,255,128));
    pcPainter->drawRect(iCUX, iCUY, iCUSize, iCUSize);
    return true;

}
