#include "checkupdatecommand.h"
#include <winsparkle.h>

CheckUpdateCommand::CheckUpdateCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool CheckUpdateCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{
    win_sparkle_check_update_with_ui();
    return true;
}
