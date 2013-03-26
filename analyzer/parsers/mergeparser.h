#ifndef MERGEPARSER_H
#define MERGEPARSER_H

#include <QObject>
#include <QTextStream>
#include "../common/comsequence.h"

class MergeParser : public QObject
{
    Q_OBJECT
public:
    explicit MergeParser(QObject *parent = 0);
    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);
signals:

public slots:

};

#endif // MERGEPARSER_H
