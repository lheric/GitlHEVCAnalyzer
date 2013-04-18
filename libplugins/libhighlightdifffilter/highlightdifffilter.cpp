#include "highlightdifffilter.h"
#include "model/modellocator.h"

HighlightDiffFilter::HighlightDiffFilter(QObject *parent) :
    QObject(parent)
{
    setName("HighlightDiffFilter");
}



bool HighlightDiffFilter::drawCTU  (QPainter* pcPainter,
                                    FilterContext* pcContext,
                                    ComSequence* pcSequence,
                                    int iPoc, int iAddr,
                                    int iCTUX, int iCTUY,
                                    int iCTUSize, double dScale)
{

//        ModelLocator* pModel = ModelLocator::getInstance();
//        ComLCU* pTestingLCU = pModel->getSequenceManager().getTestingSequence().getFrames().at(iPoc)->getLCUs().at(iAddr);
//        ComLCU* pBenchmarkLCU = pModel->getSequenceManager().getBenchmarkSequence().getFrames().at(iPoc)->getLCUs().at(iAddr);



//        bool bSameLCUMode = true;
//        if(pTestingLCU->getRecrMode().size() != pBenchmarkLCU->getRecrMode().size())
//        {
//            bSameLCUMode = false;
//        }
//        else
//        {
//            for(int i = 0; i < pTestingLCU->getRecrMode().size(); i++)
//            {
//                int iTestingMode = pTestingLCU->getRecrMode().at(i);
//                int iBenchmarkMode = pBenchmarkLCU->getRecrMode().at(i);
//                if( (iTestingMode == CU_SLIPT_FLAG) || (iBenchmarkMode == CU_SLIPT_FLAG) )
//                {
//                    if(iTestingMode != iBenchmarkMode)
//                    {
//                        bSameLCUMode = false;
//                        break;
//                    }
//                }
//            }
//        }

//        if(!bSameLCUMode)
//        {
//            QPen cPen;
//            cPen.setWidth(2);
//            cPen.setColor(QColor(Qt::red));
//            pcPainter->setPen(cPen);
//            pcPainter->setBrush(Qt::NoBrush);
//            pcPainter->drawRect(iX, iY, iWidth, iHeight);
//        }


    return true;
}


