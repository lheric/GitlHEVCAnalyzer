#ifndef SPSPARSER_H
#define SPSPARSER_H

#include <QObject>
#include <QTextStream>
#include "../common/comsequence.h"

class SpsParser : public QObject
{
    Q_OBJECT
public:
    explicit SpsParser(QObject *parent = 0);
    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);
signals:

public slots:

};

#endif // SPSPARSER_H
