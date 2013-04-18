#ifndef HIGHLIGHTDIFFFILTER_H
#define HIGHLIGHTDIFFFILTER_H
#include "drawengine/abstractfilter.h"
#include <QObject>


class HighlightDiffFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    HighlightDiffFilter(QObject *parent = 0);


    virtual bool drawCTU  (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           int iPoc, int iAddr,
                           int iCTUX, int iCTUY,
                           int iCTUSize, double dScale);





};

#endif // HIGHLIGHTFILTER_H
