#include "highlightlcufilter.h"
#include "io/analyzermsgsender.h"
#include <QFile>
#include <QTextStream>


HighlightLCUFilter::HighlightLCUFilter(QObject *parent) :
    QObject(parent)
{
    setName("Hightlight LCU");
}


bool HighlightLCUFilter::init(FilterContext* pcContext)
{
    /// read CU address needed to be hightlighted
    /// FORMAT <POC,CU_Raster_Order>
    QString strHightlightFilename = "highlightlcu.txt";
    QString strOneLine;
    QRegExp cMatchTarget;

    QFile cFile;
    QTextStream cInputStream;

    cFile.setFileName(strHightlightFilename);
    if( !(cFile.open(QIODevice::ReadOnly)) )
        return false;

    cInputStream.setDevice(&cFile);

    QTextStream *pcInputStream = &cInputStream;

    cMatchTarget.setPattern("<([0-9]+),([0-9]+)>");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            int iAddr = cMatchTarget.cap(2).toInt();
            m_cTargetLCU.insert(iPoc,iAddr);

        }

    }

    cFile.close();
    //AnalyzerMsgSender::getInstance()->msgOut(QString("%1 LCU(s) Skipped").arg(m_cTargetLCU.size()));
    return true;
}




bool HighlightLCUFilter::drawCTU (FilterContext* pcContext, QPainter* pcPainter,
                                  ComCU *pcCTU, double dScale, QRect* pcScaledArea)
{

    int iPoc  = pcCTU->getFrame()->getPoc();
    int iAddr = pcCTU->getAddr();

    if( m_cTargetLCU.contains(iPoc, iAddr))
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

