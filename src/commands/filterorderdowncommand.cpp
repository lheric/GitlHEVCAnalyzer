#include "filterorderdowncommand.h"
#include "model/modellocator.h"
#include <QDebug>

FilterOrderDownCommand::FilterOrderDownCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool FilterOrderDownCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    FilterLoader& rcFilterLoader = pModel->getDrawEngine().getFilterLoader();

    QString strFiltername = rcInputArg.getParameter("filter_name").toString();
    if( rcFilterLoader.moveDownFilter(strFiltername) )
    {
        pModel->getDrawEngine().getFilterLoader().saveFilterOrder();    ///< save new order to file
        rcOutputArg.setParameter("filter_names",  QVariant::fromValue(rcFilterLoader.getFilterNames())  );
        rcOutputArg.setParameter("filter_status", QVariant::fromValue(rcFilterLoader.getEnableStatus()) );
        return true;
    }
    return false;
}
