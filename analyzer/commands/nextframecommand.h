#ifndef NEXTFRAMECOMMAND_H
#define NEXTFRAMECOMMAND_H

#include "../modellocator.h"
#include "abstractcommand.h"

class NextFrameCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit NextFrameCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:

};

#endif // NEXTFRAMECOMMAND_H
