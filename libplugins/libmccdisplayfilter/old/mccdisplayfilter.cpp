#include "mccdisplayfilter.h"
#include <QRect>
#include <QColor>
#include <QBrush>

MCCDisplayFilter::MCCDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("MCC Display Filter");
    ///read file
}

bool MCCDisplayFilter::drawCU (QPainter* pcPainter,
                               FilterContext* pcContext,
                               ComSequence* pcSequence,
                               int iPoc, int iAddr,
                               int iZOrder, int iDepth,
                               int iCUX, int iCUY,
                               int iCUSize)
{

    QColor cCUColor = QColor(Qt::green);
    QRect cCUArea(iCUX, iCUY, iCUSize, iCUSize);


    cCUColor.setAlphaF(0.3);
    pcPainter->setPen(Qt::NoPen);
    pcPainter->setBrush(QBrush(cCUColor));
    pcPainter->drawRect(cCUArea);

    pcPainter->setPen(Qt::red);
    pcPainter->drawText(cCUArea, Qt::AlignCenter, QString("ssss"));



    return true;

}
