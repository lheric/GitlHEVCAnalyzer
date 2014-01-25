#ifndef TUDISPLAYFILTER_H
#define TUDISPLAYFILTER_H
#include "model/drawengine/abstractfilter.h"
#include <QObject>

class TUDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.sysu.gitl.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit TUDisplayFilter(QObject *parent = 0);

    virtual bool drawTU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComTU *pcTU, double dScale,  QRect* pcScaledArea);

    
public slots:
    
};

#endif // TUDISPLAYFILTER_H
