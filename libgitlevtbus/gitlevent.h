#ifndef GITLEVENT_H
#define GITLEVENT_H
#include <QString>
#include <QObject>
#include <QMetaType>
#include "gitldef.h"
#include "gitlevtdata.h"

class GitlModual;

class GitlEvent
{
public:
    GitlEvent( const QString& strEvtName );
    GitlEvent();

    ADD_CLASS_FIELD_NOSETTER(QString, strEvtName, getEvtName)
    ADD_CLASS_FIELD_NOSETTER(GitlEvtData, cEvtData, getEvtData)
};


#endif // GITLEVENT_H
