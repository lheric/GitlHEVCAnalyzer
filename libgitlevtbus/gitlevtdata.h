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
    QVariant getParameter(const QString& strParam ) const;
    bool setParameter(const QString& strParam, const QVariant& rvValue);

    QMap<QString, QVariant> cRequest() const;

protected:
    QMap<QString,QVariant> m_cParameters;  //TODO rename
signals:

public slots:

};

#endif // COMMANDREQUEST_H
