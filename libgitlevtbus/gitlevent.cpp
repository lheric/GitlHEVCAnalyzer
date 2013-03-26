#include "gitlevent.h"
#include "gitlmodual.h"


GitlEvent::GitlEvent( const QString& strEvtName )
{
    this->m_strEvtName = strEvtName;
}


GitlEvent::GitlEvent()
{
    this->m_strEvtName = "UNKNOWN";
}
