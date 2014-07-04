#ifndef FRAMEVIEW_H
#define FRAMEVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QDialog>
#include "gitldef.h"
#include "gitlview.h"

class FrameView : public QGraphicsView,  public GitlView
{
    Q_OBJECT
public:
    explicit FrameView(QWidget *parent = 0);
    void setDisplayImage(const QPixmap* pcFramePixmap);

protected:
    void xUpdateScale(GitlUpdateUIEvt& rcEvt);
    void xOnFrameArrived(GitlUpdateUIEvt& rcEvt);

protected:
    virtual void wheelEvent      ( QWheelEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseMoveEvent  ( QMouseEvent * event );
    virtual void keyPressEvent   ( QKeyEvent * event );



    /*!
     * current scale of the frame
     */
    ADD_CLASS_FIELD(double, dCurrScale, getCurrScale, setCurrScale)

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
