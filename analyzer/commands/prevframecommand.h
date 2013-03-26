#ifndef PREVFRAMECOMMAND_H
#define PREVFRAMECOMMAND_H

#include "../modellocator.h"
#include "abstractcommand.h"

class PrevFrameCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit PrevFrameCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:

};

#endif // PREVFRAMECOMMAND_H
