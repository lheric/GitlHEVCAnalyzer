#include "abstractcommand.h"

#include <QDebug>
AbstractCommand::AbstractCommand(QObject *parent) :
    QObject(parent)
{
}
AbstractCommand::~AbstractCommand()
{

}


bool AbstractCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    qCritical() << "<AbstractCommand::execute> not implemented!";
    return false;
}
