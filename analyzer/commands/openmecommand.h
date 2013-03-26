#ifndef OPENMECOMMAND_H
#define OPENMECOMMAND_H

#include "abstractcommand.h"

class OpenMECommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE OpenMECommand();

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );
};

#endif // OPENMECOMMAND_H
