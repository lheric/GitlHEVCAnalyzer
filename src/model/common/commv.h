#ifndef COMMV_H
#define COMMV_H

#include "gitldef.h"
#include <qmath.h>
class ComMV
{
public:
    explicit ComMV(): m_iHor(0), m_iVer(0){}
    explicit ComMV(int iHor, int iVer): m_iHor(iHor), m_iVer(iVer){}

    int getLengthSquare() const
    {
        return (m_iHor*m_iHor + m_iVer*m_iVer);
    }

    double getLength() const
    {
        return qSqrt(m_iHor*m_iHor + m_iVer*m_iVer);
    }

    double getAngle() const
    {
        return qAtan2(m_iVer, m_iHor);
    }

    ComMV operator - (const ComMV& other) const
    {
        return ComMV(m_iHor-other.m_iHor, m_iVer-other.m_iVer);
    }

    bool isZero() const
    {
        return (m_iHor==0 && m_iVer==0);
    }

    ADD_CLASS_FIELD(int, iRefPoc, getRefPOC, setRefPOC)     ///< reference pic POC
    ADD_CLASS_FIELD(int, iHor, getHor, setHor)
    ADD_CLASS_FIELD(int, iVer, getVer, setVer)    


};

#endif // COMMV_H
