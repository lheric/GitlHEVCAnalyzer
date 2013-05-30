#include "busydialog.h"
#include "ui_busydialog.h"
#include "io/analyzermsgsender.h"
#include "events/eventnames.h"
#include "commandrequest.h"
#include "commandrespond.h"

BusyDialog::BusyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BusyDialog)
{

    ui->setupUi(this);

    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint);

    setModualName("busy_dialog");
    subscribeToEvtByName(g_strCmdStartEvent);
    subscribeToEvtByName(g_strCmdInfoEvent);
    subscribeToEvtByName(g_strCmdEndEvent);
}

BusyDialog::~BusyDialog()
{
    delete ui;
}

void BusyDialog::setHintText(QString& str)
{
    this->ui->busyDynamicText->setText(str);
}


bool BusyDialog::detonate(GitlEvent cEvt)
{
    QString& strEvtName = cEvt.getEvtName();
    if(strEvtName == g_strCmdStartEvent)
    {
        CommandRequest cRequest;

        QVariant vValue = cEvt.getEvtData().getParameter("request");
        cRequest = vValue.value<CommandRequest>();
        cRequest.getParameter("command_name", vValue);
        const QString& strCmdName = vValue.toString();

        if( strCmdName == "decode_bitstream")
            this->show();
    }
    else if(strEvtName == g_strCmdEndEvent)
    {
        CommandRespond cRespond;
        QVariant vValue = cEvt.getEvtData().getParameter("respond");
        cRespond = vValue.value<CommandRespond>();
        cRespond.getParameter("command_name", vValue);
        const QString& strCmdName = vValue.toString();

        if( strCmdName == "decode_bitstream")
            this->hide();
    }
    else if(strEvtName == g_strCmdInfoEvent)
    {
        QVariant cVariant = cEvt.getEvtData().getParameter("message");
        this->ui->busyDynamicText->setText(cVariant.toString());
    }
    return true;

}
