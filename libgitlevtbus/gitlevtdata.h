#ifndef EVTREQUEST_H
#define EVTREQUEST_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include "gitldef.h"

class GitlEvtData
{

public:
    GitlEvtData();
    bool hasParameter(QString strParam) const;
    bool getParameter(const QString& strParam, QVariant& rvValue) const;
    bool setParameter(QString strParam, QVariant rvValue);

protected:
    QMap<QString,QVariant> m_cRequest;
signals:

public slots:

};

#endif // COMMANDREQUEST_H
