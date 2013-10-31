#include "comsequence.h"


ComSequence::ComSequence()
{
    init();
}

ComSequence::~ComSequence()
{
    for( int i = 0; i < m_cFrames.size(); i++ )
    {
        delete m_cFrames.at(i);
    }
}

void ComSequence::init()
{
    /*! Frames in this sequence */
    for( int i = 0; i < m_cFrames.size(); i++ )
    {
        delete m_cFrames.at(i);
    }
    m_cFrames.clear();

    /*! Sequence general info */
    m_strFileName.clear();
    m_iWidth  = -1;
    m_iHeight = -1;    
    m_iTotalFrames = -1;
    m_iMaxCUSize = -1;
    m_iMaxCUDepth = -1;
    m_iMinTUDepth = -1;
    m_iMaxTUDepth = -1;

    /*! YUV Info -- Currently Displaying YUV*/
    m_eYUVRole = YUV_NONE;


    /*!
     * Optional info
     * Obsolescent
     */
    m_strEncoderVersion.clear();
    m_dTotalDecTime = -1;
    m_dPSNR = -1;
    m_dBitrate = -1;
    m_dTotalEncTime = -1;
    m_dSameCUModePercent = -1;
    m_dMeanCUDepthError = -1;
}
