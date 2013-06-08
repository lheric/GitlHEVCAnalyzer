#include "msgviewer.h"
#include "events/eventnames.h"
#include "QVariant"
MsgViewer::MsgViewer(QWidget *parent ) :
    QTextBrowser(parent)
{
    subscribeToEvtByName(g_strStatusMsgEvent);
}

bool MsgViewer::detonate( GitlEvent cEvt )
{
    QVariant vValue = cEvt.getParameter("msg_detail");
    QString strMsg = vValue.toString();
    this->append(strMsg);
    return true;
}
