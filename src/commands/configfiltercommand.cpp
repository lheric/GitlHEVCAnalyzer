#include "configfiltercommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
#include <QString>

ConfigFilterCommand::ConfigFilterCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool ConfigFilterCommand::execute( GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    QString strFiltername = rcInputArg.getParameter("filter_name").toString();
    AbstractFilter* pcFilter = pModel->getDrawEngine().getFilterLoader().getFilterByName(strFiltername);
    if(pcFilter == NULL)
        return false;
    pModel->getDrawEngine().getFilterLoader().config(pcFilter);

    GitlIvkCmdEvt cRefreshEvt("refresh_screen");
    cRefreshEvt.dispatch();

    return true;
}
