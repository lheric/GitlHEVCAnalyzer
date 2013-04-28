#include "frontcontroller.h"
#include "abstractcommand.h"

#include <QDebug>


FrontController::FrontController()
{
}


bool FrontController::addCommand(CommandFormat cComandFormat)
{
    m_cCommandTable.push_back(cComandFormat);
    qDebug() << QString("%1 Register Success!").arg(cComandFormat.pMetaObject->className());
    return true;
}

bool FrontController::processRequest( CommandRequest& rcRequest, CommandRespond& rcRespond )
{

    // find command by name
    QVariant vValue;
    if( !rcRequest.getParameter("command_name", vValue) )
    {
        qWarning() << QString("No command name in request!");
        return false;
    }
    QString strCommandName = vValue.toString();
    rcRespond.setParameter("command_name", strCommandName);

    for(int i = 0; i < m_cCommandTable.size(); i++) {
        // command name matched
        if( m_cCommandTable.at(i).cCommandFormat == strCommandName )
        {

            //TODO BUG RAII Required for exception
            //create  command
            const QMetaObject* pMetaObj = m_cCommandTable.at(i).pMetaObject;
            QObject* pObj = pMetaObj->newInstance();
            AbstractCommand* pCmd = (AbstractCommand *)pObj;
            //execute command
            if( pCmd->execute(rcRequest, rcRespond) == false )
            {
                qDebug() << QString("%1 Execution Failed!").arg(pMetaObj->className());
                delete pCmd;
                return false;
            }else
            {
                qDebug() << QString("%1 Execution Success!").arg(pMetaObj->className());
                delete pCmd;
                return true;
            }
        }
    }
    qWarning() << QString("No matched command name found. %1");
    return false;
}



