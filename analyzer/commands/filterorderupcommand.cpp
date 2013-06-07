#include "filterorderupcommand.h"

FilterOrderUpCommand::FilterOrderUpCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool FilterOrderUpCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{
    return false;
}
