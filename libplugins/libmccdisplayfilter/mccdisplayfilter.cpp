#include "mccdisplayfilter.h"
#include <QRect>
#include <QColor>
#include <QBrush>
#include <iostream>
#include <QFont>
using namespace std;
MCCDisplayFilter::MCCDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("MCC Display Filter");
    ///read file
}

bool MCCDisplayFilter::init(FilterContext *pcContext)
{
    QString strHightlightFilename = "mccVal.txt";
    QString strOneLine;
    QRegExp cMatchTarget;

    QFile cFile;
    QTextStream cInputStream;

    cFile.setFileName(strHightlightFilename);
    if( !(cFile.open(QIODevice::ReadOnly)) )
        return false;

    cInputStream.setDevice(&cFile);

    QTextStream *pcInputStream = &cInputStream;

    cMatchTarget.setPattern("<([0-9]+),([0-9]+)>([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            int iAddr = cMatchTarget.cap(2).toInt();

            int mcc = cMatchTarget.cap(3).toInt();
             m_ccLCU[iPoc][iAddr] = mcc;
        }

    }

    cFile.close();
    //AnalyzerMsgSender::getInstance()->msgOut(QString("%1 LCU(s) Skipped").arg(m_cTargetLCU.size()));
    return true;
}

bool MCCDisplayFilter::drawCTU  (QPainter* pcPainter,
                       FilterContext* pcContext,
                       ComSequence* pcSequence,
                       int iPoc, int iAddr,
                       int iCTUX, int iCTUY,
                       int iCTUSize)
{



    int mcc = m_ccLCU[iPoc][iAddr];
    //cout << iAddr << " " << iPoc << " " << mcc << endl;
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,0,0));
    QRect cCTURect(iCTUX, iCTUY, iCTUSize, iCTUSize);
    QFont cFont = pcPainter->font();
    pcPainter->setPen(QColor(0,0,255));
    cFont.setPointSize(18);
    pcPainter->setFont(cFont);
    pcPainter->drawText(cCTURect,Qt::AlignCenter, QString("%1").arg(mcc));


    //if(mcc>6)
    //{
    int iAlpha = 255-mcc*15;
    iAlpha=((iAlpha<0)?(0):(iAlpha));
    iAlpha=((iAlpha>175)?(175):(iAlpha));
    pcPainter->setPen(Qt::NoPen);
    pcPainter->setBrush(QColor(0,0,0,iAlpha));
    pcPainter->drawRect(cCTURect);
    //}
//    else if(mcc == 0)
//    {
//        pcPainter->setPen(QColor(0,255,0));
//        pcPainter->drawRect(cCTURect);
//    }


    return true;
}
