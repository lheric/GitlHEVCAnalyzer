#ifndef PRINTSCREENCOMMAND_H
#define PRINTSCREENCOMMAND_H

#include <QObject>
#include "model/modellocator.h"
#include "gitlabstractcommand.h"

class PrintScreenCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit PrintScreenCommand();

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:
    
public slots:
    
};

#endif // PRINTSCREENCOMMAND_H
