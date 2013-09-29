#include "pluginfilterlist.h"
#include "model/drawengine/abstractfilter.h"
#include "views/pluginfilteritem.h"
#include "gitlivkcmdevt.h"
#include <QVector>
#include <QScrollBar>
#include <QDebug>
PluginFilterList::PluginFilterList(QWidget *parent) :
    QListWidget(parent)
{
    setModualName("plugin_filter_list");

    // load filters command
    GitlIvkCmdEvt cRequest("reload_filter");
    cRequest.dispatch();
}

void PluginFilterList::onUIUpdate(GitlUpdateUIEvt &rcEvt)
{

    // rebuild the filter list
    if(rcEvt.hasParameter("filter_names") && rcEvt.hasParameter("filter_status") )
    {
        int iVPos = verticalScrollBar()->value();
        int iHPos = horizontalScrollBar()->value();

        this->clear();
        QStringList cFilterNames = rcEvt.getParameter("filter_names").toStringList();
        QVector<bool> cFilterEnableStatus = rcEvt.getParameter("filter_status").value< QVector<bool> >();

        for(int i = 0; i < cFilterNames.size(); i++)
        {
            QListWidgetItem* pcItem = new QListWidgetItem();
            this->addItem(pcItem);
            PluginFilterItem *pItemWidget = new PluginFilterItem(cFilterNames[i], cFilterEnableStatus[i]);
            pcItem->setSizeHint(pItemWidget->sizeHint());
            setItemWidget(pcItem, pItemWidget);
        }

        verticalScrollBar()->setValue(iVPos);
        horizontalScrollBar()->setValue(iHPos);
    }

}

