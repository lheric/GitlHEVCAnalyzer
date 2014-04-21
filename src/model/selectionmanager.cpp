#include "selectionmanager.h"
#include <QRect>
#include <QDebug>
SelectionManager::SelectionManager()
{
}

ComCU *SelectionManager::getLCU(ComFrame* pcFrame, const QPointF* pcUnscaledPos)
{
    int iWidth = pcFrame->getSequence()->getWidth();
    int iHeight = pcFrame->getSequence()->getHeight();
    QRectF cFrameRect(0,0,iWidth, iHeight);
    if(cFrameRect.contains(*pcUnscaledPos))
    {
        int iLCUSize = pcFrame->getSequence()->getMaxCUSize();
        int iLCUInRow = qCeil(iWidth*1.0/iLCUSize);
        int iRow = qFloor( pcUnscaledPos->y()/iLCUSize );
        int iCol = qFloor( pcUnscaledPos->x()/iLCUSize );
        int iIndex = iRow*iLCUInRow + iCol;
        return pcFrame->getLCUs().at(iIndex);
    }
    return NULL;
}

ComCU *SelectionManager::getSCU(ComFrame* pcFrame, const QPointF* pcUnscaledPos)
{
    ComCU* pcLCU = getLCU(pcFrame, pcUnscaledPos);
    ComCU* pcCurCU = pcLCU;
    if(pcLCU != NULL)
    {
        while(!pcCurCU->getSCUs().empty())
        {
            foreach(ComCU* pcSCU, pcCurCU->getSCUs())
            {
                QRectF cSCURect(pcSCU->getX(), pcSCU->getY(), pcSCU->getSize(), pcSCU->getSize());
                if(cSCURect.contains(*pcUnscaledPos))
                {
                    pcCurCU = pcSCU;
                    break;
                }
            }
        }
        return pcCurCU;
    }
    return NULL;
}

ComPU *SelectionManager::getPU(ComFrame* pcFrame, const QPointF* pcUnscaledPos)
{
    ComCU* pcSCU = getSCU(pcFrame, pcUnscaledPos);
    if(pcSCU != NULL)
    {
        foreach(ComPU* pcPU, pcSCU->getPUs())
        {
            QRectF cPURect(pcPU->getX(), pcPU->getY(), pcPU->getWidth(), pcPU->getHeight());
            if(cPURect.contains(*pcUnscaledPos))
            {
                return pcPU;
            }
        }
    }
    return NULL;
}

ComTU *SelectionManager::getTU(ComFrame* pcFrame, const QPointF* pcUnscaledPos)
{
    ComCU* pcSCU = getSCU(pcFrame, pcUnscaledPos);

    if(pcSCU != NULL)
    {
        ComTU* pcCurTU = &pcSCU->getTURoot();
        while(!pcCurTU->getTUs().empty())
        {
            foreach(ComTU* pcTU, pcCurTU->getTUs())
            {
                QRectF cTURect(pcTU->getX(), pcTU->getY(), pcTU->getSize(), pcTU->getSize());
                if(cTURect.contains(*pcUnscaledPos))
                {
                    pcCurTU = pcTU;
                    break;
                }
            }
        }
        return pcCurTU;
    }
    return NULL;
}
