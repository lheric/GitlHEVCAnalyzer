#include "reloadfilterscommand.h"
#include "model/modellocator.h"
ReloadFiltersCommand::ReloadFiltersCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool ReloadFiltersCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    FilterLoader* pFilterLoader = &pModel->getDrawEngine().getFilterLoader();
    pFilterLoader->reloadAllFilters();
    ///
    rcRespond.setParameter("filters", QVariant::fromValue((void*)(&pFilterLoader->getFilters())));

    return true;
}
