#include "mvdisplayfilter.h"

MVDisplayFilter::MVDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("MV Display");
}

bool MVDisplayFilter::drawPU  (FilterContext* pcContext, QPainter* pcPainter,
                               ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{
    int iInterDir = pcPU->getInterDir();
    QPoint cCenter = pcScaledArea->center();
    ComMV *pcMV = NULL;
    if( iInterDir == 0 )
    {
        /// Do nothing
    }
    else if( iInterDir == 1 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(0);

        pcPainter->setPen(QColor(Qt::blue));
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);

    }
    else if( iInterDir == 2 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(0);

        pcPainter->setPen(QColor(Qt::red));
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);

    }
    else if( iInterDir == 3 )  /// bi-directional prediction
    {
        /// Get MV of PU ( first direction )
        pcMV = pcPU->getMVs().at(0);
        pcPainter->setPen(QColor(Qt::blue));
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);

        /// Get MV of PU ( second direction)
        pcMV = pcPU->getMVs().at(1);
        pcPainter->setPen(QColor(Qt::red));
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);
    }
    return true;

}
