#ifndef TILEPARSER_H
#define TILEPARSER_H

#include <QObject>
#include <QTextStream>
#include"model/common/comsequence.h"

class TileParser : public QObject
{
    Q_OBJECT
public:
    explicit TileParser(QObject *parent = 0);

    bool parseFile(QTextStream* pcInputStream, ComSequence* pcSequence);
protected:
    bool xReadTile(QTextStream* pcTileInfoStream, ComFrame * pcFrame);

signals:

public slots:

};

#endif // TILEPARSER_H
