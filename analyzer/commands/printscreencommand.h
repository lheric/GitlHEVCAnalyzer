#ifndef PRINTSCREENCOMMAND_H
#define PRINTSCREENCOMMAND_H

#include <QObject>
#include "../modellocator.h"
#include "abstractcommand.h"

class PrintScreenCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit PrintScreenCommand();

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:
    
public slots:
    
};

#endif // PRINTSCREENCOMMAND_H
