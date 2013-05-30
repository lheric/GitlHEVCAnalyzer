#ifndef REFRESHSCREENCOMMAND_H
#define REFRESHSCREENCOMMAND_H

#include <QObject>
#include "gitlabstractcommand.h"

class RefreshScreenCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit RefreshScreenCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:

public slots:
    
};

#endif // REFRESHSCREENCOMMAND_H
