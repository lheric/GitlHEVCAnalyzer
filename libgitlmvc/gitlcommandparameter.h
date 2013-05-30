#ifndef GITLCOMMANDPARAMETER_H
#define GITLCOMMANDPARAMETER_H

#include <QMap>
#include <QVariant>

class GitlCommandParameter
{

public:
    explicit GitlCommandParameter();
    bool hasParameter(QString strParam) const;
    QVariant getParameter(QString strParam) const;
    bool setParameter(QString strParam, const QVariant& rvValue);

protected:
    QMap<QString,QVariant> m_cParameters;

};

#endif // GITLCOMMANDPARAMETER_H
