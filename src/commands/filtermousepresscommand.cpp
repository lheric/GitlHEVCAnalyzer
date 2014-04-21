#include "filtermousepresscommand.h"
#include <QPointF>
#include <QMouseEvent>
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"

FilterMousePressCommand::FilterMousePressCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
    setInWorkerThread(false);
}

bool FilterMousePressCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    QPointF cScaledPoint = rcInputArg.getParameter("scaled_point").toPointF();
    Qt::MouseButton eMouseBtn = static_cast<Qt::MouseButton>(rcInputArg.getParameter("mouse_button").toInt());
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getDrawEngine().mousePress(&cScaledPoint, eMouseBtn);
    GitlIvkCmdEvt cRefreshEvt("refresh_screen");
    cRefreshEvt.dispatch();
    return true;
}
