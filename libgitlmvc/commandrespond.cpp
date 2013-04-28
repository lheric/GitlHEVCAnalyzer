#include "commandrespond.h"

#include <QDebug>

CommandRespond::CommandRespond()
{
}

bool CommandRespond::hasParameter(QString strParam) const
{
    return m_cRespond.contains(strParam);
}

bool CommandRespond::getParameter(QString strParam, QVariant& rvValue) const
{
    if( m_cRespond.contains(strParam) )
    {
        rvValue = m_cRespond[strParam];
        return true;
    }
    qWarning() << QString("Respond Parameter %1 NOT found.").arg(strParam);

    return false;
}

bool CommandRespond::setParameter(QString strParam, const QVariant& rvValue)
{
    m_cRespond[strParam] = rvValue;
    return true;
}
