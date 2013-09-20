#ifndef APPFRONTCONTROLLER_H
#define APPFRONTCONTROLLER_H

#include "gitlmtfrontcontroller.h"
//#include <QThread>
//#include <QMutex>
//#include <QMutexLocker>
//#include <QWaitCondition>
#include "gitlmodual.h"

class AppFrontController : protected GitlMTFrontController
{

protected:
    AppFrontController();
    bool xInitCommand();

    //SINGLETON
    SINGLETON_PATTERN_DECLARE(AppFrontController)

};

#endif // APPFRONTCONTROLLER_H
