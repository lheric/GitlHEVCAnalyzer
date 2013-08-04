#include "msgviewer.h"
#include "events/eventnames.h"
#include "QVariant"

MsgViewer::MsgViewer(QWidget *parent ) :
    QTextBrowser(parent)
{
    this->setLineWrapMode(QTextEdit::NoWrap);
    m_cWarningBox.setWindowTitle("Oooops!");
    m_cWarningBox.setIcon(QMessageBox::Warning);
}

void MsgViewer::onUIUpdate(GitlUpdateUIEvt &rcEvt)
{
    if( rcEvt.hasParameter("msg_detail"))
    {
        QVariant vValue = rcEvt.getParameter("msg_detail");
        QString strMsg = vValue.toString();
        QtMsgType eMsgLevel = (QtMsgType)rcEvt.getParameter("msg_level").toInt();
        this->append(strMsg);
        if( eMsgLevel >= QtCriticalMsg )
        {
            m_cWarningBox.setText(strMsg);
            m_cWarningBox.show();
        }
    }
}
