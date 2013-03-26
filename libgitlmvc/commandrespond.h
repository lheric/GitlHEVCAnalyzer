#ifndef COMMANDRESPOND_H
#define COMMANDRESPOND_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include "gitldef.h"

class CommandRespond
{
public:
    explicit CommandRespond();
    bool hasParameter(QString strParam) const;
    bool getParameter(QString strParam, QVariant& rvValue) const;
    bool setParameter(QString strParam, const QVariant& rvValue);

protected:
    QMap<QString,QVariant> m_cRespond;

signals:

public slots:

};

Q_DECLARE_METATYPE(CommandRespond)

#endif // COMMANDRESPOND_H
