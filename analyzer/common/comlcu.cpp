#include "comcu.h"

ComCU::ComCU(ComFrame *pcParent)
{
    m_pcFrame = pcParent;
    m_iAddr = -1;
    m_iZorder = -1;
    m_iDepth = -1;
}

ComCU::~ComCU()
{
    while( !m_apcSCUs.empty() )
    {
        delete m_apcSCUs.back();
        m_apcSCUs.pop_back();
    }

    while( !m_apcPUs.empty() )
    {
        delete m_apcPUs.back();
        m_apcPUs.pop_back();
    }
}
