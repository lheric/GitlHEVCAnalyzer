#ifndef SWITCHFILTERCOMMAND_H
#define SWITCHFILTERCOMMAND_H
#include "gitlabstractcommand.h"

class SwitchFilterCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SwitchFilterCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);
signals:
    
public slots:
    
};

#endif // SWITCHFILTERCOMMAND_H
