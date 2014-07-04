#ifndef SAVEFILTERORDERCOMMAND_H
#define SAVEFILTERORDERCOMMAND_H

#include "gitlabstractcommand.h"

class SaveFilterOrderCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SaveFilterOrderCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:

public slots:

};

#endif // SAVEFILTERORDERCOMMAND_H
