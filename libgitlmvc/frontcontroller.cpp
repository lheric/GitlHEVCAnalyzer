#include "frontcontroller.h"
#include "abstractcommand.h"
#include "gitliomsg.h"



FrontController::FrontController()
{
}


bool FrontController::addCommand(CommandFormat cComandFormat)
{
    m_cCommandTable.push_back(cComandFormat);
    GitlIOMsg::getInstance()->msgOut(QString("%1 Register Success!").arg(cComandFormat.pMetaObject->className()), GITL_MSG_DEBUG);
    return true;
}

bool FrontController::processRequest( CommandRequest& rcRequest, CommandRespond& rcRespond )
{

    // find command by name
    QVariant vValue;
    if( !rcRequest.getParameter("command_name", vValue) )
    {
        GitlIOMsg::getInstance()->msgOut(QString("No command name in request!"), GITL_MSG_ERROR);
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
                GitlIOMsg::getInstance()->msgOut(QString("%1 Execution Failed!").arg(pMetaObj->className()), GITL_MSG_ERROR);
                delete pCmd;
                return false;
            }else
            {
                GitlIOMsg::getInstance()->msgOut(QString("%1 Execution Success!").arg(pMetaObj->className()), GITL_MSG_DEBUG);
                delete pCmd;
                return true;
            }
        }
    }
    GitlIOMsg::getInstance()->msgOut(QString("No matched command name found. %1").arg(strCommandName));
    return false;
}



