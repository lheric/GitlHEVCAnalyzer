#ifndef BITDISPLAYFILTER_H
#define BITDISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include <QObject>
#include <QPen>
class BitDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit BitDisplayFilter(QObject *parent = 0);

    virtual bool init     (FilterContext* pcContext);

    virtual bool drawCTU  (FilterContext *pcContext, QPainter *pcPainter,
                           ComCU *pcCTU, double dScale, QRect *pcScaledArea);

signals:

    ADD_CLASS_FIELD_PRIVATE(double, dLCUAvgBit)
    
public slots:
    
};

#endif // BITDISPLAYFILTER_H
