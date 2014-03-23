#ifndef BITPARSER_H
#define BITPARSER_H

#include <QObject>
#include <QTextStream>
#include "model/common/comsequence.h"
class BitParser : public QObject
{
    Q_OBJECT
public:
    explicit BitParser(QObject *parent = 0);
    bool parseLCUBitFile(QTextStream* pcInputStream, ComSequence* pcSequence);
    bool parseSCUBitFile(QTextStream* pcInputStream, ComSequence* pcSequence);

protected:
    bool xParseSCUBitFile(QTextStream* pcSCUBitInfoStream, ComCU* pcCU);
signals:
    
public slots:
    
};

#endif // BITPARSER_H
