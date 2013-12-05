#ifndef MCCDISPLAYFILTER_H
#define MCCDISPLAYFILTER_H

#include "model/drawengine/abstractfilter.h"
#include <QObject>
#include <QMap>
#include <numeric>

enum MCCType
{
    PREDICTED,
    REAL
};

struct LCUAddr
{
    int iPoc;
    int iAddr;
    LCUAddr()
    {
        iPoc = 0;
        iAddr = 0;
    }

    bool operator < (const LCUAddr& other) const
    {
        if(iPoc < other.iPoc)
            return true;
        if(iPoc == other.iPoc)
            return iAddr < other.iAddr;
        return false;
    }
};

class LCUMCC
{
public:
    LCUMCC()
    {
        memset(*aaiMCC, 0, sizeof(int)*8*8);
    }

    int getMCC() const
    {
        return std::accumulate(*aaiMCC, *aaiMCC+8*8, 0);
    }

    LCUMCC& operator = (const LCUMCC& other)
    {
        memcpy(*aaiMCC, *other.aaiMCC, sizeof(int)*8*8);
        return *this;
    }

    void reset() { memset(*aaiMCC, 0, sizeof(int)*8*8); }

    int aaiMCC[8][8];    /// max depth 3 mcc support
};

class MCCDisplayFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    MCCDisplayFilter(QObject *parent = 0);

    virtual bool init     (FilterContext* pcContext);

    virtual bool uninit   (FilterContext* pcContext);

    virtual bool drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCTU, double dScale, QRect* pcScaledArea);

    virtual bool drawFrame(FilterContext* pcContext, QPainter* pcPainter,
                           ComFrame *pcFrame, double dScale, QRect* pcScaledArea);

    virtual bool config   (FilterContext* pcContext);

protected:
    void xCalMCCFromRef(ComFrame *pcRefFrame, QVector<LCUMCC> *pcMCCTable);
    void xCalMCCFromRefHelper(ComCU* pcCU, QVector<LCUMCC>* pcMCCTable);

    void xCalRealMCC(ComFrame *pcFrame, QVector<LCUMCC> *pcMCCTable);
    void xCalRealMCCHelper(ComCU* pcCU, QVector<LCUMCC>* pcMCCTable);

    double xGetMVMCCWeight(ComMV* pcMV);
    double xGetAreaMCCWeight(ComPU* pcPU);

protected:
    QVector<LCUMCC> m_cMCCReal;

    QVector<LCUMCC> m_cMCCFromL0;
    QVector<LCUMCC> m_cMCCFromL1;

    int m_iSeqWidth;
    int m_iSeqHeight;
    int m_iSeqMaxCUSize;
    int m_iLCUInRow;
    int m_iLCUInCol;

    MCCType m_eMCCType;


};
#endif // MERGEDISPLAYFILTER_H
