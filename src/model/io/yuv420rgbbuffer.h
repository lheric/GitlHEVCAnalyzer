#ifndef YUV420RGBBUFFER_H
#define YUV420RGBBUFFER_H

#include <QObject>
#include <QPixmap>
#include <QFile>
#include <QMap>
#include "ioyuv.h"
#include "gitldef.h"




class YUV420RGBBuffer : public QObject
{
    Q_OBJECT
public:
    explicit YUV420RGBBuffer();
    ~YUV420RGBBuffer();

    bool openYUVFile( const QString& strYUVPath, int iWidth, int iHeight, bool bIs16Bit = false );
    QPixmap* getFrame(int iPoc);


    ADD_CLASS_FIELD(int, iBufferWidth, getBufferWidth, setBufferWidth)
    ADD_CLASS_FIELD(int, iBufferHeight, getBufferHeight, setBufferHeight)
    ADD_CLASS_FIELD(int, iPoc, getPoc, setPoc)
    ADD_CLASS_FIELD(bool, bIs16Bit, getIs16Bit, setIs16Bit)


    ADD_CLASS_FIELD_PRIVATE(QPixmap, cFramePixmap)
    ADD_CLASS_FIELD_PRIVATE(uchar*, puh16BitYUVBuffer)
    ADD_CLASS_FIELD_PRIVATE(uchar*,  puhYUVBuffer)
    ADD_CLASS_FIELD_PRIVATE(uchar*,  puhRGBBuffer)
    ADD_CLASS_FIELD_PRIVATE(IOYUV,   cIOYUV)



protected:
    bool xReadFrame(int iPoc);
    void xYuv2rgb(uchar* puhYUV, uchar* puhRGB, int iWidth, int iHeight);
    void x16to8BitClip(uchar* puh8BitYUV, const uchar* puh16BitYUV, const long lSizeInUnitCount);

signals:

public slots:

};

#endif // YUVBUFFER_H
