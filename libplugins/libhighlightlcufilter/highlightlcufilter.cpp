#include "highlightlcufilter.h"
#include "io/analyzermsgsender.h"
#include <QFile>
#include <QTextStream>


HighlightLCUFilter::HighlightLCUFilter(QObject *parent) :
    QObject(parent)
{
    setName("Hightlight LCU Filter");
}


bool HighlightLCUFilter::init(FilterContext* pcContext)
{
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




bool HighlightLCUFilter::drawCTU  (QPainter* pcPainter,
                                   FilterContext* pcContext,
                                   ComSequence* pcSequence,
                                   int iPoc, int iAddr,
                                   int iCTUX, int iCTUY,
                                   int iCTUSize)
{


    if( m_cTargetLCU.contains(iPoc, iAddr))
    {
        QPen cPen;
        cPen.setWidth(2);
        cPen.setColor(QColor(Qt::red));
        pcPainter->setPen(cPen);
        pcPainter->setBrush(Qt::NoBrush);
        pcPainter->drawRect(iCTUX, iCTUY, iCTUSize, iCTUSize);
    }


    return true;
}

