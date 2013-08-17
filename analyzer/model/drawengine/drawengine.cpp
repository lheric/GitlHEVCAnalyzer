#include "drawengine.h"
#include "../io/analyzermsgsender.h"
#include <QPainter>
#include <iostream>
using namespace std;
DrawEngine::DrawEngine()
{
    m_dScale = 1.0;
}


QPixmap* DrawEngine::drawFrame( ComSequence* pcSequence, int iPoc, QPixmap *pcPixmap )
{

    ComFrame* pcFrame = pcSequence->getFrames().at(iPoc);
    int iLCUTotalNum = pcFrame->getLCUs().size();

    /// draw original pic
    m_cDrawnPixmap = pcPixmap->scaled(pcPixmap->size()*m_dScale, Qt::KeepAspectRatio, Qt::FastTransformation);
    QPainter cPainter(&m_cDrawnPixmap);

    /***********************************************************************
     *               Following is for drawing filters                      *
     ***********************************************************************/

    /// draw PU
    for( int iAddr = 0; iAddr < iLCUTotalNum; iAddr++ )
    {
        ComCU* pcLCU = pcFrame->getLCUs().at(iAddr);        
        xDrawPU( &cPainter, pcLCU );
    }

    /// draw CU
    QRect cScaledCUArea;
    for( int iAddr = 0; iAddr < iLCUTotalNum; iAddr++ )
    {
        ComCU* pcLCU = pcFrame->getLCUs().at(iAddr);
        xDrawCU( &cPainter, pcLCU );
    }

    /// draw CTU (i.e. LCU)
    for( int iAddr = 0; iAddr < iLCUTotalNum; iAddr++ )
    {
        ComCU* pcLCU = pcFrame->getLCUs().at(iAddr);
        int iPixelX = pcLCU->getX();
        int iPixelY = pcLCU->getY();
        cScaledCUArea.setCoords( iPixelX, iPixelY, (iPixelX+pcLCU->getSize())-1, (iPixelY+pcLCU->getSize())-1 );
        xScaleRect(&cScaledCUArea,&cScaledCUArea);
        m_cFilterLoader.drawCTU(&cPainter, pcLCU, m_dScale, &cScaledCUArea);
    }

    ///draw Frame
    QRect cScaledFrameArea(QPoint(0,0), m_cDrawnPixmap.size());
    m_cFilterLoader.drawFrame(&cPainter, pcFrame, m_dScale, &cScaledFrameArea);
    return &m_cDrawnPixmap;
}

bool DrawEngine::xDrawPU( QPainter* pcPainter,  ComCU* pcCU )
{

    if( pcCU->getSCUs().empty() )
    {
        /// traverse very PU in this Leaf-CU
        QRect cScaledPUArea;
        for( int iPUIdx = 0; iPUIdx < pcCU->getPUs().size(); iPUIdx++ )
        {
            ComPU* pcPU = pcCU->getPUs().at(iPUIdx);

            /// draw PU
            cScaledPUArea.setCoords( pcPU->getX(), pcPU->getY(), (pcPU->getX()+pcPU->getWidth())-1, (pcPU->getY()+pcPU->getHeight())-1 );
            xScaleRect(&cScaledPUArea, &cScaledPUArea);
            m_cFilterLoader.drawPU(pcPainter, pcPU, m_dScale, &cScaledPUArea);
        }

    }
    else
    {
        for(int iSub = 0; iSub < 4; iSub++)
        {
            xDrawPU ( pcPainter, pcCU->getSCUs().at(iSub) );
        }
    }
    return true;

}


bool DrawEngine::xDrawCU( QPainter* pcPainter,  ComCU* pcCU )
{

    if( pcCU->getSCUs().empty() )
    {

        /// draw CU
        QRect cScaledCUArea;
        cScaledCUArea.setCoords( pcCU->getX(), pcCU->getY(), (pcCU->getX()+pcCU->getSize())-1, (pcCU->getY()+pcCU->getSize())-1 );
        xScaleRect(&cScaledCUArea,&cScaledCUArea);
        m_cFilterLoader.drawCU(pcPainter, pcCU, m_dScale, &cScaledCUArea);

    }
    else
    {
        for(int iSub = 0; iSub < 4; iSub++)
        {
            xDrawCU ( pcPainter, pcCU->getSCUs().at(iSub) );
        }
    }
    return true;

}

void DrawEngine::xScaleRect( QRect* rcUnscaled, QRect* rcScaled )
{
    rcScaled->setTopLeft(rcUnscaled->topLeft()*m_dScale);
    rcScaled->setBottomRight((rcUnscaled->bottomRight()+QPoint(1,1))*m_dScale-QPoint(1,1));
}
