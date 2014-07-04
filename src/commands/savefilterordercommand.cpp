#include "savefilterordercommand.h"
#include "model/modellocator.h"

SaveFilterOrderCommand::SaveFilterOrderCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool SaveFilterOrderCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    ModelLocator* pModel = ModelLocator::getInstance();
    FilterLoader& rcFilterLoader = pModel->getDrawEngine().getFilterLoader();

    QStringList cFilterOrderList = rcInputArg.getParameter("filter_order").toStringList();

    rcFilterLoader.sortFilters(cFilterOrderList);
    rcFilterLoader.saveFilterOrder();    ///< save new order to file
    //rcOutputArg.setParameter("filter_names",  QVariant::fromValue(rcFilterLoader.getFilterNames())  );
    //rcOutputArg.setParameter("filter_status", QVariant::fromValue(rcFilterLoader.getEnableStatus()) );
    return true;


}
