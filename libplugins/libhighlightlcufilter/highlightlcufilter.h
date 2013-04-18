#ifndef HIGHLIGHTLCUFILTER_H
#define HIGHLIGHTLCUFILTER_H
#include "drawengine/abstractfilter.h"
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
    virtual bool drawCTU  (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           int iPoc, int iAddr,
                           int iCTUX, int iCTUY,
                           int iCTUSize, double dScale);

protected:
    QMultiMap<int, int> m_cTargetLCU;

};

#endif // HIGHLIGHTLCUFILTER_H
