#ifndef CONFIGFILTERCOMMAND_H
#define CONFIGFILTERCOMMAND_H
#include "abstractcommand.h"

class ConfigFilterCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit ConfigFilterCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:
    
public slots:
    
};

#endif // CONFIGFILTERCOMMAND_H
