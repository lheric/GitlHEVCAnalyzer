#include "abstractcommand.h"
#include "gitliomsg.h"
AbstractCommand::AbstractCommand(QObject *parent) :
    QObject(parent)
{
}
AbstractCommand::~AbstractCommand()
{

}


bool AbstractCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    GitlIOMsg::getInstance()->msgOut( "<AbstractCommand::execute> not implemented.", GITL_MSG_ERROR );
    return false;
}
