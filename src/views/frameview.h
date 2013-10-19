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
    void setDisplayImage(const QPixmap* pcFramePixmap);



    virtual void wheelEvent ( QWheelEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent ( QMouseEvent * event );





    /*!
     * current scale of the frame
     */
    ADD_CLASS_FIELD_NOSETTER(double, dCurrScale, getCurrScale)

    /*!
     * Stage and Item to display the frame
     */
    ADD_CLASS_FIELD_NOSETTER(QGraphicsScene, cGraphicsScene, getGraphicsScene)
    ADD_CLASS_FIELD_PRIVATE(QGraphicsPixmapItem, cGraphicsPixmapItem)


    ADD_CLASS_FIELD_PRIVATE(int, iMousePressX)
    ADD_CLASS_FIELD_PRIVATE(int, iMousePressY)
    ADD_CLASS_FIELD_PRIVATE(int, iMousePressImageX)
    ADD_CLASS_FIELD_PRIVATE(int, iMousePressImageY)
signals:

public slots:

};

#endif // FRAMEVIEW_H
