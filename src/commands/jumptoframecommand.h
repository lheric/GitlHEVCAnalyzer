#ifndef JUMPTOFRAMECOMMAND_H
#define JUMPTOFRAMECOMMAND_H

#include "model/modellocator.h"
#include "gitlabstractcommand.h"

class JumpToFrameCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit JumpToFrameCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:

public slots:

};

#endif // JUMPTOFRAMECOMMAND_H
