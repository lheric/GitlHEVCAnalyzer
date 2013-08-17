#ifndef ENCODERGENERALPARSER_H
#define ENCODERGENERALPARSER_H

#include <QObject>
#include <QTextStream>
#include "model/common/comsequence.h"

class EncoderGeneralParser : public QObject
{
    Q_OBJECT
public:
    explicit EncoderGeneralParser(QObject *parent = 0);
    bool parseFile(QTextStream* cInputStream, ComSequence* pcSequence);

signals:

public slots:

};

#endif // GENERALPARSER_H
