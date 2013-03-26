#ifndef OPENENCODERGENERALCOMMAND_H
#define OPENENCODERGENERALCOMMAND_H

#include "abstractcommand.h"




/**
  * open_general filepath(no_space!) resolutionX, resolution Y
  *
  */


class OpenEncoderGeneralCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenEncoderGeneralCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:

};

#endif // OPENGENERALCOMMAND_H
