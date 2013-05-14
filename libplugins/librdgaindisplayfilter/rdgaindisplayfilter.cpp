#include "rdgaindisplayfilter.h"
#include <QFile>
#include <QTextStream>

RDGainDisplayFilter::RDGainDisplayFilter(QObject *parent) :
    QObject(parent)
{
    setName("R-D Gain Display");
}

bool RDGainDisplayFilter::init(FilterContext *pcContext)
{
    QString strInputFilename;
    QString strOneLine;
    QRegExp cMatchTarget;
    QFile cFile;
    QTextStream cInputStream;
    QTextStream *pcInputStream = &cInputStream;
    LCUAddr cLCUAddr;

    ///read original cost
    strInputFilename = "ori.txt";
    m_cOriCost.clear();
    m_cDisFMECost.clear();
    m_cDisIMECost.clear();
    m_cDisMDCost.clear();

    cFile.setFileName(strInputFilename);
    if( !(cFile.open(QIODevice::ReadOnly)) )
        return false;

    cInputStream.setDevice(&cFile);

    cMatchTarget.setPattern("<([0-9]+),([0-9]+)>([0-9]+)");

    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            int iAddr = cMatchTarget.cap(2).toInt();

            long lCost = cMatchTarget.cap(3).toLong();

            cLCUAddr.iPoc  = iPoc;
            cLCUAddr.iAddr = iAddr;
            m_cOriCost.insert(cLCUAddr, lCost);
        }

    }
    cFile.close();

    ///read disable MD cost
    strInputFilename = "disMD.txt";
    m_cDisMDCost.clear();
    cFile.setFileName(strInputFilename);
    if( !(cFile.open(QIODevice::ReadOnly)) )
        return false;

    cInputStream.setDevice(&cFile);

    cMatchTarget.setPattern("<([0-9]+),([0-9]+)>([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            int iAddr = cMatchTarget.cap(2).toInt();

            long lCost = cMatchTarget.cap(3).toLong();

            cLCUAddr.iPoc  = iPoc;
            cLCUAddr.iAddr = iAddr;
            m_cDisMDCost.insert(cLCUAddr, lCost);
        }

    }
    cFile.close();

    ///read disable IME cost
    strInputFilename = "disIME.txt";
    m_cDisIMECost.clear();
    cFile.setFileName(strInputFilename);
    if( !(cFile.open(QIODevice::ReadOnly)) )
        return false;

    cInputStream.setDevice(&cFile);

    cMatchTarget.setPattern("<([0-9]+),([0-9]+)>([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            int iAddr = cMatchTarget.cap(2).toInt();

            long lCost = cMatchTarget.cap(3).toLong();

            cLCUAddr.iPoc  = iPoc;
            cLCUAddr.iAddr = iAddr;
            m_cDisIMECost.insert(cLCUAddr, lCost);
        }

    }
    cFile.close();

    ///read disable FME cost
    strInputFilename = "disFME.txt";
    m_cDisFMECost.clear();
    cFile.setFileName(strInputFilename);
    if( !(cFile.open(QIODevice::ReadOnly)) )
        return false;

    cInputStream.setDevice(&cFile);

    cMatchTarget.setPattern("<([0-9]+),([0-9]+)>([0-9]+)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 )
        {

            /// poc and lcu addr
            int iPoc = cMatchTarget.cap(1).toInt();
            int iAddr = cMatchTarget.cap(2).toInt();

            long lCost = cMatchTarget.cap(3).toLong();

            cLCUAddr.iPoc  = iPoc;
            cLCUAddr.iAddr = iAddr;
            m_cDisFMECost.insert(cLCUAddr, lCost);
        }

    }
    cFile.close();

    return true;
}


bool RDGainDisplayFilter::config   (FilterContext* pcContext)
{

    return true;
}


bool RDGainDisplayFilter::drawCTU  (FilterContext* pcContext, QPainter* pcPainter,
                                 ComCU *pcCTU, double dScale, QRect* pcScaledArea)
{
    int iPoc  = pcCTU->getFrame()->getPoc();
    int iAddr = pcCTU->getAddr();
    LCUAddr cLCUAddr;
    cLCUAddr.iPoc = iPoc;
    cLCUAddr.iAddr = iAddr;

    long lOriCost    = m_cOriCost.find(cLCUAddr).value();
    long lDisMDCost  = m_cDisMDCost.find(cLCUAddr).value();
    long lDisIMECost = m_cDisIMECost.find(cLCUAddr).value();
    long lDisFMECost = m_cDisFMECost.find(cLCUAddr).value();

    long lMDGain  = lDisMDCost  - lOriCost;
    long lIMEGain = lDisIMECost - lOriCost;
    long lFMEGain = lDisFMECost - lOriCost;


            //m_ccLCU[iPoc][iAddr];
    //cout << iAddr << " " << iPoc << " " << mcc << endl;
    pcPainter->setBrush(Qt::NoBrush);
    QFont cFont = pcPainter->font();
    pcPainter->setPen(QColor(255,0,0));
    cFont.setPointSize(14);
    pcPainter->setFont(cFont);
    //pcPainter->drawText(*pcScaledArea,Qt::AlignCenter, QString("%1\n%2").arg(lMDGain).arg(lFMEGain));

    /// draw LCU boundary
    QPen cPen;
    cPen.setWidth(2);
    cPen.setColor(QColor(Qt::red));
    pcPainter->setPen(cPen);
    pcPainter->setBrush(Qt::NoBrush);
    pcPainter->drawRect(*pcScaledArea);


    long lMaxRDGainDraw = 200000; /// draw max rd-gain
    VALUE_CLIP(-lMaxRDGainDraw,lMaxRDGainDraw,lMDGain);
    VALUE_CLIP(-lMaxRDGainDraw,lMaxRDGainDraw,lFMEGain);

    cPen.setWidth(1);
    /// draw MD Gain Rect
    QRect cMDGainRect = *pcScaledArea;
    cMDGainRect.setWidth(cMDGainRect.width()/3);
    cMDGainRect.setHeight((cMDGainRect.height()*lMDGain)/lMaxRDGainDraw);
    cMDGainRect.moveBottom(pcScaledArea->bottom());
    cMDGainRect.moveLeft(pcScaledArea->left()+1);
    pcPainter->setBrush(QColor(64,0,128,128));
    pcPainter->drawRect(cMDGainRect);

    /// draw IME Gain Rect
    QRect cIMEGainRect = *pcScaledArea;
    cIMEGainRect.setWidth(cIMEGainRect.width()/3);
    cIMEGainRect.setHeight((cIMEGainRect.height()*lIMEGain)/lMaxRDGainDraw);
    cIMEGainRect.moveBottom(pcScaledArea->bottom());
    cIMEGainRect.moveLeft(cMDGainRect.right()+1);
    pcPainter->setBrush(QColor(202,26,126,128));
    pcPainter->drawRect(cIMEGainRect);

    /// draw FME Gain Rect
    QRect cFMEGainRect = *pcScaledArea;
    cFMEGainRect.setWidth(cFMEGainRect.width()/3);
    cFMEGainRect.setHeight((cFMEGainRect.height()*lFMEGain)/lMaxRDGainDraw);
    cFMEGainRect.moveBottom(pcScaledArea->bottom());
    cFMEGainRect.moveLeft(cIMEGainRect.right()+1);
    pcPainter->setBrush(QColor(0,64,128,128));
    pcPainter->drawRect(cFMEGainRect);


    return true;
}
