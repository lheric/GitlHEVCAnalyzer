#ifndef CONFIGFILTERCOMMAND_H
#define CONFIGFILTERCOMMAND_H
#include "gitlabstractcommand.h"

class ConfigFilterCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit ConfigFilterCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:
    
public slots:
    
};

#endif // CONFIGFILTERCOMMAND_H
