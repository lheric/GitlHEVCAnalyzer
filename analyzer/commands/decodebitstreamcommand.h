#ifndef DECODEBITSTREAMCOMMAND_H
#define DECODEBITSTREAMCOMMAND_H
#include "abstractcommand.h"
#include "gitlmodual.h"
class DecodeBitstreamCommand : public AbstractCommand, GitlModual
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit DecodeBitstreamCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );
signals:

public slots:

};

#endif // OPENBITSTREAMCOMMAND_H
