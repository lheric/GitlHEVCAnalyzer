#ifndef COMMANDPARAMETER_H
#define COMMANDPARAMETER_H

#include <QMap>
#include <QVariant>

class CommandParameter
{

public:
    explicit CommandParameter();
    bool hasParameter(QString strParam) const;
    bool getParameter(QString strParam, QVariant& rvValue) const;
    bool setParameter(QString strParam, const QVariant& rvValue);

protected:
    QMap<QString,QVariant> m_cParameters;

};

#endif // COMMANDPARAMETER_H
