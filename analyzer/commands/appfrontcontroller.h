#ifndef APPFRONTCONTROLLER_H
#define APPFRONTCONTROLLER_H

#include "gitlfrontcontroller.h"
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include "gitlmodual.h"

class AppFrontController : public QThread, protected GitlFrontController, public GitlModual
{
public:
    AppFrontController();
    /*! The concrete action to deal with specific event
      */
    virtual bool detonate( GitlEvent& cEvt );

protected:
    bool xInitCommand();
    virtual void run();
    //SINGLETON
    SINGLETON_PATTERN_DECLARE(AppFrontController)


    ADD_CLASS_FIELD_PRIVATE(QList<GitlEvent>, cEvtQue)
    ADD_CLASS_FIELD_PRIVATE(QMutex, cEvtQueMutex)
    ADD_CLASS_FIELD_PRIVATE(QWaitCondition, cEvtQueNotEmpty)
    ADD_CLASS_FIELD_PRIVATE(QWaitCondition, cEvtQueNotFull)
    ADD_CLASS_FIELD(int, iMaxEvtInQue, setMaxEvtInQue, getMaxEvtInQue)
};

#endif // APPFRONTCONTROLLER_H
