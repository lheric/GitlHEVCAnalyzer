#include "msgviewer.h"
#include "events/eventnames.h"
#include "QVariant"

MsgViewer::MsgViewer(QWidget *parent ) :
    QTextBrowser(parent)
{
    subscribeToEvtByName(g_strStatusMsgEvent);
    this->setLineWrapMode(QTextEdit::NoWrap);
    m_cWarningBox.setWindowTitle("Oooops!");
    m_cWarningBox.setIcon(QMessageBox::Warning);
}

bool MsgViewer::detonate( GitlEvent& cEvt )
{
    QVariant vValue = cEvt.getParameter("msg_detail");
    QString strMsg = vValue.toString();
    QtMsgType eMsgLevel = (QtMsgType)cEvt.getParameter("msg_level").toInt();
    this->append(strMsg);
    if( eMsgLevel >= QtCriticalMsg )
    {
        m_cWarningBox.setText(strMsg);
        m_cWarningBox.show();
    }
    return true;
}
