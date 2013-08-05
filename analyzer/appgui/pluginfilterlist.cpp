#include "pluginfilterlist.h"
#include "drawengine/abstractfilter.h"
#include "io/analyzermsgsender.h"
#include "appgui/pluginfilteritem.h"
#include "gitlcommandrespond.h"
#include "gitlcommandrequest.h"
#include "gitlivkcmdevt.h"

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
    if(rcEvt.hasParameter("filters"))
    {
        this->clear();
        QVariant vValue = rcEvt.getParameter("filters");
        QVector<AbstractFilter*> *papcFilter = (QVector<AbstractFilter*>*)vValue.value<void*>();

        for(int i = 0; i < papcFilter->size(); i++)
        {
            QListWidgetItem* pcItem = new QListWidgetItem();
            this->addItem(pcItem);
            AbstractFilter* pFilter = papcFilter->at(i);
            PluginFilterItem *pItemWidget = new PluginFilterItem(pFilter);
            pcItem->setSizeHint(pItemWidget->sizeHint());
            setItemWidget(pcItem, pItemWidget);
        }
    }
}

