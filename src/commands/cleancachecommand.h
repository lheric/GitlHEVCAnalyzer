#ifndef CLEANCACHECOMMAND_H
#define CLEANCACHECOMMAND_H
#include "gitlabstractcommand.h"

class CleanCacheCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit CleanCacheCommand(QObject *parent = 0);
    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);
};

#endif // CLEANCACHECOMMAND_H
