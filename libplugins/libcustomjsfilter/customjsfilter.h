#ifndef CUDISPLAYFILTER_H
#define CUDISPLAYFILTER_H
#include "drawengine/abstractfilter.h"
#include <QObject>
#include <QtScript/QScriptEngine>

class CustomJSFilter : public QObject, public AbstractFilter
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cn.edu.gitl.sysu.gitlhevcanalyzer.AbstractFilter")
    Q_INTERFACES(AbstractFilter)
public:
    explicit CustomJSFilter(QObject *parent = 0);
    virtual bool init     (FilterContext* pcContext);
    virtual bool drawCU   (FilterContext* pcContext, QPainter* pcPainter,
                           ComCU *pcCU, double dScale,  QRect* pcScaledArea);
protected:
    bool xLoadFile(QString &strJSFilterSource);

signals:
    
public slots:
    ADD_CLASS_FIELD_PRIVATE(QString, strJSFilepath)
    ADD_CLASS_FIELD_PRIVATE(QScriptEngine, cScriptEngin)
    ADD_CLASS_FIELD_PRIVATE(QScriptValue, cJSFilter)
    
};

#endif // CUDISPLAYFILTER_H
