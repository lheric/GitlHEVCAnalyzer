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
    friend class GitlModual;
private:
    explicit GitlModualDelegate(GitlModual *pcDelegator);

public:
    /*!
     * \brief subscribeToEvtByName listening to an event by name
     * \param strEvtName event name
     */
    void subscribeToEvtByName( const QString& strEvtName );

    /*!
     * \brief subscribeToEvtByName not listening to an event by name
     * \param strEvtName event name
     */
    void unsubscribeToEvtByName( const QString& strEvtName );

    /*!
     * \brief dispatchEvt dispatch an event to subscribers
     * \param pcEvt event
     */
    void dispatchEvt( GitlEvent* pcEvt );

public slots:
    /*!
     * \brief detonate notifyed by event bus
     * \param cEvt
     * \return
     */
    bool detonate( GitlEvent cEvt );

protected:
    bool xIsListenToEvt( const QString& strEvtName );

    ADD_CLASS_FIELD( QString, strModualName, getModualName, setModualName )
    ADD_CLASS_FIELD_PRIVATE( QVector<QString>, cListeningEvts )
    ADD_CLASS_FIELD_NOSETTER( GitlEventBus*, pcGitlEvtBus, getGitlEvtBus )
    ADD_CLASS_FIELD_PRIVATE(GitlModual*, pcDelegator)
    ADD_CLASS_FIELD_PRIVATE( QMutex, cModualMutex )
    
};

#endif // GITLMODUALDELEGATE_H
