#ifndef COMTILE_H
#define COMTILE_H
#include "gitldef.h"

class ComFrame;

class ComTile
{
public:
    ComTile(ComFrame* pcParent);

    ADD_CLASS_FIELD(ComFrame*, pcFrame, getFrame, setFrame)             ///< parent frame that holds this tile
    ADD_CLASS_FIELD(int, iFirstCUAddr, getFirstCUAddr, setFirstCUAddr)  ///< first CU in this tile
    ADD_CLASS_FIELD(int, iWidth, getWidth, setWidth)                    ///< tile width in CU num
    ADD_CLASS_FIELD(int, iHeight, getHeight, setHeight)                 ///< tile height in CU num
};

#endif // COMTILE_H
