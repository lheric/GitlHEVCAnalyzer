#include "tudisplayfilter.h"

TUDisplayFilter::TUDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("TU Structure");
    m_cConfigDialog.setWindowTitle("TU Structure Filter");
    m_cConfigDialog.addColorPicker("TU Mode Color", &m_cConfig.getTUColor());
    m_cConfigDialog.addSlider("Opaque", 0.0, 1.0, &m_cConfig.getOpaque());
}

bool TUDisplayFilter::config(FilterContext *pcContext)
{
    m_cConfigDialog.exec();
    m_cConfig.applyOpaque();
    return true;
}


bool TUDisplayFilter::drawTU   (FilterContext* pcContext, QPainter* pcPainter,
                                ComTU *pcTU, double dScale,  QRect* pcScaledArea)
{

    /// Draw TU Rect
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(m_cConfig.getTUColor());
    pcPainter->drawRect(*pcScaledArea);

    return true;
}
