#ifndef COMMV_H
#define COMMV_H

#include "../common/comanalyzerdef.h"

class ComMV
{
public:
    explicit ComMV(): m_iHor(0), m_iVer(0){}

    ADD_CLASS_FIELD(int, iHor, getHor, setHor)
    ADD_CLASS_FIELD(int, iVer, getVer, setVer)

//    ADD_CLASS_FIELD(int, iFMESP, getFMESP, setFMESP)        ///< Number of search points (SP) in fractional-pixel motion estimation (FPME)
//    ADD_CLASS_FIELD(int, iSAD, getSAD, setSAD)              ///< Optimal SAD of best match point

};

#endif // COMMV_H
