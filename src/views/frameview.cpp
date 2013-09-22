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

        //AnalyzerMsgSender::getInstance()->msgOut(QString("%1").arg(dNextScale));
        GitlIvkCmdEvt cEvt("zoom_frame");
        cEvt.setParameter("scale", dNextScale);
        cEvt.dispatch();


        int iImgX = m_cGraphicsPixmapItem.scenePos().x();
        int iImgY = m_cGraphicsPixmapItem.scenePos().y();

        int iMouseX = mapToScene(event->pos()).x();
        int iMouseY = mapToScene(event->pos()).y();

        //AnalyzerMsgSender::getInstance()->msgOut(QString("%1 %2 %3 %4").arg(iImgX).arg(iImgY).arg(iMouseX).arg(iMouseY));

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
}

void FrameView::mouseMoveEvent ( QMouseEvent * event )
{
    int iTransX = mapToScene(event->pos()).x() - m_iMousePressX;
    int iTransY = mapToScene(event->pos()).y() - m_iMousePressY;
    m_cGraphicsPixmapItem.setPos(m_iMousePressImageX+iTransX,
                                 m_iMousePressImageY+iTransY);
}



