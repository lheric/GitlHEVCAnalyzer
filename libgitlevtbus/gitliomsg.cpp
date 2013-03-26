#include "gitliomsg.h"
#include <iostream>
using namespace std;

SINGLETON_PATTERN_IMPLIMENT(GitlIOMsg)

GitlIOMsg::GitlIOMsg() :
    m_cMsgOutStream( stdout, QIODevice::WriteOnly )
{
    m_eOuputLevel = GITL_MSG_DEBUG;
}

void GitlIOMsg::msgOut( const QString& msg, GITL_MSG_LEVEL eMsgLevel )
{
    QMutexLocker cOutStreamLocker(&m_cOutStreamMutex);
    if( eMsgLevel < m_eOuputLevel )
        return;
    m_cMsgOutStream << msg << endl;
}
