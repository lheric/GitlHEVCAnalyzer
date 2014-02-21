#include "yuv420rgbbuffer.h"
#include <QFile>
#include <QDebug>


YUV420RGBBuffer::YUV420RGBBuffer()
{

    m_iBufferWidth = 0;
    m_iBufferHeight = 0;
    m_iFrameCount = -1;
    m_puh16BitYUVBuffer = NULL;
    m_puhYUVBuffer = NULL;
    m_puhRGBBuffer = NULL;
    m_bIs16Bit = false;
}

YUV420RGBBuffer::~YUV420RGBBuffer()
{

    delete[] m_puhYUVBuffer;
    m_puhYUVBuffer = NULL;

    delete[] m_puhRGBBuffer;
    m_puhRGBBuffer = NULL;

    delete[] m_puh16BitYUVBuffer;
    m_puh16BitYUVBuffer = NULL;
}


bool YUV420RGBBuffer::openYUVFile( const QString& strYUVPath, int iWidth, int iHeight, bool bIs16Bit)
{



    /// if new size dosen't match current size, delete old one and create new one
    if( iWidth != m_iBufferWidth || iHeight != m_iBufferHeight || m_bIs16Bit != bIs16Bit)
    {
        int i16BitMulti = bIs16Bit?2:1;

        delete[] m_puhYUVBuffer;
        m_puhYUVBuffer = new uchar[((iWidth * iHeight * 3) / 2) * i16BitMulti];


        delete[] m_puhRGBBuffer;
        m_puhRGBBuffer = new uchar[iWidth * iHeight * 3];        

    }


    m_iBufferWidth = iWidth;
    m_iBufferHeight = iHeight;
    m_bIs16Bit = bIs16Bit;

    /// set YUV file reader
    if( !m_cIOYUV.openYUVFilePath(strYUVPath) )
    {
        qCritical() << "YUV Buffer Initialization Fail";
        return false;
    }


    return true;

}

QPixmap* YUV420RGBBuffer::getFrame(int iFrameCount)
{
    QPixmap* pcFramePixmap = NULL;

    if( xReadFrame(iFrameCount) )
    {
        QImage cFrameImg(m_puhRGBBuffer, m_iBufferWidth, m_iBufferHeight, QImage::Format_RGB888 );
        m_cFramePixmap = QPixmap::fromImage(cFrameImg);
        pcFramePixmap = &m_cFramePixmap;
    }
    else
    {
        qCritical() << "Read YUV File Failure.";
    }

    return pcFramePixmap;
}

bool YUV420RGBBuffer::xReadFrame(int iFrameCount)
{
    int i16BitMultiplier = ( m_bIs16Bit ? 2 : 1 );

    if( iFrameCount < 0 )
        return false;
    m_iFrameCount = iFrameCount;
    int iFrameSizeInByte = (m_iBufferWidth*m_iBufferHeight*3/2)*i16BitMultiplier;
    if( m_cIOYUV.seekTo(iFrameCount*iFrameSizeInByte) == false )
        return false;
    int iReadBytes = 0;   ///read

    iReadBytes = m_cIOYUV.readOneFrame(m_puhYUVBuffer, (uint)iFrameSizeInByte);
    if(m_bIs16Bit)
    {
        x16to8BitClip(m_puhYUVBuffer, m_puhYUVBuffer, iReadBytes/i16BitMultiplier);
    }



    if( iReadBytes != iFrameSizeInByte )
    {
        qCritical() << "Read YUV Frame Error";
        return false;
    }
    else
    {
        /// YUV to RGB conversion
        xYuv2rgb(m_puhYUVBuffer, m_puhRGBBuffer, m_iBufferWidth, m_iBufferHeight);
        return true;
    }

}

/**
  *  \brief YUV to RGB conversion
  *
  *  http://www.fourcc.org/fccyvrgb.php
  *
  *  R = Y + 1.402 (Cr-128)
  *
  *  G = Y - 0.34414 (Cb-128) - 0.71414 (Cr-128)
  *
  *  B = Y + 1.772 (Cb-128)
  *
  *
  **/
void YUV420RGBBuffer::xYuv2rgb(uchar* puhYUV, uchar* puhRGB, int iWidth, int iHeight)
{


    int uiFrameSizeInPixel = iWidth*iHeight;
    uchar* const puhY = puhYUV;
    uchar* const puhU = puhYUV + uiFrameSizeInPixel;
    uchar* const puhV = puhYUV + uiFrameSizeInPixel*5/4;

    long lYOffset, lUVOffset;
    int iY, iU, iV;
    int tempR, tempG, tempB;

    uchar* iCurRgbPixelOffset = 0;
    for(int y = 0; y < iHeight; ++y)
    {
        for(int x = 0; x < iWidth; ++x)
        {
            lYOffset  = iWidth*y+x;
            lUVOffset = iWidth/2*(y/2)+(x/2);

            iY = puhY[lYOffset];
            iU = puhU[lUVOffset];
            iV = puhV[lUVOffset];

//            tempR = iY
//                    + iV + (iV>>2) + (iV>>3) + (iV>>6)- 179;
//            tempG = iY
//                    - (iU>>2) - (iU>>4) - (iU>>5) + 44
//                    - (iV>>1) - (iV>>3) - (iV>>4) - (iV>>6) + 91;
//            tempB = iY
//                    + iU + (iU>>1) + (iU>>2) - 227;

            tempR = iY
                    + 1.402 * iV - 179;
            tempG = iY
                    - 0.34414 * iU + 44
                    - 0.71414 * iV + 91;
            tempB = iY
                    + 1.772 * iU - 227;


            tempR = VALUE_CLIP(0,255,tempR);
            tempG = VALUE_CLIP(0,255,tempG);
            tempB = VALUE_CLIP(0,255,tempB);

            iCurRgbPixelOffset = puhRGB+3*(iWidth*y+x);
            *(iCurRgbPixelOffset)     = tempR;
            *(iCurRgbPixelOffset + 1) = tempG;
            *(iCurRgbPixelOffset + 2) = tempB;


        }
    }
}


void YUV420RGBBuffer::x16to8BitClip(uchar* puh8BitYUV, const uchar* puh16BitYUV,const long lSizeInUnitCount)
{
    qint16* pi16BitPelValue = NULL;
    for(int i = 0; i < lSizeInUnitCount; i++)
    {
        pi16BitPelValue = (qint16*)(puh16BitYUV+2*i);
        puh8BitYUV[i] = VALUE_CLIP(0,255,(*pi16BitPelValue)+128);
    }
}
