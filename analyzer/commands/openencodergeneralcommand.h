#ifndef OPENENCODERGENERALCOMMAND_H
#define OPENENCODERGENERALCOMMAND_H

#include "gitlabstractcommand.h"




/**
  * open_general filepath(no_space!) resolutionX, resolution Y
  *
  */


class OpenEncoderGeneralCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenEncoderGeneralCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:

public slots:

};

#endif // OPENGENERALCOMMAND_H
