#ifndef GITLBLOCKEVTDISPATCHER_H
#define GITLBLOCKEVTDISPATCHER_H


#include <QObject>
#include <QList>
#include "gitlmodual.h"
#include "gitlevent.h"
#include "gitldef.h"

class GitlBlockEvtDispatcher : public QObject
{
    Q_OBJECT
public:
    GitlBlockEvtDispatcher();
    ~GitlBlockEvtDispatcher();
    /*! blocking notification
      */
    void postEvent(GitlEvent* pcEvt);

signals:
    /*! message to send
     */
    void eventTriggered(GitlEvent pcEvt);



};

#endif // GITLBLOCKEVTDISPATCHER_H
