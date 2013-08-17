#ifndef MVDISPLAYFILTER_H
#define MVDISPLAYFILTER_H

#include <QObject>
#include "model/drawengine/abstractfilter.h"

class MVDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit MVDisplayFilter(QObject *parent = 0);

    virtual bool config(FilterContext *pcContext);

    virtual bool drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComPU* pcPU, double dScale,
                           QRect *pcScaledArea);


signals:
    
public slots:


    ADD_CLASS_FIELD_PRIVATE(bool, bShowRefPOC)  ///< show reference POC or not
    
};

#endif // MVDISPLAYFILTER_H
