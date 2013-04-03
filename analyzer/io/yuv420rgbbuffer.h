#ifndef YUV420RGBBUFFER_H
#define YUV420RGBBUFFER_H

#include <QObject>
#include <QPixmap>
#include <QFile>
#include "ioyuv.h"
#include "gitldef.h"

class YUV420RGBBuffer : public QObject
{
    Q_OBJECT
public:
    explicit YUV420RGBBuffer();
    ~YUV420RGBBuffer();

    bool setYUVFile( const QString& strYUVPath, int iWidth, int iHeight );
    QPixmap* getFrame(int iPoc);


    ADD_CLASS_FIELD(int, iBufferWidth, getBufferWidth, setBufferWidth)
    ADD_CLASS_FIELD(int, iBufferHeight, getBufferHeight, setBufferHeight)
    ADD_CLASS_FIELD(int, iPoc, getPoc, setPoc)


    ADD_CLASS_FIELD_PRIVATE(QPixmap, cFramePixmap)
    ADD_CLASS_FIELD_PRIVATE(uchar*,  puhYUVBuffer)
    ADD_CLASS_FIELD_PRIVATE(uchar*,  puhRGBBuffer)
    ADD_CLASS_FIELD_PRIVATE(IOYUV,   cIOYUV)

protected:
    bool xReadFrame(int iPoc);
    void xYuv2rgb(uchar* puhYUV, uchar* puhRGB, int iWidth, int iHeight);

signals:

public slots:

};

#endif // YUVBUFFER_H
