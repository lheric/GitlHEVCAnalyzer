#ifndef MVPARSER_H
#define MVPARSER_H

#include <QObject>
#include <QTextStream>
#include "../common/comsequence.h"

class MVParser : public QObject
{
    Q_OBJECT
public:
    explicit MVParser(QObject *parent = 0);
    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);
signals:

public slots:

};

#endif // MVPARSER_H
