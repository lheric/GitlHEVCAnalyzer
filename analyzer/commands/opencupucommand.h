#ifndef OPENCUPUCOMMAND_H
#define OPENCUPUCOMMAND_H

#include "gitlabstractcommand.h"

class OpenCUPUCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenCUPUCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );
signals:

public slots:

};

#endif // OPENCUPUCOMMAND_H
