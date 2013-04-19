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
}
