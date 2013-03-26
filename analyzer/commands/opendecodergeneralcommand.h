#ifndef OPENDECODERGENERALCOMMAND_H
#define OPENDECODERGENERALCOMMAND_H

#include "abstractcommand.h"

class OpenDecoderGeneralCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenDecoderGeneralCommand(QObject *parent = 0);
    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );
signals:

public slots:

};

#endif // OPENDECODERGENERALCOMMAND_H
