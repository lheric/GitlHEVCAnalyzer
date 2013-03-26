#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include "gitldef.h"

class FrameView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit FrameView(QWidget *parent = 0);
    virtual void wheelEvent ( QWheelEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );

    ADD_CLASS_FIELD_NOSETTER(QGraphicsScene, cGraphicsScene, getGraphicsScene)
    ADD_CLASS_FIELD_NOSETTER(QGraphicsPixmapItem, cGraphicsPixmapItem, getGraphicsPixmapItem)
    ADD_CLASS_FIELD_PRIVATE(int, iMousePressX)
    ADD_CLASS_FIELD_PRIVATE(int, iMousePressY)
    ADD_CLASS_FIELD_PRIVATE(int, iMousePressImageX)
    ADD_CLASS_FIELD_PRIVATE(int, iMousePressImageY)
signals:

public slots:

};

#endif // FRAMEVIEW_H
