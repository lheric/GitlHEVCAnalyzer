#include "mergedisplayfilter.h"
#include <QRect>

MergeDisplayFilter::MergeDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("Merge Display Filter");
}

bool MergeDisplayFilter::drawPU   (FilterContext* pcContext, QPainter* pcPainter,
                                   ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{
    int iMergeIndex = pcPU->getMergeIndex();
    QColor cPUColor;



    if( iMergeIndex == -1 )    ///< not merge mode
    {
        //do nothing
    }
    else
    {
//        if(abs(pcPU->getMV(0)->getHor())>0 || abs(pcPU->getMV(0)->getVer())>0)
        {
            cPUColor = QColor(Qt::green);
            cPUColor.setAlphaF(0.3);
            pcPainter->setPen(Qt::NoPen);
            pcPainter->setBrush(QBrush(cPUColor));

            pcPainter->drawRect(*pcScaledArea);
        }

    }
    return true;

}
