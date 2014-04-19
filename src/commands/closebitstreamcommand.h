#ifndef CLOSEBITSTREAMCOMMAND_H
#define CLOSEBITSTREAMCOMMAND_H
#include "gitlabstractcommand.h"

class CloseBitstreamCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit CloseBitstreamCommand(QObject *parent = 0);
    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:

public slots:

};

#endif // CLOSEBITSTREAMCOMMAND_H
