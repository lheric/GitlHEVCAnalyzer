#ifndef ANALYZERMSGSENDER_H
#define ANALYZERMSGSENDER_H
#include "gitliomsg.h"
#include "gitldef.h"
#include "gitlmodual.h"

class AnalyzerMsgSender : public GitlIOMsg, public GitlModual
{
public:
    AnalyzerMsgSender();
    void msgOut( const QString& msg , GITL_MSG_LEVEL eMsgLevel = GITL_MSG_INFO );

    ///SINGLETON
    SINGLETON_PATTERN_DECLARE(AnalyzerMsgSender)
};

#endif // ANALYZERMSGSENDER_H
