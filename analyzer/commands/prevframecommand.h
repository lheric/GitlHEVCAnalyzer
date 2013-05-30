#ifndef PREVFRAMECOMMAND_H
#define PREVFRAMECOMMAND_H

#include "model/modellocator.h"
#include "gitlabstractcommand.h"

class PrevFrameCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit PrevFrameCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:

public slots:

};

#endif // PREVFRAMECOMMAND_H
