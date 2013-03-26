#ifndef OPENYUVCOMMAND_H
#define OPENYUVCOMMAND_H

#include "abstractcommand.h"




/**
  * open_yuv filepath(no_space!) resolutionX, resolution Y
  *
  */


class OpenYUVCommand : public AbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenYUVCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:

};

#endif // OPENYUVCOMMAND_H
