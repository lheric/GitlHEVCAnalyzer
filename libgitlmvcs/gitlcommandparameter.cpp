#include "gitlcommandparameter.h"
#include <QDebug>
GitlCommandParameter::GitlCommandParameter()
{
}

bool GitlCommandParameter::hasParameter(QString strParam) const
{
    return m_cParameters.contains(strParam);
}

QVariant GitlCommandParameter::getParameter(QString strParam) const
{
    if( m_cParameters.contains(strParam) )
    {
        return m_cParameters[strParam];
    }
    qWarning() << QString("Respond Parameter %1 NOT found.").arg(strParam);

    return QVariant();
}

bool GitlCommandParameter::setParameter(QString strParam, const QVariant& rvValue)
{
    m_cParameters[strParam] = rvValue;
    return true;
}
