#include "reloadfilterscommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
#include <QApplication>
ReloadFiltersCommand::ReloadFiltersCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
    setInWorkerThread(false);
}

bool ReloadFiltersCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{

    ModelLocator* pModel = ModelLocator::getInstance();
    FilterLoader* pFilterLoader = &pModel->getDrawEngine().getFilterLoader();
    pFilterLoader->reloadAllFilters();
    /// refresh screen
    GitlIvkCmdEvt cRefresh("refresh_screen");
    cRefresh.dispatch();
    ///
    rcOutputArg.setParameter("filter_names",  QVariant::fromValue(pFilterLoader->getFilterNames())  );
    rcOutputArg.setParameter("filter_status", QVariant::fromValue(pFilterLoader->getEnableStatus()) );

    return true;
}
