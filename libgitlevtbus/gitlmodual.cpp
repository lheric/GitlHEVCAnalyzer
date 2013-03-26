#include "gitlmodual.h"
#include "gitleventbus.h"


GitlModual::GitlModual() :
    m_cDelegate(this)
{    
}

bool GitlModual::listenToEvtByName( const QString& strEvtName )
{
    return m_cDelegate.listenToEvtByName(strEvtName);
}

bool GitlModual::dispatchEvt( GitlEvent* pcEvt )
{
    return m_cDelegate.dispatchEvt(pcEvt);
}

void GitlModual::setModualName( QString strModualName )
{
    m_cDelegate.setModualName(strModualName);
}
