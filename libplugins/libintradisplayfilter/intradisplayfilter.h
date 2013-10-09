#ifndef INTRADISPLAYFILTER_H
#define INTRADISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include <QObject>

class IntraDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit IntraDisplayFilter(QObject *parent = 0);
    virtual bool config   (FilterContext* pcContext);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU *pcPU, double dScale,  QRect* pcScaledArea);


signals:
    
public slots:
    
};

#endif // INTRADISPLAYFILTER_H
