#ifndef CHECKUPDATECOMMAND_H
#define CHECKUPDATECOMMAND_H
#include "gitlabstractcommand.h"

class CheckUpdateCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit CheckUpdateCommand(QObject *parent = 0);
    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );
    
signals:
    
public slots:
    
};

#endif // CHECKUPDATECOMMAND_H
