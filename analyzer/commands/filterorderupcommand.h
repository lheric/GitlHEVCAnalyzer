#ifndef FILTERORDERUPCOMMAND_H
#define FILTERORDERUPCOMMAND_H
#include "gitlabstractcommand.h"

class FilterOrderUpCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit FilterOrderUpCommand(QObject *parent = 0);

    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:
    
public slots:
    
};

#endif // FILTERORDERUPCOMMAND_H
