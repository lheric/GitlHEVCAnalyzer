#include "cudisplayfilter.h"

CUDisplayFilter::CUDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("CU Structure");
    m_bShowPU = true;
}

bool CUDisplayFilter::config   (FilterContext* pcContext)
{
    m_bShowPU = !m_bShowPU;
    return true;
}


bool CUDisplayFilter::drawCU   (FilterContext* pcContext, QPainter* pcPainter,
                                ComCU *pcCU, double dScale,  QRect* pcScaledArea)
{
    /// Scaled CU Area
//    QPoint cLeftTop(iCUX*dScale+0.5, iCUY*dScale+0.5);
//    QPoint cBottomRight((iCUX+iCUSize)*dScale-0.5, (iCUY+iCUSize)*dScale-0.5);
//    QRect cCUArea(cLeftTop, cBottomRight);

    /// Draw CU Rect
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,255,255,128));
    pcPainter->drawRect(*pcScaledArea);

    /// Draw PU
    if(m_bShowPU)
    {
        QPen cPen;
        cPen.setStyle(Qt::DashLine);
        cPen.setWidth(1);
        cPen.setBrush(QBrush(QColor(255,255,255,128)));
        pcPainter->setPen(cPen);

        PartSize ePartSize = pcCU->getPartSize();

        //Top-left corner of this sub-CU
        int iXInFrame = pcScaledArea->left();
        int iYInFrame = pcScaledArea->top();
        //width of this sub-CUU
        int iWidth = pcScaledArea->width();
        switch(ePartSize)
        {
        case SIZE_2Nx2N:           ///< symmetric motion partition,  2Nx2N
            break;
        case SIZE_2NxN:            ///< symmetric motion partition,  2Nx N
            pcPainter->drawLine(iXInFrame, iYInFrame+iWidth/2, iXInFrame+iWidth, iYInFrame+iWidth/2);
            break;
        case SIZE_Nx2N:            ///< symmetric motion partition,   Nx2N
            pcPainter->drawLine(iXInFrame+iWidth/2, iYInFrame, iXInFrame+iWidth/2, iYInFrame+iWidth);
            break;
        case SIZE_NxN:             ///< symmetric motion partition,   Nx N
            pcPainter->drawLine(iXInFrame, iYInFrame+iWidth/2, iXInFrame+iWidth, iYInFrame+iWidth/2);
            pcPainter->drawLine(iXInFrame+iWidth/2, iYInFrame, iXInFrame+iWidth/2, iYInFrame+iWidth);
            break;
        case SIZE_2NxnU:           ///< asymmetric motion partition, 2Nx( N/2) + 2Nx(3N/2)
            pcPainter->drawLine(iXInFrame, iYInFrame+iWidth/4, iXInFrame+iWidth, iYInFrame+iWidth/4);
            break;
        case SIZE_2NxnD:           ///< asymmetric motion partition, 2Nx(3N/2) + 2Nx( N/2)
            pcPainter->drawLine(iXInFrame, iYInFrame+iWidth*3/4, iXInFrame+iWidth, iYInFrame+iWidth*3/4);
            break;
        case SIZE_nLx2N:           ///< asymmetric motion partition, ( N/2)x2N + (3N/2)x2N
            pcPainter->drawLine(iXInFrame+iWidth/4, iYInFrame, iXInFrame+iWidth/4, iYInFrame+iWidth);
            break;
        case SIZE_nRx2N:           ///< asymmetric motion partition, (3N/2)x2N + ( N/2)x2N
            pcPainter->drawLine(iXInFrame+iWidth*3/4, iYInFrame, iXInFrame+iWidth*3/4, iYInFrame+iWidth);
            break;
        case SIZE_NONE:
            break;
        default:
            Q_ASSERT(false);
        }
    }


    return true;
}
