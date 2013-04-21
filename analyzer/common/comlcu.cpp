#include "comcu.h"

ComCU::ComCU(ComFrame *pcParent)
{
    m_pcFrame = pcParent;
    m_iAddr = -1;
    m_iZorder = -1;
    m_iDepth = -1;
    m_iX = -1;
    m_iY = -1;
    m_iSize = -1;
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

int ComCU::getPUNum( PartSize ePartSize )
{
    int iTotalNum = 0;
    switch ( ePartSize )
    {
    case SIZE_2Nx2N:
        iTotalNum = 1;
        break;
    case SIZE_NxN:
        iTotalNum = 4;
        break;
    case SIZE_2NxN:
    case SIZE_Nx2N:
    case SIZE_2NxnU:
    case SIZE_2NxnD:
    case SIZE_nLx2N:
    case SIZE_nRx2N:
        iTotalNum = 2;
        break;
    default:
        Q_ASSERT( ePartSize==SIZE_NONE ); ///< SIZE_NONE (out of boundary)
        iTotalNum = 0;
        break;
    }
    return iTotalNum;
}

void ComCU::getPUOffsetAndSize(  int        iLeafCUSize,
                                 PartSize   ePartSize,
                                 int        uiPUIdx,
                                 int        &riXOffset,
                                 int        &riYOffset,
                                 int        &riWidth,
                                 int        &riHeight )
{
    switch ( ePartSize )
    {
    case SIZE_2NxN:
        riWidth     = iLeafCUSize;
        riHeight    = iLeafCUSize >> 1;
        riXOffset   = 0;
        riYOffset   = ( uiPUIdx == 0 )? 0 : iLeafCUSize >> 1;
        break;
    case SIZE_Nx2N:
        riWidth     = iLeafCUSize >> 1;
        riHeight    = iLeafCUSize;
        riXOffset   = ( uiPUIdx == 0 )? 0 : iLeafCUSize >> 1;
        riYOffset   = 0;
        break;
    case SIZE_NxN:
        riWidth     = iLeafCUSize >> 1;
        riHeight    = iLeafCUSize >> 1;
        riXOffset   = ( (uiPUIdx&1) == 0 )? 0 : iLeafCUSize >> 1;
        riYOffset   = ( (uiPUIdx&2) == 0 )? 0 : iLeafCUSize >> 1;
        break;
    case SIZE_2NxnU:
        riWidth     = iLeafCUSize;
        riHeight    = ( uiPUIdx == 0 ) ?  iLeafCUSize >> 2 : ( iLeafCUSize >> 2 ) + ( iLeafCUSize >> 1 );
        riXOffset   = 0;
        riYOffset   = ( uiPUIdx == 0 ) ? 0 : iLeafCUSize >> 2;
        break;
    case SIZE_2NxnD:
        riWidth     = iLeafCUSize;
        riHeight    = ( uiPUIdx == 0 ) ?  ( iLeafCUSize >> 2 ) + ( iLeafCUSize >> 1 ) : iLeafCUSize >> 2;
        riXOffset   = 0;
        riYOffset   = ( uiPUIdx == 0 ) ? 0 : (iLeafCUSize >> 2) + (iLeafCUSize >> 1);
        break;
    case SIZE_nLx2N:
        riWidth     = ( uiPUIdx == 0 ) ? iLeafCUSize >> 2 : ( iLeafCUSize >> 2 ) + ( iLeafCUSize >> 1 );
        riHeight    = iLeafCUSize;
        riXOffset   = ( uiPUIdx == 0 ) ? 0 : iLeafCUSize >> 2;
        riYOffset   = 0;
        break;
    case SIZE_nRx2N:
        riWidth     = ( uiPUIdx == 0 ) ? ( iLeafCUSize >> 2 ) + ( iLeafCUSize >> 1 ) : iLeafCUSize >> 2;
        riHeight    = iLeafCUSize;
        riXOffset   = ( uiPUIdx == 0 ) ? 0 : (iLeafCUSize >> 2) + (iLeafCUSize >> 1);
        riYOffset   = 0;
        break;
    default:
        Q_ASSERT( ePartSize == SIZE_2Nx2N );
        riWidth     = iLeafCUSize;
        riHeight    = iLeafCUSize;
        riXOffset   = 0;
        riYOffset   = 0;
        break;
    }
}
