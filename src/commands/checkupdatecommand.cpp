#include "checkupdatecommand.h"
#include <QDebug>
//#include <winsparkle.h>

CheckUpdateCommand::CheckUpdateCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool CheckUpdateCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
//    win_sparkle_check_update_with_ui();
    qDebug() << "Update function is under development..";
    return true;
}
