#include "nzmvdisplayfilter.h"

NZMVDisplayFilter::NZMVDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("Non-Zero FMV Display Filter");
}

bool NZMVDisplayFilter::drawPU  (FilterContext* pcContext, QPainter* pcPainter,
                               ComPU *pcPU, double dScale,  QRect* pcScaledArea)
{
    int iInterDir = pcPU->getInterDir();   
    ComMV *pcMV = NULL;
    bool bNotZeroFME = false;
    if( iInterDir == 0 )
    {
        /// Do nothing
    }
    else if( iInterDir == 1 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(0);
        bNotZeroFME = xIsFractionalMV(pcMV);


    }
    else if( iInterDir == 2 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(1);
        bNotZeroFME = xIsFractionalMV(pcMV);

    }
    else if( iInterDir == 3 )  /// bi-directional prediction
    {
        /// Get MV of PU ( first direction )
        pcMV = pcPU->getMVs().at(0);
        bNotZeroFME = bNotZeroFME||xIsFractionalMV(pcMV);

        /// Get MV of PU ( second direction)
        pcMV = pcPU->getMVs().at(1);
        bNotZeroFME = bNotZeroFME||xIsFractionalMV(pcMV);

    }

    if(bNotZeroFME)
    {
        QPen cPen;
        cPen.setWidth(2);
        cPen.setColor(QColor(Qt::red));
        pcPainter->setPen(cPen);
        pcPainter->setBrush(Qt::NoBrush);
        pcPainter->drawRect(*pcScaledArea);
    }
    return true;

}

bool NZMVDisplayFilter::xIsFractionalMV(ComMV* pcMV)
{
    if((pcMV->getHor()%4 != 0) || (pcMV->getVer()%4 != 0))
        return true;
    return false;
}
