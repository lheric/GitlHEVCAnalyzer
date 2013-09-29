#ifndef MODIFYPREFERENCESCOMMAND_H
#define MODIFYPREFERENCESCOMMAND_H
#include "gitlabstractcommand.h"

class ModifyPreferencesCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit ModifyPreferencesCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:
    
public slots:
    
};

#endif // MODIFYPREFERENCESCOMMAND_H
