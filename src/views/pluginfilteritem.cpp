#include "pluginfilteritem.h"
#include "ui_pluginfilteritem.h"
#include "gitlivkcmdevt.h"
#include <QDebug>

PluginFilterItem::PluginFilterItem(const QString& strFilterName, const bool bEnable, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PluginFilterItem)
{
    ui->setupUi(this);
    this->m_strFilterName = strFilterName;
    ui->filterNameLabel->setText(strFilterName);
    ui->enableCheckBox->setChecked(bEnable);
}

PluginFilterItem::~PluginFilterItem()
{
    delete ui;
}

void PluginFilterItem::on_enableCheckBox_clicked()
{
    /// enable/disable filter
    GitlIvkCmdEvt cRequest("switch_filter");
    cRequest.setParameter("filter_name",   m_strFilterName);
    cRequest.setParameter("filter_enable", ui->enableCheckBox->isChecked());
    cRequest.dispatch();
}

void PluginFilterItem::on_configBtn_clicked()
{
    GitlIvkCmdEvt cRequest("config_filter");
    cRequest.setParameter("filter_name", m_strFilterName);
    cRequest.dispatch();
}
