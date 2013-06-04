#include "gitlabstractcommand.h"

#include <QDebug>
GitlAbstractCommand::GitlAbstractCommand(QObject *parent) :
    QObject(parent)
{
}
GitlAbstractCommand::~GitlAbstractCommand()
{

}


bool GitlAbstractCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{
    qCritical() << "<AbstractCommand::execute> not implemented!";
    return false;
}
