#include "frameview.h"
#include <QWheelEvent>
#include <QMimeData>
#include "gitlivkcmdevt.h"
#include <QDebug>
FrameView::FrameView(QWidget *parent) :
    QGraphicsView(parent)
{
    m_dCurrScale = 1.0;
    m_cGraphicsPixmapItem.setTransformationMode(Qt::FastTransformation);
    m_cGraphicsScene.addItem(&m_cGraphicsPixmapItem);
    setScene(&m_cGraphicsScene);
    setAcceptDrops(false); //avoid blocking drop event to QMainWindow

    listenToParams("picture", MAKE_CALLBACK(FrameView::xOnFrameArrived));   /// display frame
    listenToParams("scale", MAKE_CALLBACK(FrameView::xUpdateScale));        /// record latest scale

}



void FrameView::setDisplayImage(const QPixmap* pcFramePixmap)
{
    if(pcFramePixmap == NULL)
        return;
    m_cGraphicsPixmapItem.setPixmap(*pcFramePixmap);
}

void FrameView::xUpdateScale(GitlUpdateUIEvt &rcEvt)
{
    m_dCurrScale = rcEvt.getParameter("scale").toDouble();
}

void FrameView::xOnFrameArrived(GitlUpdateUIEvt& rcEvt)
{
    QVariant vValue = rcEvt.getParameter("picture");
    QPixmap* pcPixMap = (QPixmap*)(vValue.value<void *>());
    setDisplayImage(pcPixMap);
}

void FrameView::wheelEvent ( QWheelEvent * event )
{
    //AnalyzerMsgSender::getInstance()->msgOut(QString("%1").arg(event->delta()));
    int iDelta = event->delta();
    double dIncrement = 0;
    if( iDelta < 0 )
    {
        dIncrement = -0.2;
    }
    else
    {
        dIncrement = 0.2;
    }

    double dNextScale = m_dCurrScale + dIncrement;
    if( dNextScale < 0.1 )
    {
        /// scale limit
    }
    else
    {

        // zooming
        GitlIvkCmdEvt cEvt("zoom_frame");
        cEvt.setParameter("scale", dNextScale);
        cEvt.dispatch();

        // center the mouse pos
        int iImgX = m_cGraphicsPixmapItem.scenePos().x();
        int iImgY = m_cGraphicsPixmapItem.scenePos().y();

        int iMouseX = mapToScene(event->pos()).x();
        int iMouseY = mapToScene(event->pos()).y();

        m_cGraphicsPixmapItem.moveBy((iImgX-iMouseX)*dIncrement/m_dCurrScale,
                                     (iImgY-iMouseY)*dIncrement/m_dCurrScale);

        m_dCurrScale = dNextScale;


    }
}

void FrameView::mousePressEvent ( QMouseEvent * event )
{
    m_iMousePressX = mapToScene(event->pos()).x();
    m_iMousePressY = mapToScene(event->pos()).y();
    m_iMousePressImageX = m_cGraphicsPixmapItem.x();
    m_iMousePressImageY = m_cGraphicsPixmapItem.y();

    int iMouseX = mapToScene(event->pos()).x();
    int iMouseY = mapToScene(event->pos()).y();

    /// only respond to mouse action inside a frame (shink for 1px)
    QRectF cFrameRect = m_cGraphicsPixmapItem.boundingRect();
    cFrameRect.translate(1,1);
    cFrameRect.setWidth(cFrameRect.width()-2);
    cFrameRect.setHeight(cFrameRect.height()-2);
    cFrameRect = m_cGraphicsPixmapItem.mapRectToScene(cFrameRect);
    if(cFrameRect.contains(iMouseX,iMouseY))
    {
        QPointF cScaledPoint = m_cGraphicsPixmapItem.mapFromScene(iMouseX, iMouseY);
        GitlIvkCmdEvt cFilterMousePressCMD("mousepress_filter");
        cFilterMousePressCMD.setParameter("scaled_point", cScaledPoint);
        cFilterMousePressCMD.setParameter("mouse_button",int(event->button()));
        cFilterMousePressCMD.dispatch();
    }

}

void FrameView::mouseMoveEvent ( QMouseEvent * event )
{
    int iTransX = mapToScene(event->pos()).x() - m_iMousePressX;
    int iTransY = mapToScene(event->pos()).y() - m_iMousePressY;
    m_cGraphicsPixmapItem.setPos(m_iMousePressImageX+iTransX,
                                 m_iMousePressImageY+iTransY);
}

void FrameView::keyPressEvent(QKeyEvent *event)
{
    GitlIvkCmdEvt cFilterKeyPressCMD("keypress_filter");
    cFilterKeyPressCMD.setParameter("pressed_key", event->key());
    cFilterKeyPressCMD.dispatch();
    QGraphicsView::keyPressEvent(event);    /// do not block key event
}



