#include "comlcu.h"

ComLCU::ComLCU()
{
}

ComLCU::~ComLCU()
{
    for( int i = 0; i < m_cMVs.size(); i++ )
    {
        delete m_cMVs.at(i);
    }
//    for( int i = 0; i < m_cModeTry.size(); i++ )
//    {
//        delete m_cModeTry.at(i);
//    }
}
