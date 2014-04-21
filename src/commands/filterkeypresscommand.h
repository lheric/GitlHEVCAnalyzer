#ifndef FILTERKEYPRESSCOMMAND_H
#define FILTERKEYPRESSCOMMAND_H
#include "gitlabstractcommand.h"

class FilterKeyPressCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit FilterKeyPressCommand(QObject *parent = 0);
    Q_INVOKABLE bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);
signals:

public slots:


};

#endif // FILTERKEYPRESSCOMMAND_H
