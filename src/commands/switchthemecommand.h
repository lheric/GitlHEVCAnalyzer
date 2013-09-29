#ifndef SWITCHTHEMECOMMAND_H
#define SWITCHTHEMECOMMAND_H

#include "gitlabstractcommand.h"

class SwitchThemeCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SwitchThemeCommand(QObject *parent = 0);

    Q_INVOKABLE virtual bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

signals:
    
public slots:
    
};

#endif // SWITCHTHEMECOMMAND_H
