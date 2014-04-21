#include "cudisplayfilter.h"
#include "model/modellocator.h"
#include <QDebug>
CUDisplayFilter::CUDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("CU Structure");

    ///
    m_cConfigDialog.setWindowTitle("CU Structure Filter");
    m_cConfigDialog.addCheckbox("Display PU", "", &m_cConfig.getShowPU());
    m_cConfigDialog.addCheckbox("Only Disaly LCU", "", &m_cConfig.getShowLCUOnly());
    m_cConfigDialog.addColorPicker("PU Color", &m_cConfig.getPUColor());
    m_cConfigDialog.addColorPicker("Largest CU Color (LCU/CTU)", &m_cConfig.getLCUColor());
    m_cConfigDialog.addColorPicker("Leaf CU Color (Smallest CU)", &m_cConfig.getSCUColor());
    m_cConfigDialog.addSlider("Opaque", 0.1, 1.0, &m_cConfig.getOpaque() );

    /// init lcu pen
    m_cLCUPen.setStyle(Qt::SolidLine);
    m_cLCUPen.setWidth(3);
    m_cLCUPen.setBrush(QBrush(m_cConfig.getLCUColor()));

    /// init cu pen
    m_cCUPen.setStyle(Qt::SolidLine);
    m_cCUPen.setWidth(1);
    m_cCUPen.setBrush(QBrush(m_cConfig.getSCUColor()));

    /// init pu pen
    m_cPUPen.setStyle(Qt::DashLine);
    m_cPUPen.setWidth(1);
    m_cPUPen.setBrush(QBrush(m_cConfig.getPUColor()));

    /// selected CU
    m_pcSelectedCU = NULL;


}

bool CUDisplayFilter::config   (FilterContext* pcContext)
{
    m_cConfigDialog.exec();
    m_cConfig.applyOpaque();
    m_cLCUPen.setBrush(QBrush(m_cConfig.getLCUColor()));
    m_cCUPen.setBrush(QBrush(m_cConfig.getSCUColor()));
    m_cPUPen.setBrush(QBrush(m_cConfig.getPUColor()));
    return true;
}


bool CUDisplayFilter::drawCTU  (FilterContext *pcContext, QPainter *pcPainter,
                                ComCU *pcCTU, double dScale, QRect *pcScaledArea)
{
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(m_cLCUPen);
    pcPainter->drawRect(*pcScaledArea);
    return true;
}

bool CUDisplayFilter::drawCU   (FilterContext* pcContext, QPainter* pcPainter,
                                ComCU *pcCU, double dScale,  QRect* pcScaledArea)
{
    /// only show LCU
    if(m_cConfig.getShowLCUOnly())
        return true;

    /// Draw CU Rect


    if(pcCU == m_pcSelectedCU)      /// selected
    {
        QPen cSelectedPen = m_cCUPen;
        cSelectedPen.setWidth(m_cCUPen.width()+3);
        pcPainter->setBrush(QBrush(QColor(255,0,0,128)));
        pcPainter->setPen(cSelectedPen);
    }
    else
    {
        pcPainter->setBrush(Qt::NoBrush);
        pcPainter->setPen(m_cCUPen);
    }
    pcPainter->drawRect(*pcScaledArea);

    /// Draw PU
    if(m_cConfig.getShowPU())
    {
        pcPainter->setPen(m_cPUPen);

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


bool CUDisplayFilter::mousePress(FilterContext *pcContext, QPainter *pcPainter, ComFrame *pcFrame, const QPointF *pcUnscaledPos, const QPointF *scaledPos, double dScale, Qt::MouseButton eMouseBtn)
{
    m_pcSelectedCU = pcContext->pcSelectionManager->getSCU(pcFrame, pcUnscaledPos);
    return true;
}

bool CUDisplayFilter::keyPress(FilterContext *pcContext, QPainter *pcPainter, ComFrame *pcFrame, int iKeyPressed)
{
    if(iKeyPressed == Qt::Key_Escape)
        m_pcSelectedCU = NULL;
    return true;
}
