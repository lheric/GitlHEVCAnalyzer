#ifndef GITLABSTRACTCOMMAND_H
#define GITLABSTRACTCOMMAND_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include "gitldef.h"

#include "gitlcommandrequest.h"
#include "gitlcommandrespond.h"



class GitlAbstractCommand : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit GitlAbstractCommand(QObject *parent = 0);
    virtual ~GitlAbstractCommand();
    /// implement this function in subclass
    Q_INVOKABLE virtual bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );

signals:

public slots:

};

#endif // GITLABSTRACTCOMMAND_H
