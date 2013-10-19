#ifndef TIMELINEINDICATORITEM_H
#define TIMELINEINDICATORITEM_H

#include <QGraphicsItemGroup>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include "gitldef.h"

class TimelineIndicatorItem : public QGraphicsItemGroup
{
public:
    TimelineIndicatorItem();



    ADD_CLASS_FIELD_PRIVATE(QGraphicsRectItem, cRectTop)
    ADD_CLASS_FIELD_PRIVATE(QGraphicsRectItem, cRectBar)

};

#endif // TIMELINEINDICATORITEM_H
