#include "gitlmodualdelegate.h"
#include "gitleventbus.h"

GitlModualDelegate::GitlModualDelegate(GitlModual *pcDelegator)
{
    m_pcDelegator = pcDelegator;
    m_pcGitlEvtBus = GitlEventBus::getInstance();
    m_pcGitlEvtBus->registerModual(this);
    m_strModualName = "undefined_modual_name";

}



bool GitlModualDelegate::listenToEvtByName( const QString& strEvtName )
{
    m_cListeningEvts.push_back(strEvtName);
    return true;
}

bool GitlModualDelegate::detonate(GitlEvent cEvt )
{
    QMutexLocker locker(&m_cModualMutex);
    if( xIsListenToEvt(cEvt.getEvtName()) == true )
        this->m_pcDelegator->detonate(cEvt);
    return true;
}

bool GitlModualDelegate::xIsListenToEvt( const QString& strEvtName )
{

    for(int iEvtIdx = 0; iEvtIdx < m_cListeningEvts.size(); iEvtIdx++ )
    {
        if( strEvtName == m_cListeningEvts.at(iEvtIdx) )
        {
            return true;
        }
    }
    return false;
}

bool GitlModualDelegate::dispatchEvt( GitlEvent* pcEvt )
{
    m_pcGitlEvtBus->post(pcEvt);
    return true;
}
