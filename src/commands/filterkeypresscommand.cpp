#include "filterkeypresscommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
FilterKeyPressCommand::FilterKeyPressCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
    this->setInWorkerThread(false);
}


bool FilterKeyPressCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    int iPressedKey = rcInputArg.getParameter("pressed_key").toInt();
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getDrawEngine().keyPress(iPressedKey);
    GitlIvkCmdEvt cRefreshEvt("refresh_screen");
    cRefreshEvt.dispatch();
    return true;
}
