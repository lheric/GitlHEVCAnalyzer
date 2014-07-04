#include "zoomframecommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
ZoomFrameCommand::ZoomFrameCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool ZoomFrameCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    QVariant vValue;
    double dScale = 0.0;
    if( rcInputArg.hasParameter("scale") )
    {
        vValue = rcInputArg.getParameter("scale");
        dScale = vValue.toDouble();
    }
    else
    {
        return false;
    }
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getDrawEngine().setScale(dScale);
    rcOutputArg.setParameter("scale", dScale);
    /// refresh
    GitlIvkCmdEvt cRefreshEvt("refresh_screen");
    cRefreshEvt.dispatch();

    return true;

}
