#ifndef SWITCHYUVCOMMAND_H
#define SWITCHYUVCOMMAND_H
#include "gitlabstractcommand.h"

class SwitchYUVCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SwitchYUVCommand(QObject *parent = 0);
    Q_INVOKABLE virtual bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:
    
public slots:
    
};

#endif // SWITCHYUVCOMMAND_H
