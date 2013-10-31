#include "switchfiltercommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
#include <QDebug>
SwitchFilterCommand::SwitchFilterCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool SwitchFilterCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    ModelLocator* pModel = ModelLocator::getInstance();
    FilterLoader* pFilterLoader = &pModel->getDrawEngine().getFilterLoader();
    QString strFiltername = rcInputArg.getParameter("filter_name").toString();
    bool bEnable = rcInputArg.getParameter("filter_enable").toBool();
    AbstractFilter* pcFilter = pFilterLoader->getFilterByName(strFiltername);
    if( pcFilter == NULL )
    {
        qCritical() << QString("Cannot find filter %1").arg(strFiltername);
        return false;
    }
    pcFilter->setEnable(bEnable);
    /// refresh
    GitlIvkCmdEvt cRefreshEvt("refresh_screen");
    cRefreshEvt.dispatch();
    ///
    rcOutputArg.setParameter("filter_names",  QVariant::fromValue(pFilterLoader->getFilterNames())  );
    rcOutputArg.setParameter("filter_status", QVariant::fromValue(pFilterLoader->getEnableStatus()) );
    return true;
}
