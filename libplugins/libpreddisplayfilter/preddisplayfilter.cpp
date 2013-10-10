#include "preddisplayfilter.h"
#include <QDebug>

PredDisplayFilter::PredDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("Pred Type Structure");
}


bool PredDisplayFilter::drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                                  ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{
    //different mode different color
    QColor cSkipClr(QColor(0,0,0,128));
    QColor cInterClr(QColor(51,0,204,128));
    QColor cIntraClr(QColor(204,255,51,128));


    PredMode eMode = pcPU->getPredMode();
    pcPainter->setPen(Qt::NoPen);
    if(eMode == MODE_INTER)
        pcPainter->setBrush(cInterClr);
    else if(eMode == MODE_INTRA)
        pcPainter->setBrush(cIntraClr);
    else if(eMode == MODE_SKIP)
        pcPainter->setBrush(cSkipClr);
    else
        qWarning() << "Unexpected Prediction Type";

    pcPainter->drawRect(*pcScaledArea);

    return true;
}
