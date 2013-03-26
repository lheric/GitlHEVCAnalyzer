#ifndef INTRAPARSER_H
#define INTRAPARSER_H

#include <QObject>
#include <QTextStream>
#include "../common/comsequence.h"

class IntraParser : public QObject
{
    Q_OBJECT
public:
    explicit IntraParser(QObject *parent = 0);
    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);
signals:

public slots:

};

#endif // INTRAPARSER_H
