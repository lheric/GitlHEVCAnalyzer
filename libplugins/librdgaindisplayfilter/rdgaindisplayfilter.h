#ifndef RDGAINDISPLAYFILTER_H
#define RDGAINDISPLAYFILTER_H

#include <QObject>
#include "drawengine/abstractfilter.h"

class RDGainDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit RDGainDisplayFilter(QObject *parent = 0);
    bool config   (FilterContext* pcContext);
    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU* pcPU, double dScale,
                           QRect *pcScaledArea);
signals:
    
public slots:
    
};

#endif // RDGAINDISPLAYFILTER_H
