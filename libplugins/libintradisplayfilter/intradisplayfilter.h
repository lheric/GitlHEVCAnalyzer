#ifndef INTRADISPLAYFILTER_H
#define INTRADISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include "views/filterconfigdialog.h"
#include "gitldef.h"
#include <QObject>
#include <QColor>


class IntraDisplayFilterConfig
{
public:
    IntraDisplayFilterConfig()
    {
        m_cColor = QColor(255,255,255);
        m_dOpaque = 0.7;
        applyOpaque();
    }

    void applyOpaque()
    {
        m_cColor.setAlphaF(m_dOpaque);
    }

    ADD_CLASS_FIELD(QColor, cColor, getColor, setColor)
    ADD_CLASS_FIELD(double, dOpaque, getOpaque, setOpaque)
};



class IntraDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit IntraDisplayFilter(QObject *parent = 0);
    virtual bool config   (FilterContext* pcContext);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU *pcPU, double dScale,  QRect* pcScaledArea);


    ADD_CLASS_FIELD_PRIVATE(FilterConfigDialog, cConfigDialog)
    ADD_CLASS_FIELD_PRIVATE(IntraDisplayFilterConfig, cConfig)

signals:
    
public slots:
    
};

#endif // INTRADISPLAYFILTER_H
