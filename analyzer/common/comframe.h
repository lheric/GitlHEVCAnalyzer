#ifndef COMFRAME_H
#define COMFRAME_H


#include "comdef.h"
#include "comlcu.h"
class ComSequence;
class ComFrame
{
public:
    explicit ComFrame();
    ~ComFrame();
    void init();

    /*! CUs in one frame
      */
    ADD_CLASS_FIELD(QVector<ComLCU*>, cLCUs, getLCUs, setLCUs)

    /*! Frame info
      */
    ADD_CLASS_FIELD(int, iPoc, getPoc, setPoc)

    /*! Obsolescent
     */
    ADD_CLASS_FIELD(double, dPSNR, getPSNR, setPSNR)
    ADD_CLASS_FIELD(double, dBitrate, getBitrate, setBitrate)
    ADD_CLASS_FIELD(double, dTotalEncTime, getTotalEncTime, setTotalEncTime)
    ADD_CLASS_FIELD(double, dTotalDecTime, getTotalDecTime, setTotalDecTime)


};

#endif // COMFRAME_H
