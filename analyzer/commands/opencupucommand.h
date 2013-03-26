#ifndef OPENCUPUCOMMAND_H
#define OPENCUPUCOMMAND_H

#include "abstractcommand.h"

class OpenCUPUCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenCUPUCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );
signals:

public slots:

};

#endif // OPENCUPUCOMMAND_H
