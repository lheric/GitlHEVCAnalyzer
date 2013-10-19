#ifndef TIMELINEFRAMEITEM_H
#define TIMELINEFRAMEITEM_H

#include <QGraphicsItemGroup>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QObject>
#include "gitldef.h"

class TimeLineFrameItem : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    explicit TimeLineFrameItem(int iPercent = 0, int iPOC = -1);
    void setHeightPercent(int iPercent);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
signals:
    void barClick(int iPOC);
    

    ADD_CLASS_FIELD(int, iPOC, getPOC, setPOC)
    ADD_CLASS_FIELD_NOSETTER(int, iHeightPercent, getHeightPercent)
    ADD_CLASS_FIELD(int, iMaxHeight, getMaxHeight, setMaxHeight)
    ADD_CLASS_FIELD(int, iMaxWidth, getMaxWidth, setMaxWidth)

    ADD_CLASS_FIELD_PRIVATE(QGraphicsRectItem, cFrameBar)
    ADD_CLASS_FIELD_PRIVATE(QGraphicsRectItem, cHitArea)
    ADD_CLASS_FIELD_PRIVATE(QGraphicsSimpleTextItem, cPocText)
};

#endif // TIMELINEFRAMEITEM_H
