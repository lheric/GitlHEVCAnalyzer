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
    QVector<AbstractFilter*>& racFilters = pModel->getDrawEngine().getFilterLoader().getFilters();

    QString strFiltername = rcInputArg.getParameter("filter_name").toString();
    for(int i = 0; i < racFilters.size(); i++)
    {
        if(racFilters.at(i)->getName() == strFiltername)
        {
            if(i == 0)
            {
                qWarning() << "Can't move up the first filter!";
                return false;
            }
            else
            {
                AbstractFilter* pcTemp = racFilters.at(i-1);
                racFilters.replace(i-1, racFilters.at(i));
                racFilters.replace(i, pcTemp);
                pModel->getDrawEngine().getFilterLoader().saveFilterOrder();    ///< save new order to file
                rcOutputArg.setParameter("filters", QVariant::fromValue((void*)(&pModel->getDrawEngine().getFilterLoader().getFilters())));
                return true;
            }
        }
    }

    qWarning() << QString("Can't find the filter named %1!").arg(strFiltername);
    return false;
}
