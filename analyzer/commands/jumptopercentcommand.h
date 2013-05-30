#ifndef JUMPTOPERCENTCOMMAND_H
#define JUMPTOPERCENTCOMMAND_H
#include "gitlabstractcommand.h"

class JumpToPercentCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit JumpToPercentCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:

public slots:
    
};

#endif // JUMPTOPERCENTCOMMAND_H
