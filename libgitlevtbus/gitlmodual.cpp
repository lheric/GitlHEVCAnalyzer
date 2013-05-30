#include "gitlmodual.h"
#include "gitleventbus.h"


GitlModual::GitlModual() :
    m_cDelegate(this)
{    
}

void GitlModual::subscribeToEvtByName( const QString& strEvtName )
{
    return m_cDelegate.subscribeToEvtByName(strEvtName);
}

void GitlModual::unsubscribeToEvtByName( const QString& strEvtName )
{
    return m_cDelegate.unsubscribeToEvtByName(strEvtName);
}

void GitlModual::dispatchEvt( GitlEvent& pcEvt )
{
    m_cDelegate.dispatchEvt(&pcEvt);
}

void GitlModual::setModualName( QString strModualName )
{
    m_cDelegate.setModualName(strModualName);
}
