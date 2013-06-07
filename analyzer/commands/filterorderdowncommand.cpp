#include "filterorderdowncommand.h"

FilterOrderDownCommand::FilterOrderDownCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool FilterOrderDownCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{
    return false;
}
