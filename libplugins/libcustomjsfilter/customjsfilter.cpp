#include "customjsfilter.h"
#include <QFile>
CustomJSFilter::CustomJSFilter(QObject *parent) :
    QObject(parent)
{
    setName("Custom Javascript Filter");
    m_strJSFilepath = "custom_js_filter.js";
}

bool CustomJSFilter::init (FilterContext* pcContext)
{
    QString strJSFilterSource;
    if( !xLoadFile(strJSFilterSource) )
    {
        return false;
    }
    m_cJSFilter = m_cScriptEngin.evaluate(strJSFilterSource);
    return true;
}

bool CustomJSFilter::drawCU   (FilterContext* pcContext, QPainter* pcPainter,
                                ComCU *pcCU, double dScale,  QRect* pcScaledArea)
{
    /// Scaled CU Area
//    QPoint cLeftTop(iCUX*dScale+0.5, iCUY*dScale+0.5);
//    QPoint cBottomRight((iCUX+iCUSize)*dScale-0.5, (iCUY+iCUSize)*dScale-0.5);
//    QRect cCUArea(cLeftTop, cBottomRight);

    /// Draw CU Rect
//    pcPainter->setBrush(Qt::NoBrush);
//    pcPainter->setPen(QColor(255,255,255,128));
//    pcPainter->drawRect(*pcScaledArea);
    QScriptValue cJSFunction = m_cScriptEngin.globalObject().property("test");
    cJSFunction.call(QScriptValue(),QScriptValueList());
    return true;
}

bool CustomJSFilter::xLoadFile(QString& strJSFilterSource)
{
    QFile cFile(m_strJSFilepath);
    if( !cFile.open(QFile::ReadOnly) )
        return false;

    strJSFilterSource = QLatin1String(cFile.readAll());
    cFile.close();
    return true;
}
