#include "reloadfilterscommand.h"
#include "model/modellocator.h"
ReloadFiltersCommand::ReloadFiltersCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool ReloadFiltersCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    FilterLoader* pFilterLoader = &pModel->getDrawEngine().getFilterLoader();
    pFilterLoader->reloadAllFilters();
    ///
    rcOutputArg.setParameter("filter_names",  QVariant::fromValue(pFilterLoader->getFilterNames())  );
    rcOutputArg.setParameter("filter_status", QVariant::fromValue(pFilterLoader->getEnableStatus()) );

    return true;
}
