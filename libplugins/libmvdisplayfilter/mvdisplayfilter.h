#ifndef MVDISPLAYFILTER_H
#define MVDISPLAYFILTER_H

#include <QObject>
#include "drawengine/abstractfilter.h"

class MVDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit MVDisplayFilter(QObject *parent = 0);

    virtual bool drawPU   (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           ComPU* pcPU,
                           int iPoc, int iAddr,
                           int iZOrder, int iDepth,
                           PartSize ePartSize, int iPUIndex,
                           int iPUX, int iPUY,
                           int iPUWidth, int iPUHeight, double dScale);


signals:
    
public slots:
    
};

#endif // MVDISPLAYFILTER_H
