#include "comtile.h"

ComTile::ComTile(ComFrame* pcParent)
{
    m_pcFrame = pcParent;
    m_iFirstCUAddr = -1;
    m_iWidth = -1;
    m_iHeight = -1;
}
