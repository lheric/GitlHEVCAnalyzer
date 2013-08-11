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
    rcOutputArg.setParameter("filters", QVariant::fromValue((void*)(&pFilterLoader->getFilters())));

    return true;
}
