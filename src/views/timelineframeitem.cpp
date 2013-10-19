#include "timelineframeitem.h"
#include <QDebug>
#include <QPen>
#include <QFont>
#include <QBrush>
#include <QColor>
TimeLineFrameItem::TimeLineFrameItem(int iPercent, int iPOC)
{
    m_iMaxWidth = 20;
    m_iMaxHeight = 50;
    m_iPOC = iPOC;

    /// layout
    m_cFrameBar.setRect(0,0,m_iMaxWidth,m_iMaxHeight);

    m_cHitArea.setRect(0,-m_iMaxHeight*1.5,m_iMaxWidth,m_iMaxHeight*3);
    m_cHitArea.setPen(Qt::NoPen);

    m_cPocText.setPos(0,0);

    QFont cFont;
    cFont.setPointSize(12);
    cFont.setBold(true);
    m_cPocText.setFont(cFont);
    m_cPocText.setBrush(QBrush(QColor(Qt::gray)));
    m_cPocText.setText(QString("#%1").arg(m_iPOC));
    m_cPocText.setPos(19,10);
    m_cPocText.setRotation(90);


    /// change frame bar height
    setHeightPercent(iPercent);

    /// group
    this->addToGroup(&m_cFrameBar);
    this->addToGroup(&m_cHitArea);
    this->addToGroup(&m_cPocText);

    ///
    setAcceptHoverEvents(true);
}

void TimeLineFrameItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if(m_iPOC < 0)
    {
        qWarning() << QString("Frame Bar With Invalid POC Num %1").arg(m_iPOC);
        return;
    }
    emit barClick(m_iPOC);
}

void TimeLineFrameItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    m_cHitArea.setBrush(QBrush(QColor(0,255,0,50)));
    m_cHitArea.update(m_cHitArea.rect());
}

void TimeLineFrameItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    m_cHitArea.setBrush(QBrush(QColor(0,255,0,0)));
    m_cHitArea.update(m_cHitArea.rect());
}


void TimeLineFrameItem::setHeightPercent(int iPercent)
{
    if(iPercent < 0 || iPercent > 100)
    {
        qWarning() << QString("Invalid Timeline Frame Bar Height Percentage %1").arg(iPercent);
        return;
    }
    m_iHeightPercent = iPercent;
    QRectF cRect= m_cFrameBar.rect();
    cRect.setHeight(-m_iMaxHeight*iPercent/100);
    m_cFrameBar.setRect(cRect);

    /// set fill color according to height
    m_cFrameBar.setPen(QPen(QColor(255,255,255,128)));
    QColor cFillColor;

    int iClip = VALUE_CLIP(0, 240, iPercent*240/100);
    double dHue = (240-iClip)/360.0;
    cFillColor.setHsvF(dHue, 1.0, 1.0, 0.6);
    m_cFrameBar.setBrush(QBrush(cFillColor));


}


