#ifndef MCCDISPLAYFILTER_H
#define MCCDISPLAYFILTER_H

#include "model/drawengine/abstractfilter.h"
#include <QObject>
#include <QMap>

struct LCUAddr
{
    int iPoc;
    int iAddr;
    bool operator < (const LCUAddr& other) const
    {
        if(iPoc < other.iPoc)
            return true;
        if(iPoc == other.iPoc)
            return iAddr < other.iAddr;
        return false;
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


    virtual bool drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCTU, double dScale, QRect* pcScaledArea);


protected:
    QMultiMap<LCUAddr, int> m_cLCUMCC;



};
#endif // MERGEDISPLAYFILTER_H
