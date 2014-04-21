#ifndef FILTERMOUSEPRESSCOMMAND_H
#define FILTERMOUSEPRESSCOMMAND_H
#include "gitlabstractcommand.h"
class FilterMousePressCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit FilterMousePressCommand(QObject *parent = 0);
    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:

public slots:

};

#endif // FILTERMOUSEPRESSCOMMAND_H
