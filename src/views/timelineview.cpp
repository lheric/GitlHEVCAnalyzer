#include "timelineview.h"
#include "model/common/comsequence.h"
#include "gitlivkcmdevt.h"
#include <QDebug>
#include <QWheelEvent>

TimeLineView::TimeLineView(QWidget *parent) :
    QGraphicsView(parent)
{
    /// init member
    m_pcCurDrawnSeq = NULL;
    m_iMaxBitForFrame = INT_MAX;

    setModualName("timeline");

    ///set listeners
    listenToParams("current_sequence", MAKE_CALLBACK(TimeLineView::onSequenceChanged));
    listenToParams("current_frame_poc", MAKE_CALLBACK(TimeLineView::onPOCChanged));

    ///
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(&m_cScene);

}

TimeLineView::~TimeLineView()
{
    /// takes back the ownership of the indicator, or there will be a double deletion by scene
    m_cScene.removeItem(&m_cCurFrameIndicator);
}

void TimeLineView::onSequenceChanged(GitlUpdateUIEvt &rcEvt)
{
    /// Get current sequence
    ComSequence* pcCurSequence = (ComSequence*)(rcEvt.getParameter("current_sequence").value<void*>());
    if(m_pcCurDrawnSeq!=pcCurSequence)  /// if sequence changed, clear and release all item on stage
    {
        m_pcCurDrawnSeq = pcCurSequence;
        xClearAllDrawing();
        xCalMaxBitForFrame(pcCurSequence);
    }
    /// Draw bars
    xDrawFrameBars(pcCurSequence);
}

void TimeLineView::onPOCChanged(GitlUpdateUIEvt &rcEvt)
{
    /// ensure the bars are drawn at first
    if(rcEvt.hasParameter("current_sequence"))
        onSequenceChanged(rcEvt);
    ///
    int iCurFramePOC = rcEvt.getParameter("current_frame_poc").toInt();
    Q_ASSERT(m_cFrameBars.size() > iCurFramePOC);
    TimeLineFrameItem* pcFrameBar = m_cFrameBars.at(iCurFramePOC);
    Q_ASSERT(pcFrameBar->getPOC() == iCurFramePOC);
    /// firstly remove it from scene
    if(m_cCurFrameIndicator.scene() != NULL)
        m_cScene.removeItem(&m_cCurFrameIndicator);

    /// recalculat position
    qreal dXOffset = pcFrameBar->boundingRect().width()-m_cCurFrameIndicator.boundingRect().width();
    dXOffset /= 2;
    QPointF cNewPos = pcFrameBar->pos() + QPointF(dXOffset,-75);    /// magic number
    m_cCurFrameIndicator.setPos(cNewPos);

    /// secondly add a new one
    m_cCurFrameIndicator.setZValue(1000);
    m_cScene.addItem(&m_cCurFrameIndicator);

    /// center the view
    this->centerOn(pcFrameBar);
}

void TimeLineView::frameBarClicked(int iPoc)
{
    GitlIvkCmdEvt cEvt("jumpto_frame");
    cEvt.setParameter("poc", iPoc);
    cEvt.dispatch();
}


void TimeLineView::wheelEvent(QWheelEvent * event)
{
    if( event->delta() < 0 ) /// rotated towards user
    {
        GitlIvkCmdEvt cEvt("next_frame");
        cEvt.dispatch();
    }
    else
    {
        GitlIvkCmdEvt cEvt("prev_frame");
        cEvt.dispatch();
    }
}

void TimeLineView::xDrawFrameBars(ComSequence* pcSequence)
{
    QRectF cRect(0,0,20,50);
    int iCounter = 0;
    int iGap = 2;

    foreach(ComFrame* pcFrame, pcSequence->getFrames())
    {
        int iPercent = pcFrame->getBitCount()*100/m_iMaxBitForFrame;
        iPercent = VALUE_CLIP(0, 100, iPercent);
        TimeLineFrameItem* pRectItem = new TimeLineFrameItem(iPercent, pcFrame->getPoc());
        connect(pRectItem, SIGNAL(barClick(int)), this, SLOT(frameBarClicked(int)));
        pRectItem->setPos(iCounter*(cRect.width()+iGap), 0);
        pRectItem->setPOC(pcFrame->getPoc());
        m_cFrameBars.push_back(pRectItem);
        m_cScene.addItem(pRectItem);
        iCounter++;
    }
}

void TimeLineView::xClearAllDrawing()
{
    foreach(TimeLineFrameItem* pcRectItem, m_cFrameBars)
    {
        pcRectItem->disconnect();
        m_cScene.removeItem(pcRectItem);
        delete pcRectItem;
    }
    m_cFrameBars.clear();
}

void TimeLineView::xCalMaxBitForFrame(ComSequence* pcSequence)
{
    // TODO not the real bit of frame (only the sum of LCUS)
    int iTotalBit = 0;
    foreach(ComFrame* pcFrame, pcSequence->getFrames())
            iTotalBit += pcFrame->getBitCount();
    int iTotalFrames = pcSequence->getFrames().size();
    m_iMaxBitForFrame = iTotalBit/iTotalFrames;
    if(m_iMaxBitForFrame == 0)  ///< avoid divide zero error
        ++m_iMaxBitForFrame;
    m_iMaxBitForFrame *= 5; ///< max bit is 5 times of avg bit

}
