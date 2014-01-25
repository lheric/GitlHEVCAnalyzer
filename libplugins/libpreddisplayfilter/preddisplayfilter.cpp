#include "preddisplayfilter.h"
#include <QDebug>

PredDisplayFilter::PredDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("Pred Type Display");
    m_cConfigDialog.setWindowTitle("Predition Type Filter");
    m_cConfigDialog.addColorPicker("Skip Mode Color (Abandoned after HM-8.0)", &m_cConfig.getSkipColor());
    m_cConfigDialog.addColorPicker("Inter Mode Color", &m_cConfig.getInterColor());
    m_cConfigDialog.addColorPicker("Intra Mode Color", &m_cConfig.getIntraColor());
    m_cConfigDialog.addSlider("Opaque", 0.0, 1.0, &m_cConfig.getOpaque());
}

bool PredDisplayFilter::config(FilterContext *pcContext)
{
    m_cConfigDialog.exec();
    m_cConfig.applyOpaque();
    return true;
}


bool PredDisplayFilter::drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                                  ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{

    PredMode eMode = pcPU->getPredMode();
    pcPainter->setPen(Qt::NoPen);
    if(eMode == MODE_INTER)
        pcPainter->setBrush(m_cConfig.getInterColor());
    else if(eMode == MODE_INTRA)
        pcPainter->setBrush(m_cConfig.getIntraColor());
    else if(eMode == MODE_SKIP)
        pcPainter->setBrush(m_cConfig.getSkipColor());
    else
        qWarning() << "Unexpected Prediction Type";

    pcPainter->drawRect(*pcScaledArea);

    return true;
}
