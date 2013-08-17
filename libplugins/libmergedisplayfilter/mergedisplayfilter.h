#ifndef MERGEDISPLAYFILTER_H
#define MERGEDISPLAYFILTER_H

#include "model/drawengine/abstractfilter.h"
#include <QObject>

class MergeDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    MergeDisplayFilter(QObject *parent = 0);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU* pcPU, double dScale,
                           QRect *pcScaledArea);


};
#endif // MERGEDISPLAYFILTER_H
