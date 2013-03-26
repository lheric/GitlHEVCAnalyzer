#ifndef JUMPTOFRAMECOMMAND_H
#define JUMPTOFRAMECOMMAND_H

#include "../modellocator.h"
#include "abstractcommand.h"

class JumpToFrameCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit JumpToFrameCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:

};

#endif // JUMPTOFRAMECOMMAND_H
