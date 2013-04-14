#ifndef COMMANDREQUEST_H
#define COMMANDREQUEST_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include "gitldef.h"

class CommandRequest
{
public:
    explicit CommandRequest();
    bool hasParameter(QString strParam) const;
    bool getParameter(QString strParam, QVariant& rvValue) const;
    bool setParameter(QString strParam, const QVariant& rvValue);

protected:
    QMap<QString,QVariant> m_cRequest;


};

Q_DECLARE_METATYPE(CommandRequest)

#endif // COMMANDREQUEST_H
