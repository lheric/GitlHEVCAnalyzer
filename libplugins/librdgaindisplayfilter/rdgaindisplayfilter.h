#ifndef RDGAINDISPLAYFILTER_H
#define RDGAINDISPLAYFILTER_H

#include <QObject>
#include <QMultiMap>
#include "drawengine/abstractfilter.h"
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


class RDGainDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit RDGainDisplayFilter(QObject *parent = 0);
    virtual bool init     (FilterContext *pcContext);
    virtual bool config   (FilterContext* pcContext);
    virtual bool drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCTU, double dScale, QRect* pcScaledArea);

signals:

protected:
    QMultiMap<LCUAddr, long> m_cOriCost;
    QMultiMap<LCUAddr, long> m_cDisMDCost;
    QMultiMap<LCUAddr, long> m_cDisIMECost;
    QMultiMap<LCUAddr, long> m_cDisFMECost;
    
public slots:
    
};

#endif // RDGAINDISPLAYFILTER_H
