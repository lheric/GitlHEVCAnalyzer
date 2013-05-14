#include "mccdisplayfilter.h"
#include <QRect>
#include <QColor>
#include <QBrush>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QString>
#include <QDebug>

#include <iostream>
using namespace std;

MCCDisplayFilter::MCCDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("MCC Display");
}

bool MCCDisplayFilter::init(FilterContext *pcContext)
{
    QString strHightlightFilename = "mccVal.txt";
    QString strOneLine;
    QRegExp cMatchTarget;

    QFile cFile;
    QTextStream cInputStream;
    LCUAddr cLCUAddr;

    m_cLCUMCC.clear();

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

            int iMCC = cMatchTarget.cap(3).toInt();
            cLCUAddr.iPoc  = iPoc;
            cLCUAddr.iAddr = iAddr;
            m_cLCUMCC.insert(cLCUAddr, iMCC);
        }

    }

    cFile.close();
    return true;
}

bool MCCDisplayFilter::drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                                 ComCU *pcCTU, double dScale, QRect* pcScaledArea)
{
    int iPoc  = pcCTU->getFrame()->getPoc();
    int iAddr = pcCTU->getAddr();
    LCUAddr cLCUAddr;
    cLCUAddr.iPoc = iPoc;
    cLCUAddr.iAddr = iAddr;
    int iMCC = m_cLCUMCC.find(cLCUAddr).value();

    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->setPen(QColor(255,0,0));
    QFont cFont = pcPainter->font();
    pcPainter->setPen(QColor(0,0,255));
    cFont.setPointSize(18);
    pcPainter->setFont(cFont);
    pcPainter->drawText(*pcScaledArea,Qt::AlignCenter, QString("%1").arg(iMCC));


    int iAlpha = 255-iMCC*15;
    iAlpha=((iAlpha<0)?(0):(iAlpha));
    iAlpha=((iAlpha>175)?(175):(iAlpha));
    pcPainter->setPen(Qt::NoPen);
    pcPainter->setBrush(QColor(0,0,0,iAlpha));
    pcPainter->drawRect(*pcScaledArea);



    return true;
}
