#ifndef SWITCHYUVCOMMAND_H
#define SWITCHYUVCOMMAND_H
#include "gitlabstractcommand.h"
#include "model/common/comsequence.h"
class SwitchYUVCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit SwitchYUVCommand(QObject *parent = 0);
    Q_INVOKABLE virtual bool execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg);

private:
    QString xGetYUVFilenameByRole(YUVRole eRole);

signals:
    
public slots:
    
};

#endif // SWITCHYUVCOMMAND_H
