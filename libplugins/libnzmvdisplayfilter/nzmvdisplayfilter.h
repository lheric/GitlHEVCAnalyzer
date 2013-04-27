#ifndef NZMVDISPLAYFILTER_H
#define NZMVDISPLAYFILTER_H

#include <QObject>
#include "drawengine/abstractfilter.h"

class NZMVDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit NZMVDisplayFilter(QObject *parent = 0);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU* pcPU, double dScale,
                           QRect *pcScaledArea);
protected:
    bool xIsFractionalMV(ComMV* pcMV);

signals:
    
public slots:
    
};

#endif // NZMVDISPLAYFILTER_H
