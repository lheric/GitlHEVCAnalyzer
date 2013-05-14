#ifndef COMMV_H
#define COMMV_H

#include "gitldef.h"

class ComMV
{
public:
    explicit ComMV(): m_iHor(0), m_iVer(0){}

    ADD_CLASS_FIELD(int, iRefPoc, getRefPOC, setRefPOC)     ///< reference pic POC
    ADD_CLASS_FIELD(int, iHor, getHor, setHor)
    ADD_CLASS_FIELD(int, iVer, getVer, setVer)    


};

#endif // COMMV_H
