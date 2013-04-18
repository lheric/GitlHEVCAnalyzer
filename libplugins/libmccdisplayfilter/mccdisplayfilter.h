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


    virtual bool drawCTU  (QPainter* pcPainter,
                           FilterContext* pcContext,
                           ComSequence* pcSequence,
                           int iPoc, int iAddr,
                           int iCTUX, int iCTUY,
                           int iCTUSize, double dScale);


protected:
    int m_ccLCU[30][200];




};
#endif // MERGEDISPLAYFILTER_H
