#include "mvdisplayfilter.h"

MVDisplayFilter::MVDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("MV Display");
    m_bShowRefPOC = false;

    /// MV pen
    m_cPenL0.setColor(QColor(Qt::blue));
    m_cPenL1.setColor(QColor(Qt::red));

    /// text pen
    m_cPenText.setColor(QColor(Qt::yellow));

    /// circle filling
    m_cCircleL0Fill.setStyle(Qt::SolidPattern);
    m_cCircleL0Fill.setColor(QColor(Qt::blue));
    m_cCircleL1Fill.setStyle(Qt::SolidPattern);
    m_cCircleL1Fill.setColor(QColor(Qt::red));
}

bool MVDisplayFilter::config  (FilterContext* pcContext)
{
    m_bShowRefPOC = !m_bShowRefPOC;
    return true;
}

bool MVDisplayFilter::drawPU  (FilterContext* pcContext, QPainter* pcPainter,
                               ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{
    int iInterDir = pcPU->getInterDir();
    QPoint cCenter = pcScaledArea->center();
    ComMV *pcMV = NULL;


    QFont cFont = pcPainter->font();
    cFont.setPointSize(10);
    pcPainter->setFont(cFont);

    if( iInterDir == 0 )
    {
        /// Do nothing
    }
    else if( iInterDir == 1 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(0);
        pcPainter->setPen(m_cPenL0);
        pcPainter->setBrush(m_cCircleL0Fill);
        pcPainter->drawEllipse((QPointF)cCenter, 1.5, 1.5);
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);

        if(m_bShowRefPOC)
            pcPainter->drawText(*pcScaledArea, Qt::AlignCenter, QString("L0 %1").arg(pcMV->getRefPOC()));

    }
    else if( iInterDir == 2 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(0);

        pcPainter->setPen(m_cPenL1);
        pcPainter->setBrush(m_cCircleL1Fill);
        pcPainter->drawEllipse((QPointF)cCenter, 1.5, 1.5);
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);

        if(m_bShowRefPOC)
            pcPainter->drawText(*pcScaledArea, Qt::AlignCenter, QString("L1 %1").arg(pcMV->getRefPOC()));

    }
    else if( iInterDir == 3 )  /// bi-directional prediction
    {
        /// Get MV of PU ( first direction )
        pcMV = pcPU->getMVs().at(0);
        pcPainter->setPen(m_cPenL0);
        pcPainter->setBrush(m_cCircleL0Fill);
        pcPainter->drawEllipse((QPointF)cCenter, 1.5, 1.5);
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);

        /// Get MV of PU ( second direction)
        pcMV = pcPU->getMVs().at(1);
        pcPainter->setPen(m_cPenL1);
        pcPainter->setBrush(m_cCircleL1Fill);
        pcPainter->drawEllipse((QPointF)cCenter, 1.5, 1.5);
        pcPainter->drawLine(cCenter, cCenter+QPoint(pcMV->getHor(),pcMV->getVer())*dScale/4);

        if(m_bShowRefPOC)
            pcPainter->drawText(*pcScaledArea, Qt::AlignCenter, QString("L0 %1 L1 %2").arg(pcPU->getMVs().at(0)->getRefPOC()).arg(pcPU->getMVs().at(1)->getRefPOC()));
    }
    return true;

}
