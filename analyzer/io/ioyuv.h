#ifndef IOYUV_H
#define IOYUV_H

#include <QObject>
#include <QDataStream>
#include <QFile>
#include "gitldef.h"

class IOYUV : public QObject
{
    Q_OBJECT
public:
    explicit IOYUV(QObject *parent = 0);
    ~IOYUV();

    bool setYUVFilePath(const QString &strYUVFilePath);
    bool seekTo(qint64 llOffset);
    int readOneFrame(uchar* phuFrameBuffer, uint iLenInByte);
    int writeOneFrame(uchar* phuFrameBuffer, uint iLenInByte);

    ADD_CLASS_FIELD_NOSETTER(QFile, cYUVFile, getYUVFile )
    ADD_CLASS_FIELD_NOSETTER(QDataStream, cYUVStream, getYUVStream )
signals:

public slots:

};

#endif // IOYUV_H
