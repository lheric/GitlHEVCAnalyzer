#ifndef COMFRAME_H
#define COMFRAME_H

#include "comcu.h"
class ComSequence;
class ComFrame
{
public:
    explicit ComFrame(ComSequence *pcParent);
    ~ComFrame();

    bool operator < (const ComFrame& cOther) const
    {
        return (m_iPoc < cOther.m_iPoc);
    }

    /*! CUs in one frame
      */
    ADD_CLASS_FIELD(QVector<ComCU*>, cLCUs, getLCUs, setLCUs)

    /*! Frame info
      */
    ADD_CLASS_FIELD(ComSequence*, pcSequence, getSequence, setSequence)
    ADD_CLASS_FIELD(int, iPoc, getPoc, setPoc)

    /*! L0 & L1 & LC*/
    ADD_CLASS_FIELD_NOSETTER(QVector<int>, aiL0List, getL0List )
    ADD_CLASS_FIELD_NOSETTER(QVector<int>, aiL1List, getL1List )
    ADD_CLASS_FIELD_NOSETTER(QVector<int>, aiLCList, getLCList )


    /*! Decoding time comsumed*/
    ADD_CLASS_FIELD(double, dTotalDecTime, getTotalDecTime, setTotalDecTime)

    /*! Obsolescent
     */
    ADD_CLASS_FIELD(double, dPSNR, getPSNR, setPSNR)
    ADD_CLASS_FIELD(double, dBitrate, getBitrate, setBitrate)
    ADD_CLASS_FIELD(double, dTotalEncTime, getTotalEncTime, setTotalEncTime)



};

#endif // COMFRAME_H
