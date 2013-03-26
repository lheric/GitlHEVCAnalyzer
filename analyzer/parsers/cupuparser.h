#ifndef CUPUPARSER_H
#define CUPUPARSER_H

#include <QObject>
#include <QTextStream>
#include "../common/comsequence.h"
//


class CUPUParser : public QObject
{
    Q_OBJECT
public:
    explicit CUPUParser(QObject *parent = 0);
    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);
    bool xReadInCUMode(QTextStream* pcCUInfoStream, QVector<int>& piCUMode);
signals:

public slots:

};

#endif // CUPUPARSER_H
