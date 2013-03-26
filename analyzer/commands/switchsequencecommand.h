#ifndef SWITCHSEQUENCECOMMAND_H
#define SWITCHSEQUENCECOMMAND_H
#include "abstractcommand.h"

class SwitchSequenceCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SwitchSequenceCommand(QObject *parent = 0);

    Q_INVOKABLE virtual bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:

};

#endif // SWITCHSEQUENCECOMMAND_H
