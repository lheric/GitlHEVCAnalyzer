#ifndef SWITCHSEQUENCECOMMAND_H
#define SWITCHSEQUENCECOMMAND_H
#include "gitlabstractcommand.h"

class SwitchSequenceCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SwitchSequenceCommand(QObject *parent = 0);

    Q_INVOKABLE virtual bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:

public slots:

};

#endif // SWITCHSEQUENCECOMMAND_H
