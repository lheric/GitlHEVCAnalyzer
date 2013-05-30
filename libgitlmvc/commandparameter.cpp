#include "commandparameter.h"
#include <QDebug>
CommandParameter::CommandParameter()
{
}

bool CommandParameter::hasParameter(QString strParam) const
{
    return m_cParameters.contains(strParam);
}

bool CommandParameter::getParameter(QString strParam, QVariant& rvValue) const
{
    if( m_cParameters.contains(strParam) )
    {
        rvValue = m_cParameters[strParam];
        return true;
    }
    qWarning() << QString("Respond Parameter %1 NOT found.").arg(strParam);

    return false;
}

bool CommandParameter::setParameter(QString strParam, const QVariant& rvValue)
{
    m_cParameters[strParam] = rvValue;
    return true;
}
