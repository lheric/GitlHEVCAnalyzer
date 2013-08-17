#ifndef ANALYZERMSGSENDER_H
#define ANALYZERMSGSENDER_H
#include "gitldef.h"
#include "gitlmodual.h"
#include <QDebug>

class AnalyzerMsgSender : public GitlModual
{
public:
    AnalyzerMsgSender();
    void msgOut(const QString& strMsg, QtMsgType eMsgLevel = QtDebugMsg);

    ///SINGLETON
    SINGLETON_PATTERN_DECLARE(AnalyzerMsgSender)
};

#endif // ANALYZERMSGSENDER_H
