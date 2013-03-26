#ifndef MCCDISPLAYFILTER_H
#define MCCDISPLAYFILTER_H

#include "drawengine/abstractfilter.h"
#include <QObject>

struct MccVal
{
    int Addr;
    int Poc;

    bool operator < (const MccVal& other) const
    {
        return (Poc < other.Poc) || (Addr < other.Addr);
    }
};

class MCCDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    MCCDisplayFilter(QObject *parent = 0);

    virtual bool init     (FilterContext* pcContext);
    virtual bool uninit   (FilterContext* pcContext) {return true;}
    virtual bool config   (FilterContext* pcContext) {return true;}
    virtual bool drawFrame(QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence, int iPoc)
    {
        return true;
    }

    virtual bool drawCTU  (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           int iPoc, int iAddr,
                           int iCTUX, int iCTUY,
                           int iCTUSize);


    virtual bool drawCU   (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           int iPoc, int iAddr,
                           int iZOrder, int iDepth,
                           int iCUX, int iCUY,
                           int iCUSize)
    {
        return true;
    }



    virtual bool drawPU   (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           ComPU* pcPU,
                           int iPoc, int iAddr,
                           int iZOrder, int iDepth,
                           PartSize ePartSize, int iPUIndex,
                           int iPUX, int iPUY,
                           int iPUWidth, int iPUHeight)
    {
        return true;
    }

protected:
    int m_ccLCU[30][200];




};
#endif // MERGEDISPLAYFILTER_H
