#ifndef HIGHLIGHTDIFFFILTER_H
#define HIGHLIGHTDIFFFILTER_H
#include "model/drawengine/abstractfilter.h"
#include <QObject>


class HighlightDiffFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    HighlightDiffFilter(QObject *parent = 0);


    virtual bool drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCTU, double dScale, QRect* pcScaledArea);





};

#endif // HIGHLIGHTFILTER_H
