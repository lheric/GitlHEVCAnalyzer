#ifndef OPENMECOMMAND_H
#define OPENMECOMMAND_H

#include "gitlabstractcommand.h"

class OpenMECommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE OpenMECommand();

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );
};

#endif // OPENMECOMMAND_H
