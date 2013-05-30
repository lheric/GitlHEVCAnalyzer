#include "gitlevtdata.h"

#include <QDebug>

GitlEvtData::GitlEvtData()
{
}

bool GitlEvtData::hasParameter(QString strParam) const
{
    return m_cParameters.contains(strParam);
}

QVariant GitlEvtData::getParameter(const QString& strParam ) const
{
    QVariant rvValue;
    if( m_cParameters.contains(strParam) )
    {
        rvValue = m_cParameters[strParam];
    }
    else
    {
        qWarning() << QString("Respond Parameter %1 NOT found.").arg(strParam);
    }
    return rvValue;
}

bool GitlEvtData::setParameter(const QString& strParam, const QVariant& rvValue)
{
    m_cParameters[strParam] = rvValue;
    return true;
}

QMap<QString, QVariant> GitlEvtData::cRequest() const
{
    return m_cParameters;
}


