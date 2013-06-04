#include "gitlfrontcontroller.h"
#include "gitlabstractcommand.h"

#include <QDebug>

SINGLETON_PATTERN_IMPLIMENT(GitlFrontController)

GitlFrontController::GitlFrontController()
{
}


bool GitlFrontController::addCommand(GitlCommandFormat cCommandFormat)
{
    m_cCommandTable.push_back(cCommandFormat);
    qDebug() << QString("%1 Register Success!").arg(cCommandFormat.pMetaObject->className());
    return true;
}
bool GitlFrontController::addCommand(const QString cCommandName, const QMetaObject* pMetaObject)
{
    GitlCommandFormat cCommandFormat;
    cCommandFormat.cCommandName = cCommandName;
    cCommandFormat.pMetaObject  = pMetaObject;
    m_cCommandTable.push_back(cCommandFormat);
    qDebug() << QString("%1 Register Success!").arg(cCommandFormat.pMetaObject->className());
    return true;
}

void GitlFrontController::removeAllCommand()
{
    m_cCommandTable.clear();
}


bool GitlFrontController::processRequest( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{

    // find command by name
    QVariant vValue;
    if( rcRequest.hasParameter("command_name") )
    {
        vValue = rcRequest.getParameter("command_name");

    }
    else
    {
        qWarning() << QString("No command name in request!");
        return false;
    }
    QString strCommandName = vValue.toString();
    rcRespond.setParameter("command_name", strCommandName);

    for(int i = 0; i < m_cCommandTable.size(); i++) {
        // command name matched
        if( m_cCommandTable.at(i).cCommandName == strCommandName )
        {

            //TODO BUG RAII Required for exception
            //create  command
            const QMetaObject* pMetaObj = m_cCommandTable.at(i).pMetaObject;
            QObject* pObj = pMetaObj->newInstance();
            GitlAbstractCommand* pCmd = (GitlAbstractCommand *)pObj;
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
    qWarning() << QString("No matched command name found. %1").arg(strCommandName);
    return false;
}



