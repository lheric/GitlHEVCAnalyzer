#include "timelineindicatoritem.h"
#include <QColor>
#include <QPen>
#include <QBrush>
TimelineIndicatorItem::TimelineIndicatorItem()
{

    m_cRectTop.setRect(0,0,10,16);
    m_cRectTop.setPen(QColor(255,255,255,128));
    m_cRectTop.setBrush(QColor(255,0,0,255));
    addToGroup(&m_cRectTop);


    QRectF cUpperRect = m_cRectTop.rect();
    QPointF cBL = (cUpperRect.bottomLeft()+cUpperRect.bottomRight())/2;

    m_cRectBar.setRect(cBL.x()-1,cBL.y(),3,65);
    m_cRectBar.setPen(Qt::NoPen);
    m_cRectBar.setBrush(QColor(255,0,0,255));
    addToGroup(&m_cRectBar);
}
