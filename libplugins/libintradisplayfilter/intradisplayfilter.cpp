#include "intradisplayfilter.h"
#include <QDebug>
#include <QMap>
#include <qmath.h>

/// Intra direction rotation degree
static qreal s_aiIntraRotation[] =
{
    0,   0,                                                                 ///< [0,  1] Planar, DC (Invalid)
    225.00, 219.09, 213.27, 207.98, 202.11, 195.71, 188.88, 183.57,         ///< [2,  9]
    180.00, 176.43, 171.12, 164.29, 157.89, 152.02, 146.73, 140.91, 135,    ///< [10, 18]
    129.09, 123.27, 117.98, 112.11, 105.71, 98.88,  93.57,  90.00,          ///< [19, 26]
    86.43,  81.12,  74.29,  67.89,  62.02,  56.73,  50.91,  45.00           ///< [27, 34]
};

//3.57,8.88,15.71,22.11,27.98,33.27,39.09,45


IntraDisplayFilter::IntraDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("Intra Mode Display");
}

bool IntraDisplayFilter::config   (FilterContext* pcContext)
{
    return true;
}


bool IntraDisplayFilter::drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                                                ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{

    /// Draw PU Rect
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,255,255,128));
    pcPainter->drawRect(*pcScaledArea);

    /// Draw intra mode
    if(pcPU->getPredMode() == MODE_INTRA)
    {

        int iIntraDir = pcPU->getIntraDirLuma();

        pcPainter->setClipRect(*pcScaledArea, Qt::ReplaceClip);
        pcPainter->setClipping(true);

        if(iIntraDir == 0)      /// PLANAR
        {
            pcPainter->drawEllipse(pcScaledArea->center(), pcScaledArea->width()/2, pcScaledArea->width()/2);
        }
        else if(iIntraDir == 1) /// DC
        {
            pcPainter->drawEllipse(pcScaledArea->topLeft(), pcScaledArea->width()/2, pcScaledArea->width()/2);
        }
        else if(iIntraDir >= 2 && iIntraDir <= 34)  /// Angular
        {
            double dRotation = -s_aiIntraRotation[iIntraDir];
            int iLength = pcScaledArea->width();
            const QLine cLine(QPoint(0,0), QPoint(iLength, 0));

            pcPainter->translate(pcScaledArea->center());
            pcPainter->rotate(dRotation);
            pcPainter->drawLine(cLine);
            pcPainter->rotate(180);
            pcPainter->drawLine(cLine);
            pcPainter->resetMatrix();
        }
        else
        {
            qCritical() << QString("Unexpected Intra Angular: %1").arg(iIntraDir);
        }

        pcPainter->setClipping(false);

    }

    return true;
}
