#include "gitlcommandparameter.h"
#include <QDebug>
GitlCommandParameter::GitlCommandParameter()
{
}

bool GitlCommandParameter::hasParameter(QString strParam) const
{
    return m_cParameters.contains(strParam);
}

bool GitlCommandParameter::getParameter(QString strParam, QVariant& rvValue) const
{
    if( m_cParameters.contains(strParam) )
    {
        rvValue = m_cParameters[strParam];
        return true;
    }
    qWarning() << QString("Respond Parameter %1 NOT found.").arg(strParam);

    return false;
}

bool GitlCommandParameter::setParameter(QString strParam, const QVariant& rvValue)
{
    m_cParameters[strParam] = rvValue;
    return true;
}
