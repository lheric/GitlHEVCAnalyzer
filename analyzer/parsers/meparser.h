#ifndef MEPARSER_H
#define MEPARSER_H

#include <QObject>
#include <QTextStream>
#include "../common/comsequence.h"
class MEParser : public QObject
{
    Q_OBJECT
public:
    explicit MEParser(QObject *parent = 0);
    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);


signals:

public slots:

};

#endif // MEPARSER_H
