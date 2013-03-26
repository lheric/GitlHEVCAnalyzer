#ifndef GITLMODUALDELEGATE_H
#define GITLMODUALDELEGATE_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include "gitldef.h"
#include "gitlevent.h"

class GitlModual;
class GitlEventBus;

class GitlModualDelegate : public QObject
{
    Q_OBJECT
public:
    explicit GitlModualDelegate(GitlModual *pcDelegator);

public slots:
    /*! Event Bus Listener
     */
    bool detonate( GitlEvent cEvt );

public:
    bool listenToEvtByName( const QString& strEvtName );
    bool dispatchEvt( GitlEvent* pcEvt );

protected:
    bool xIsListenToEvt( const QString& strEvtName );




    ADD_CLASS_FIELD( QString, strModualName, getModualName, setModualName )
    ADD_CLASS_FIELD_PRIVATE( QVector<QString>, cListeningEvts )
    ADD_CLASS_FIELD_NOSETTER( GitlEventBus*, pcGitlEvtBus, getGitlEvtBus )
    ADD_CLASS_FIELD_PRIVATE(GitlModual*, pcDelegator)
    ADD_CLASS_FIELD_PRIVATE( QMutex, cModualMutex )
    
};

#endif // GITLMODUALDELEGATE_H
