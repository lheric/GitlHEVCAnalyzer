#include "pluginfilteritem.h"
#include "ui_pluginfilteritem.h"
#include "events/eventnames.h"
#include "gitlcommandrequest.h"
#include "gitlivkcmdevt.h"

PluginFilterItem::PluginFilterItem(AbstractFilter* pcFilter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PluginFilterItem)
{
    ui->setupUi(this);
    Q_ASSERT(pcFilter);
    m_pcFilter = pcFilter;

    ui->filterNameLabel->setText(pcFilter->getName());
    ui->enableCheckBox->setChecked(pcFilter->getEnable());
}

PluginFilterItem::~PluginFilterItem()
{
    delete ui;
}

void PluginFilterItem::on_enableCheckBox_clicked()
{
    GitlIvkCmdEvt cRequest("refresh_screen");
    cRequest.dispatch();
    m_pcFilter->setEnable(ui->enableCheckBox->isChecked());
}

void PluginFilterItem::on_configBtn_clicked()
{
    GitlIvkCmdEvt cRequest("config_filter");
    cRequest.dispatch();
}

void PluginFilterItem::on_upBtn_clicked()
{
    GitlIvkCmdEvt cRequest("moveup_filter");
    cRequest.setParameter("filter_name", m_pcFilter->getName());
    cRequest.setParameter("filter", QVariant::fromValue((void*)(m_pcFilter)));
    cRequest.dispatch();

}

void PluginFilterItem::on_downBtn_clicked()
{
    GitlIvkCmdEvt cRequest("movedown_filter");
    cRequest.setParameter("filter_name", m_pcFilter->getName());
    cRequest.setParameter("filter", QVariant::fromValue((void*)(m_pcFilter)));
    cRequest.dispatch();
}
