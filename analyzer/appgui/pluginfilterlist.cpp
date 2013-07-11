#include "pluginfilterlist.h"
#include "events/eventnames.h"
#include "drawengine/abstractfilter.h"
#include "io/analyzermsgsender.h"
#include "appgui/pluginfilteritem.h"
#include "gitlcommandrespond.h"
#include "gitlcommandrequest.h"
PluginFilterList::PluginFilterList(QWidget *parent) :
    QListWidget(parent)
{
    setModualName("plugin_filter_list");
    subscribeToEvtByName(g_strCmdEndEvent);

//    setDragDropMode(QAbstractItemView::InternalMove);
//    setDropIndicatorShown(false);

    // load filters command
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "reload_filter");
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}

bool PluginFilterList::detonate( GitlEvent& cEvt )
{

    if(!cEvt.hasParameter("respond"))
    {
        qWarning() << QString("PluginFilterList Received Command Finish Event Without \"Respond\"");
        return false;
    }
    const GitlCommandRespond& cRespond = cEvt.getParameter("respond").value<GitlCommandRespond>();

    xRefreshListByRespond(cRespond);

//    if( strEvtName == g_strPluginFilterLoaded )     ///< filter loaded
//    {

//        QVariant vValue = cEvt.getParameter("filter");
//        AbstractFilter* pFilter = (AbstractFilter*)vValue.value<void*>();
//        /// make checkable & init status
//        QListWidgetItem* pcItem = new QListWidgetItem();
//        this->addItem(pcItem);
//        PluginFilterItem *pItemWidget = new PluginFilterItem(pFilter);
//        pcItem->setSizeHint(pItemWidget->sizeHint());
//        setItemWidget(pcItem, pItemWidget);
//    }
//    else if( strEvtName == g_strPluginFilterUnloaded )  ///< filter unloaded
//    {
//        QVariant vValue = cEvt.getParameter("filter_name");

//        /// find and remove
//        QString strFilterName = vValue.toString();
//        QList<QListWidgetItem*> apcFound = this->findItems(strFilterName,Qt::MatchFixedString|Qt::MatchCaseSensitive);

//        for(int i = 0; i < apcFound.size(); i++)
//        {
//            QListWidgetItem* pcItem = apcFound.at(i);
//            delete this->takeItem(row(pcItem));
//        }

//    }
//    /*else if (strEvtName == g_strCmdEndEvent )   ///< filter order changed, refresh list
//    {
//        this->clear();

//        QVariant vValue = cEvt.getParameter("respond").value<GitlCommandRespond>().getParameter("filters");
//        QVector<AbstractFilter*> *papcFilter = (QVector<AbstractFilter*>*)vValue.value<void*>();

//        for(int i = 0; i < papcFilter->size(); i++)
//        {
//            QListWidgetItem* pcItem = new QListWidgetItem();
//            this->addItem(pcItem);
//            AbstractFilter* pFilter = papcFilter->at(i);
//            PluginFilterItem *pItemWidget = new PluginFilterItem(pFilter);
//            pcItem->setSizeHint(pItemWidget->sizeHint());
//            setItemWidget(pcItem, pItemWidget);
//        }

//    }*/
    return true;
}
void PluginFilterList::xRefreshListByRespond(const GitlCommandRespond& rcRespond)
{
    if(rcRespond.hasParameter("filters"))
    {
        this->clear();
        QVariant vValue = rcRespond.getParameter("filters");
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
