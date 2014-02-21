#ifndef TUDISPLAYFILTER_H
#define TUDISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include "views/filterconfigdialog.h"
#include <QObject>
#include <QColor>


class TUDisplayFilterConfig
{
public:
    TUDisplayFilterConfig()
    {
        m_cTUColor  = QColor(0,0,0);
        m_dOpaque = 0.7;
        applyOpaque();
    }

    void applyOpaque()
    {
        m_cTUColor.setAlphaF(m_dOpaque);
    }
    ADD_CLASS_FIELD(QColor, cTUColor, getTUColor, setTUColor)
    ADD_CLASS_FIELD(double, dOpaque, getOpaque, setOpaque)
};


class TUDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit TUDisplayFilter(QObject *parent = 0);

    virtual bool drawTU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComTU *pcTU, double dScale,  QRect* pcScaledArea);
    virtual bool config   (FilterContext* pcContext);
    
public slots:

protected:
    ADD_CLASS_FIELD_PRIVATE(TUDisplayFilterConfig, cConfig)
    ADD_CLASS_FIELD_PRIVATE(FilterConfigDialog, cConfigDialog)
    
};

#endif // TUDISPLAYFILTER_H
