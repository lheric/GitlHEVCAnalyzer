#include "analyzermsgsender.h"
#include "events/eventnames.h"
#include "gitlupdateuievt.h"

///SINGLETON
SINGLETON_PATTERN_IMPLIMENT(AnalyzerMsgSender)

AnalyzerMsgSender::AnalyzerMsgSender()
{
}

void AnalyzerMsgSender::msgOut( const QString& strMsg , QtMsgType eMsgLevel)
{    
    GitlUpdateUIEvt cEvt;
    cEvt.setParameter("msg_detail", strMsg);
    cEvt.setParameter("msg_level",(int)eMsgLevel);
    dispatchEvt(cEvt);
}
