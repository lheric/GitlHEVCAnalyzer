#ifndef CUDISPLAYFILTER_H
#define CUDISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include "views/filterconfigdialog.h"
#include "gitldef.h"
#include <QObject>
#include <QPen>

class CUDisplayFilterConfig
{
public:
    CUDisplayFilterConfig()
    {
        m_bShowPU = true;
        m_bShowLCUOnly = false;
        m_cLCUColor = QColor(255,255,255);
        m_cSCUColor = QColor(255,255,255);
        m_cPUColor  = QColor(255,255,255);
        m_dOpaque   = 0.5;
        applyOpaque();
    }

    void applyOpaque()
    {
        m_cLCUColor.setAlphaF(m_dOpaque);
        m_cSCUColor.setAlphaF(m_dOpaque);
        m_cPUColor.setAlphaF(m_dOpaque);
    }

    ADD_CLASS_FIELD(QColor, cLCUColor, getLCUColor, setLCUColor)
    ADD_CLASS_FIELD(QColor, cSCUColor, getSCUColor, setSCUColor)
    ADD_CLASS_FIELD(QColor, cPUColor,  getPUColor,  setPUColor)
    ADD_CLASS_FIELD(bool,   bShowPU,   getShowPU,   setShowPU)
    ADD_CLASS_FIELD(bool,   bShowLCUOnly, getShowLCUOnly, setShowLCUOnly)
    ADD_CLASS_FIELD(double, dOpaque,   getOpaque,   setOpaque)

};




class CUDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit CUDisplayFilter(QObject *parent = 0);
    virtual bool config   (FilterContext* pcContext);

    virtual bool drawCTU  (FilterContext *pcContext, QPainter *pcPainter,
                           ComCU *pcCTU, double dScale, QRect *pcScaledArea);

    virtual bool drawCU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCU, double dScale,  QRect* pcScaledArea);

    virtual bool mousePress(FilterContext *pcContext, QPainter *pcPainter, ComFrame *pcFrame,
                            const QPointF *pcUnscaledPos, const QPointF *scaledPos, double dScale, Qt::MouseButton eMouseBtn);

    virtual bool keyPress  (FilterContext *pcContext, QPainter *pcPainter, ComFrame *pcFrame, int iKeyPressed);
signals:

    ADD_CLASS_FIELD_PRIVATE(QPen, cLCUPen)
    ADD_CLASS_FIELD_PRIVATE(QPen, cCUPen)
    ADD_CLASS_FIELD_PRIVATE(QPen, cPUPen)

    ADD_CLASS_FIELD_PRIVATE(FilterConfigDialog, cConfigDialog)
    ADD_CLASS_FIELD_PRIVATE(CUDisplayFilterConfig, cConfig)
    ADD_CLASS_FIELD_PRIVATE(ComCU*, pcSelectedCU)
    
public slots:
    


};

#endif // CUDISPLAYFILTER_H
