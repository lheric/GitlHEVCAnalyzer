#include "messageviewer.h"
#include "ui_messageviewer.h"

struct MessageSwitchItem
{
    QtMsgType  eMsgType;
    QCheckBox* pcSwitch;
};


MessageViewer::MessageViewer(QWidget *parent) :
    QWidget(parent),
    m_cWarningBox(this),
    ui(new Ui::MessageViewer)
{
    ui->setupUi(this);

    m_cWarningBox.setWindowTitle("Warning");
    m_cWarningBox.setModal(false);
    m_cDefalutTextColor = ui->msgTextBrowser->palette().foreground().color();

    listenToParams(QStringList()<<"msg_detail"<<"msg_level",
                   MAKE_CALLBACK(MessageViewer::onMessageArrived));
}

MessageViewer::~MessageViewer()
{
    delete ui;
}


void MessageViewer::onMessageArrived(GitlUpdateUIEvt &rcEvt)
{
    /// <message level -- switch> table
    static MessageSwitchItem s_messageSwith[] =
    {
        {QtDebugMsg,    ui->debugCheckBox },
        {QtWarningMsg,  ui->warningCheckBox },
        {QtCriticalMsg, ui->critiCheckBox },
        {QtFatalMsg,    ui->fatalCheckBox },
        {QtMsgType(-1),    NULL}      /// end mark
    };

    QVariant vValue = rcEvt.getParameter("msg_detail");
    QString strMsg = vValue.toString();
    QtMsgType eMsgLevel = (QtMsgType)rcEvt.getParameter("msg_level").toInt();

    /// different message, different color
    if(eMsgLevel != QtDebugMsg)
        ui->msgTextBrowser->setTextColor(QColor(Qt::red));
    else
        ui->msgTextBrowser->setTextColor(m_cDefalutTextColor);


    /// check if the message type is allow
    MessageSwitchItem *pcItem = s_messageSwith;
    bool bIsAllowed = false;
    do
    {
        if(pcItem->eMsgType == eMsgLevel && pcItem->pcSwitch->isChecked())
        {
            bIsAllowed = true;
            break;
        }
        pcItem++;
    }while(pcItem->pcSwitch != NULL);


    if(bIsAllowed)
    {
        /// display message
        ui->msgTextBrowser->append(strMsg);
        /// severe message should prompt dialog
        if( eMsgLevel >= QtCriticalMsg )
        {
            m_cWarningBox.setText(strMsg);
            m_cWarningBox.show();
        }
    }

}

void MessageViewer::on_clearMsgBtn_clicked()
{
    ui->msgTextBrowser->clear();
}
