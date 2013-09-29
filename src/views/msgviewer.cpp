#include "msgviewer.h"
#include "QVariant"
#include <QPalette>
MsgViewer::MsgViewer(QWidget *parent ) :
    QTextBrowser(parent)
{
    this->setLineWrapMode(QTextEdit::NoWrap);
    m_cWarningBox.setWindowTitle("Oooops!");
    m_cWarningBox.setIcon(QMessageBox::Warning);
    m_cDefalutTextColor = palette().foreground().color();
}

void MsgViewer::onUIUpdate(GitlUpdateUIEvt &rcEvt)
{
    if( rcEvt.hasParameter("msg_detail"))
    {
        QVariant vValue = rcEvt.getParameter("msg_detail");
        QString strMsg = vValue.toString();
        QtMsgType eMsgLevel = (QtMsgType)rcEvt.getParameter("msg_level").toInt();
        if(eMsgLevel >= QtWarningMsg)
            this->setTextColor(QColor(Qt::red));
        else
            this->setTextColor(m_cDefalutTextColor);
        this->append(strMsg);

        if( eMsgLevel >= QtCriticalMsg )
        {
            m_cWarningBox.setText(strMsg);
            m_cWarningBox.show();
        }
    }
}
