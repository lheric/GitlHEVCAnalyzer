#ifndef GITLCOMMANDPARAMETER_H
#define GITLCOMMANDPARAMETER_H

#include <QMap>
#include <QVariant>

class GitlCommandParameter
{

public:
    explicit GitlCommandParameter();
    bool hasParameter(QString strParam) const;
    bool getParameter(QString strParam, QVariant& rvValue) const;
    bool setParameter(QString strParam, const QVariant& rvValue);

protected:
    QMap<QString,QVariant> m_cParameters;

};

#endif // GITLCOMMANDPARAMETER_H
