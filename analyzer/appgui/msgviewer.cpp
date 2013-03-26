#include "msgviewer.h"
#include "events/eventnames.h"
#include "QVariant"
MsgViewer::MsgViewer(QWidget *parent ) :
    QTextBrowser(parent)
{
    listenToEvtByName(g_strStatusMsgEvent);
}

bool MsgViewer::detonate( GitlEvent cEvt )
{
    QVariant vValue;
    cEvt.getEvtData().getParameter("msg_detail", vValue);
    QString strMsg = vValue.toString();
    this->append(strMsg);
    return true;
}
