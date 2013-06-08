#include "pluginfilteritem.h"
#include "ui_pluginfilteritem.h"
#include "events/eventnames.h"
#include "gitlcommandrequest.h"

PluginFilterItem::PluginFilterItem(AbstractFilter* pcFilter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PluginFilterItem)
{
    ui->setupUi(this);
    Q_ASSERT(pcFilter);
    m_pcFilter = pcFilter;

    ui->filterNameLabel->setText(pcFilter->getName());
    ui->enableCheckBox->setEnabled(pcFilter->getEnable());
}

PluginFilterItem::~PluginFilterItem()
{
    delete ui;
}

void PluginFilterItem::on_enableCheckBox_clicked()
{
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "refresh_screen");
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);

    m_pcFilter->setEnable(ui->enableCheckBox->isChecked());
}

void PluginFilterItem::on_configBtn_clicked()
{
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "config_filter");
    cRequest.setParameter("filter", QVariant::fromValue((void*)(m_pcFilter)));
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}

void PluginFilterItem::on_upBtn_clicked()
{
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "moveup_filter");
    cRequest.setParameter("filter_name", m_pcFilter->getName());
    cRequest.setParameter("filter", QVariant::fromValue((void*)(m_pcFilter)));
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}
