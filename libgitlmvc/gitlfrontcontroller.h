#ifndef GITLFRONTCONTROLLER_H
#define GITLFRONTCONTROLLER_H


#include <QMetaObject>
#include <QVector>

#include "gitldef.h"
#include "gitlcommandrequest.h"
#include "gitlcommandrespond.h"


typedef struct GitlCommandFormat_s
{
    QString cCommandFormat;
    const QMetaObject* pMetaObject;
}GitlCommandFormat;


class GitlFrontController
{
public:
    explicit GitlFrontController();
    bool processRequest( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond );
    bool addCommand(GitlCommandFormat cComandFormat);

protected:

    /// command table
    ADD_CLASS_FIELD(QVector<GitlCommandFormat>, cCommandTable, getCommandTable, setCommandTable)

};

#endif // GITLFRONTCONTROLLER_H
