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
    if( iMergeIndex == -1 )    ///< -1 = not merge mode for this PU
    {
        //do nothing
    }
    else
    {
        QColor cPUColor = QColor(Qt::green);
        cPUColor.setAlphaF(0.3);
        pcPainter->setPen(Qt::NoPen);
        pcPainter->setBrush(QBrush(cPUColor));
        pcPainter->drawRect(*pcScaledArea);

    }
    return true;

}
