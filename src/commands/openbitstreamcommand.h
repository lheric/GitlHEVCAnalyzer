#ifndef DECODEBITSTREAMCOMMAND_H
#define DECODEBITSTREAMCOMMAND_H
#include "gitlabstractcommand.h"
#include "gitlmodual.h"
class OpenBitstreamCommand : public GitlAbstractCommand, GitlModual
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit OpenBitstreamCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);
signals:

public slots:

};

#endif // OPENBITSTREAMCOMMAND_H
