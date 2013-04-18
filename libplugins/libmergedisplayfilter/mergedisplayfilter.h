#ifndef MERGEDISPLAYFILTER_H
#define MERGEDISPLAYFILTER_H

#include "drawengine/abstractfilter.h"
#include <QObject>

class MergeDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    MergeDisplayFilter(QObject *parent = 0);

    virtual bool drawPU   (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           ComPU* pcPU,
                           int iPoc, int iAddr,
                           int iZOrder, int iDepth,
                           PartSize ePartSize, int iPUIndex,
                           int iPUX, int iPUY,
                           int iPUWidth, int iPUHeight, double dScale);


};
#endif // MERGEDISPLAYFILTER_H
