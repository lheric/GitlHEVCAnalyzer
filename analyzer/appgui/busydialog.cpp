#include "busydialog.h"
#include "ui_busydialog.h"
#include "io/analyzermsgsender.h"
#include "events/eventnames.h"
#include "gitlcommandrequest.h"
#include "gitlcommandrespond.h"

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


bool BusyDialog::detonate(GitlEvent& cEvt)
{
    QString& strEvtName = cEvt.getEvtName();
    if(strEvtName == g_strCmdStartEvent)
    {
        GitlCommandRequest cRequest;

        QVariant vValue = cEvt.getParameter("request");
        cRequest = vValue.value<GitlCommandRequest>();
        vValue = cRequest.getParameter("command_name");
        const QString& strCmdName = vValue.toString();

        if( strCmdName == "decode_bitstream")
            this->show();
    }
    else if(strEvtName == g_strCmdEndEvent)
    {
        GitlCommandRespond cRespond;
        QVariant vValue = cEvt.getParameter("respond");
        cRespond = vValue.value<GitlCommandRespond>();
        vValue = cRespond.getParameter("command_name");
        const QString& strCmdName = vValue.toString();

        if( strCmdName == "decode_bitstream")
            this->hide();
    }
    else if(strEvtName == g_strCmdInfoEvent)
    {
        QVariant cVariant = cEvt.getParameter("message");
        this->ui->busyDynamicText->setText(cVariant.toString());
    }
    return true;

}
