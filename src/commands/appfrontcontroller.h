#ifndef APPFRONTCONTROLLER_H
#define APPFRONTCONTROLLER_H

#include "gitlfrontcontroller.h"
#include "gitlmodual.h"

class AppFrontController : protected GitlFrontController
{
public:
    virtual void run();
protected:
    AppFrontController();
    bool xInitCommand();

    //SINGLETON
    SINGLETON_PATTERN_DECLARE(AppFrontController)

};

#endif // APPFRONTCONTROLLER_H
