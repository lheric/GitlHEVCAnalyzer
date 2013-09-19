#include "filterorderupcommand.h"
#include "model/modellocator.h"
#include <QDebug>
FilterOrderUpCommand::FilterOrderUpCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool FilterOrderUpCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{

    ModelLocator* pModel = ModelLocator::getInstance();
    FilterLoader& rcFilterLoader = pModel->getDrawEngine().getFilterLoader();

    QString strFiltername = rcInputArg.getParameter("filter_name").toString();
    if( rcFilterLoader.moveUpFilter(strFiltername) )
    {
        pModel->getDrawEngine().getFilterLoader().saveFilterOrder();    ///< save new order to file
        rcOutputArg.setParameter("filter_names",  QVariant::fromValue(rcFilterLoader.getFilterNames())  );
        rcOutputArg.setParameter("filter_status", QVariant::fromValue(rcFilterLoader.getEnableStatus()) );
        return true;
    }
    return false;
}
