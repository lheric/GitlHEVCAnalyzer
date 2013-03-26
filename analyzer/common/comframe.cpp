#include "comframe.h"

ComFrame::ComFrame()
{
    init();
}

ComFrame::~ComFrame()
{
    for( int i = 0; i < m_cLCUs.size(); i++ )
    {
        delete m_cLCUs.at(i);
    }
}

void ComFrame::init()
{
    /*! CUs in one frame
      */
    for( int i = 0; i < m_cLCUs.size(); i++ )
    {
        delete m_cLCUs.at(i);
    }
    m_cLCUs.clear();
    /*! Frame info
      */
    m_iPoc = -1;
    m_dPSNR = -1;
    m_dBitrate = -1;
    m_dTotalEncTime = -1;
    m_dTotalDecTime = -1;
}
