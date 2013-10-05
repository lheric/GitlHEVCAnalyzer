#include "checkupdatecommand.h"
#include <QDebug>

CheckUpdateCommand::CheckUpdateCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool CheckUpdateCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    qDebug() << "Update function is under development..";
    return true;
}
