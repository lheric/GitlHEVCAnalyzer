#ifndef FRONTCONTROLLER_H
#define FRONTCONTROLLER_H


#include <QMetaObject>
#include <QVector>

#include "gitldef.h"
#include "commandrequest.h"
#include "commandrespond.h"


typedef struct CommandFormat_s
{
    QString cCommandFormat;
    const QMetaObject* pMetaObject;
}CommandFormat;


class FrontController
{
public:
    explicit FrontController();
    bool processRequest( CommandRequest& rcRequest, CommandRespond& rcRespond );
    bool addCommand(CommandFormat cComandFormat);

protected:

    ///< command table
    ADD_CLASS_FIELD(QVector<CommandFormat>, cCommandTable, getCommandTable, setCommandTable)

};

#endif // FRONTCONTROLLER_H
