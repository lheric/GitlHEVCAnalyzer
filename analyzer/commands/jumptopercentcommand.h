#ifndef JUMPTOPERCENTCOMMAND_H
#define JUMPTOPERCENTCOMMAND_H
#include "abstractcommand.h"

class JumpToPercentCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit JumpToPercentCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:
    
};

#endif // JUMPTOPERCENTCOMMAND_H
