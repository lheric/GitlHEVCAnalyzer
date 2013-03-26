#ifndef ABSTRACTCOMMAND_H
#define ABSTRACTCOMMAND_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include "gitldef.h"

#include "commandrequest.h"
#include "commandrespond.h"



class AbstractCommand : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit AbstractCommand(QObject *parent = 0);
    virtual ~AbstractCommand();
    /// implement this function in subclass
    Q_INVOKABLE virtual bool execute( CommandRequest& rcRequest, CommandRespond& rcRespond );

signals:

public slots:

};

#endif // ABSTRACTCOMMAND_H
