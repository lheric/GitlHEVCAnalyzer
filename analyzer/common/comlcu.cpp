#include "comcu.h"

ComCU::ComCU()
{
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
