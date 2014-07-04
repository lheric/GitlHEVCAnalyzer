#include "tiledisplayfilter.h"

#include "model/modellocator.h"
#include <QPen>

TileDisplayFilter::TileDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("Tile Disply Filter");
    m_cConfigDialog.addColorPicker("Color", &m_cConfig.getPenColor());
    m_cConfigDialog.addSlider("Line Width", 1.0, 10.0, &m_cConfig.getPenWidth());
}



bool TileDisplayFilter::drawTile(FilterContext *pcContext, QPainter *pcPainter, ComTile *pcTile, double dScale, QRect *pcScaledArea)
{

    QPen iPen;
    iPen.setStyle(Qt::SolidLine);
    iPen.setWidth(m_cConfig.getPenWidth());
    iPen.setBrush(m_cConfig.getPenColor());
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(iPen);
    pcPainter->drawRect(*pcScaledArea);

    return true;


}

bool TileDisplayFilter::config(FilterContext *pcContext)
{
    m_cConfigDialog.exec();
    return true;
}
