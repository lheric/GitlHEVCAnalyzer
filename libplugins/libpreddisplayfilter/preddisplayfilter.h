#ifndef PREDDISPLAYFILTER_H
#define PREDDISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include <QObject>

class PredDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit PredDisplayFilter(QObject *parent = 0);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU *pcPU, double dScale,  QRect* pcScaledArea);


    
public slots:
    
};

#endif // PREDDISPLAYFILTER_H
