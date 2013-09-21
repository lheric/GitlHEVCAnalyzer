#ifndef COMTU_H
#define COMTU_H
#include "gitldef.h"
#include <QVector>
class ComTU
{
public:
    ComTU();
    ADD_CLASS_FIELD(QVector<ComTU*>, apcTUs, getTUs, setTUs)
    ADD_CLASS_FIELD(int, iX, getX, setX)                                        ///< X Position in frame
    ADD_CLASS_FIELD(int, iY, getY, setY)                                        ///< Y Position in frame
    ADD_CLASS_FIELD(int, iSize, getSize, setSize)                               ///< CU Size

};

#endif // COMTU_H
