#ifndef OPENDECODERGENERALCOMMAND_H
#define OPENDECODERGENERALCOMMAND_H

#include "gitlabstractcommand.h"

class OpenDecoderGeneralCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenDecoderGeneralCommand(QObject *parent = 0);
    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );
signals:

public slots:

};

#endif // OPENDECODERGENERALCOMMAND_H
