#include "comframe.h"

ComFrame::ComFrame(ComSequence* pcParent)
{
    m_pcSequence = pcParent;
    m_iPoc = -1;
    m_dPSNR = -1;
    m_dBitrate = -1;
    m_dTotalEncTime = -1;
    m_dTotalDecTime = -1;
}

ComFrame::~ComFrame()
{
    for( int i = 0; i < m_cLCUs.size(); i++ )
    {
        delete m_cLCUs.at(i);
    }
}


