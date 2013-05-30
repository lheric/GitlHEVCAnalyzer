#ifndef ZOOMFRAMECOMMAND_H
#define ZOOMFRAMECOMMAND_H
#include "gitlabstractcommand.h"

class ZoomFrameCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit ZoomFrameCommand(QObject *parent = 0);

    Q_INVOKABLE virtual bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:
    
public slots:
    
};

#endif // ZOOMFRAMECOMMAND_H
