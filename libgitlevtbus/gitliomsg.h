#ifndef GITLIOMSG_H
#define GITLIOMSG_H

#include <QObject>
#include <QTextStream>
#include <QMutex>
#include <QMutexLocker>
#include "gitldef.h"

enum GITL_MSG_LEVEL {
    GITL_MSG_DETIAL,
    GITL_MSG_DEBUG,
    GITL_MSG_INFO,
    GITL_MSG_WARNING,
    GITL_MSG_ERROR,
    GITL_MSG_FATAL
};



class GitlIOMsg
{
public:
    GitlIOMsg();
    void msgOut( const QString& msg , GITL_MSG_LEVEL eMsgLevel = GITL_MSG_INFO );

    ///
    ADD_CLASS_FIELD( GITL_MSG_LEVEL, eOuputLevel, getOuputLevel, setOutputLevel )
    ADD_CLASS_FIELD_NOSETTER( QTextStream, cMsgOutStream, getMsgOutStream )
    ADD_CLASS_FIELD_PRIVATE( QMutex, cOutStreamMutex )
    ///SINGLETON
    SINGLETON_PATTERN_DECLARE(GitlIOMsg)

signals:

public slots:

};

#endif // IOMESSAGE_H
