#ifndef HIGHLIGHTLCUFILTER_H
#define HIGHLIGHTLCUFILTER_H
#include "model/drawengine/abstractfilter.h"
#include <QObject>
#include <QMap>

class HighlightLCUFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    HighlightLCUFilter(QObject *parent = 0);

    virtual bool init     (FilterContext* pcContext);
    virtual bool drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCTU, double dScale, QRect* pcScaledArea);

protected:
    QMultiMap<int, int> m_cTargetLCU;

};

#endif // HIGHLIGHTLCUFILTER_H
