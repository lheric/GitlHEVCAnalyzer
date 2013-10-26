#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <QGraphicsView>
#include "timelineframeitem.h"
#include "timelineindicatoritem.h"
#include "gitlview.h"
#include "gitldef.h"

class ComSequence;

class TimeLineView : public QGraphicsView, public GitlView
{
    Q_OBJECT
public:
    explicit TimeLineView(QWidget *parent = 0);

    virtual void onUIUpdate(GitlUpdateUIEvt& rcEvt);

public slots:
    void frameBarClicked(int iPoc);


protected:
    void wheelEvent(QWheelEvent * event);

private:
    void xDrawFrameBars(ComSequence* pcSequence);
    void xClearAllDrawing();
    void xCalMaxBitForFrame(ComSequence* pcSequence);

    ADD_CLASS_FIELD_PRIVATE(ComSequence*, pcCurDrawnSeq)
    ADD_CLASS_FIELD_PRIVATE(QVector<TimeLineFrameItem*>, cFrameBars)
    ADD_CLASS_FIELD_PRIVATE(TimelineIndicatorItem, cCurFrameIndicator)
    ADD_CLASS_FIELD_PRIVATE(QGraphicsScene, cScene)

    ADD_CLASS_FIELD_PRIVATE(int, iMaxBitForFrame)
signals:
    

    
};

#endif // TIMELINEVIEW_H
