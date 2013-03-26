#ifndef PREDPARSER_H
#define PREDPARSER_H

#include <QObject>
#include <QTextStream>
#include "../common/comsequence.h"

class PredParser : public QObject
{
    Q_OBJECT
public:
    explicit PredParser(QObject *parent = 0);
    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);
signals:

public slots:

};

#endif // PREDPARSER_H
