#include "mvdisplayfilter.h"

MVDisplayFilter::MVDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("MV Display Filter");
}

bool MVDisplayFilter::drawPU   (QPainter* pcPainter,
                                           FilterContext* pcContext,
                                           ComSequence* pcSequence,
                                           ComPU* pcPU,
                                           int iPoc, int iAddr,
                                           int iZOrder, int iDepth,
                                           PartSize ePartSize, int iPUIndex,
                                           int iPUX, int iPUY,
                                           int iPUWidth, int iPUHeight, double dScale)
{
    int iInterDir = pcPU->getInterDir();
    ComMV *pcMV = NULL;
    if( iInterDir == 0 )
    {
        /// Do nothing
    }
    else if( iInterDir == 1 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(0);

        pcPainter->setPen(QColor(Qt::blue));
        pcPainter->drawLine(iPUX  + iPUWidth/2,     iPUY + iPUHeight/2,
                          iPUX+iPUWidth/2+pcMV->getHor()/4, iPUY+iPUHeight/2+pcMV->getVer()/4);

    }
    else if( iInterDir == 2 )  /// uni-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(1);

        pcPainter->setPen(QColor(Qt::red));
        pcPainter->drawLine(iPUX  + iPUWidth/2,     iPUY + iPUHeight/2,
                          iPUX+iPUWidth/2+pcMV->getHor()/4, iPUY+iPUHeight/2+pcMV->getVer()/4);

    }
    else if( iInterDir == 3 )  /// bi-directional prediction
    {
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(0);

        pcPainter->setPen(QColor(Qt::blue));
        pcPainter->drawLine(iPUX  + iPUWidth/2,     iPUY + iPUHeight/2,
                          iPUX+iPUWidth/2+pcMV->getHor()/4, iPUY+iPUHeight/2+pcMV->getVer()/4);
        /// Get MV of PU
        pcMV = pcPU->getMVs().at(1);

        pcPainter->setPen(QColor(Qt::red));
        pcPainter->drawLine(iPUX  + iPUWidth/2,     iPUY + iPUHeight/2,
                          iPUX+iPUWidth/2+pcMV->getHor()/4, iPUY+iPUHeight/2+pcMV->getVer()/4);
    }
    return true;

}
