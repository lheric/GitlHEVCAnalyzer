#ifndef OPENYUVCOMMAND_H
#define OPENYUVCOMMAND_H

#include "gitlabstractcommand.h"




/**
  * open_yuv filepath(no_space!) resolutionX, resolution Y
  *
  */


class OpenYUVCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenYUVCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:

public slots:

};

#endif // OPENYUVCOMMAND_H
