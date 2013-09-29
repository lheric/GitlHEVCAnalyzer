#ifndef QUERYPREFERENCESCOMMAND_H
#define QUERYPREFERENCESCOMMAND_H

#include "gitlabstractcommand.h"

class QueryPreferencesCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QueryPreferencesCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:

public slots:

};

#endif // QUERYPREFERENCESCOMMAND_H
