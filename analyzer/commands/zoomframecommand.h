#ifndef ZOOMFRAMECOMMAND_H
#define ZOOMFRAMECOMMAND_H
#include "abstractcommand.h"

class ZoomFrameCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit ZoomFrameCommand(QObject *parent = 0);

    Q_INVOKABLE virtual bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:
    
public slots:
    
};

#endif // ZOOMFRAMECOMMAND_H
