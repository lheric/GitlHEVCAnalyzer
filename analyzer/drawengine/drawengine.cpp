#include "drawengine.h"
#include "../common/comanalyzerdef.h"
#include "../io/analyzermsgsender.h"
#include <QPainter>
#include <iostream>
using namespace std;
DrawEngine::DrawEngine()
{
    m_dFrameScale = 1.0;
}


QPixmap* DrawEngine::drawFrame( ComSequence* pcSequence, int iPoc, QPixmap *pcPixmap )
{

    ComFrame* pcFrame = pcSequence->getFrames().at(iPoc);
    int iSeqWidth = pcSequence->getWidth();
    int iSeqHeight = pcSequence->getHeight();
    int iLCUTotalNum = pcFrame->getLCUs().size();
    int iMaxCUSize = pcSequence->getMaxCUSize();
    int iCUOneRow = (iSeqWidth+iMaxCUSize-1)/iMaxCUSize;

    m_cDrawnPixmap = pcPixmap->scaled(pcPixmap->size()*m_dFrameScale, Qt::KeepAspectRatio, Qt::FastTransformation);
    QPainter cPainter(&m_cDrawnPixmap);



    /// for every CU in this frame
    for( int iAddr = 0; iAddr < iLCUTotalNum; iAddr++ )
    {
        ComCU* pcLCU = pcFrame->getLCUs().at(iAddr);
        int iPixelX = (pcLCU->getAddr()%iCUOneRow)*iMaxCUSize;
        int iPixelY = (pcLCU->getAddr()/iCUOneRow)*iMaxCUSize;

        /// skip the LCU acrossing the frame boundary
        if( (iPixelX + iMaxCUSize > iSeqWidth) ||
                (iPixelY + iMaxCUSize > iSeqHeight) )
        {
            continue;
        }



        /// draw CU
//        QVector<int> aiMode = pcLCU->getCUPUMode();
//        QVector<int> aiPred = pcLCU->getPredType();
//        QVector<int> aiInterDir = pcLCU->getInterDir();
//        QVector<ComMV*> apcMVs = pcLCU->getMVs();
//        QVector<int> aiMergeIndex = pcLCU->getMergeIndex();
//        QVector<int> aiIntraDir = pcLCU->getIntraDir();



        xDrawCU(pcSequence,
                pcLCU,
                &cPainter,
                iPoc,
                iAddr,
                0,
                0,
                iPixelX,
                iPixelY,
                iMaxCUSize
                );



        /// draw CTU
        m_cFilterLoader.drawCTU(&cPainter, pcSequence, iPoc, iAddr, iPixelX, iPixelY, iMaxCUSize, m_dFrameScale);

    }///LCU loop end


    ///Frame Filter
    m_cFilterLoader.drawFrame(&cPainter, pcSequence, iPoc, m_dFrameScale);
    return &m_cDrawnPixmap;
}




bool DrawEngine::xDrawCU     ( ComSequence*   pcSequence,
                              ComCU*          pcCU,
                              QPainter*       pcPainter,
                              int             iPoc,
                              int             iAddr,
                              int             iZOrder,
                              int             iDepth,
                              int             iCUX,
                              int             iCUY,
                              int             iCUSize
                              )

{

    int iTotalNumPart = 1 << ( (pcSequence->getMaxCUDepth()-iDepth) << 1 );

    //

    //draw CU
    if( pcCU->getSCUs().empty() )
    {

        //cout << iDepth << " " << iCUSize << endl;
        /// draw CU
        m_cFilterLoader.drawCU(pcPainter, pcSequence, iPoc, iAddr,
                               iZOrder, iDepth, iCUX, iCUY, iCUSize, m_dFrameScale);

        /// draw PU
        /// traverse very PU in this Leaf-CU
        int iMode = pcCU->getPartSize();

        for( int iPUIdx = 0; iPUIdx < pcCU->getPUs().size(); iPUIdx++ )
        {
            ComPU* pcPU = pcCU->getPUs().at(iPUIdx);

            /// Calculate PU position and size
            int iPUOffsetX, iPUOffsetY, iPUWidth, iPUHeight;
            xGetPUOffsetAndSize(iCUSize, (PartSize)iMode, iPUIdx, iPUOffsetX, iPUOffsetY, iPUWidth, iPUHeight);
            int iPUX = iCUX + iPUOffsetX;
            int iPUY = iCUY + iPUOffsetY;            

            m_cFilterLoader.drawPU(pcPainter, pcSequence, pcPU, iPoc, iAddr, iZOrder, iDepth,
                                   (PartSize)iMode, iPUIdx, iPUX, iPUY, iPUWidth, iPUHeight, m_dFrameScale );

        }

    }
    else
    {
        for(int iSub = 0; iSub < 4; iSub++)
        {

            int iSubCUX = iCUX + iSub%2 * (iCUSize/2);
            int iSubCUY = iCUY + iSub/2 * (iCUSize/2);
            xDrawCU (  pcSequence,
                       pcCU->getSCUs().at(iSub),
                       pcPainter,
                       iPoc,
                       iAddr,
                       iZOrder+iSub*iTotalNumPart/4,
                       iDepth+1,
                       iSubCUX,
                       iSubCUY,
                       iCUSize/2
                    );
        }
    }
    return true;

}





void DrawEngine::xGetPUOffsetAndSize(int        iLeafCUSize,
                                     PartSize   ePartSize,
                                     int        uiPUIdx,
                                     int&       riXOffset,
                                     int&       riYOffset,
                                     int&       riWidth,
                                     int&       riHeight )
{
  switch ( ePartSize )
  {
    case SIZE_2NxN:
      riWidth = iLeafCUSize;
      riHeight = iLeafCUSize >> 1;
      riXOffset = 0;
      riYOffset = ( uiPUIdx == 0 )? 0 : iLeafCUSize >> 1;
      break;
    case SIZE_Nx2N:
      riWidth = iLeafCUSize >> 1;
      riHeight = iLeafCUSize;
      riXOffset = ( uiPUIdx == 0 )? 0 : iLeafCUSize >> 1;
      riYOffset = 0;
      break;
    case SIZE_NxN:
      riWidth = iLeafCUSize >> 1;
      riHeight = iLeafCUSize >> 1;
      riXOffset = ( (uiPUIdx&1) == 0 )? 0 : iLeafCUSize >> 1;
      riYOffset = ( (uiPUIdx&2) == 0 )? 0 : iLeafCUSize >> 1;
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
      riWidth = iLeafCUSize;
      riHeight = iLeafCUSize;
      riXOffset   = 0;
      riYOffset   = 0;
      break;
  }
}

