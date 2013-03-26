#ifndef REFRESHSCREENCOMMAND_H
#define REFRESHSCREENCOMMAND_H

#include <QObject>
#include "abstractcommand.h"

class RefreshScreenCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit RefreshScreenCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:
    
};

#endif // REFRESHSCREENCOMMAND_H
