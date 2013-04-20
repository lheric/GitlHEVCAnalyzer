#include "yuv420rgbbuffer.h"
#include "io/analyzermsgsender.h"
#include <QFile>

YUV420RGBBuffer::YUV420RGBBuffer()
{

    m_iBufferWidth = 0;
    m_iBufferHeight = 0;
    m_iPoc = -1;
    m_puhYUVBuffer = NULL;
    m_puhRGBBuffer = NULL;
}

YUV420RGBBuffer::~YUV420RGBBuffer()
{

    delete[] m_puhYUVBuffer;
    m_puhYUVBuffer = NULL;

    delete[] m_puhRGBBuffer;
    m_puhRGBBuffer = NULL;
}


bool YUV420RGBBuffer::setYUVFile( const QString& strYUVPath, int iWidth, int iHeight )
{
    /// if new size dosen't match current size, delete old one and create new one
    if( iWidth != m_iBufferWidth || iHeight != m_iBufferHeight )
    {


        delete[] m_puhYUVBuffer;
        m_puhYUVBuffer = new uchar[(iWidth * iHeight * 3) / 2];


        delete[] m_puhRGBBuffer;
        m_puhRGBBuffer = new uchar[iWidth * iHeight * 3];



    }


    m_iBufferWidth = iWidth;
    m_iBufferHeight = iHeight;
    /// set YUV file reader
    if( !m_cIOYUV.setYUVFilePath(strYUVPath) )
    {
        AnalyzerMsgSender::getInstance()->msgOut("YUV Buffer Initialization Fail", GITL_MSG_ERROR);
        return false;
    }


    return true;

}

QPixmap* YUV420RGBBuffer::getFrame(int iPoc)
{
    QPixmap* pcFramePixmap = NULL;

    if( xReadFrame(iPoc) )
    {
        QImage cFrameImg(m_puhRGBBuffer, m_iBufferWidth, m_iBufferHeight, QImage::Format_RGB888 );
        m_cFramePixmap = QPixmap::fromImage(cFrameImg);
        pcFramePixmap = &m_cFramePixmap;
    }
    else
    {
        AnalyzerMsgSender::getInstance()->msgOut("Read YUV File Failure.", GITL_MSG_ERROR);
    }

    return pcFramePixmap;
}

bool YUV420RGBBuffer::xReadFrame(int iPoc)
{
    if( iPoc < 0 )
        return false;
    m_iPoc = iPoc;
    int iFrameSizeInByte = m_iBufferWidth*m_iBufferHeight*3/2;
    if( m_cIOYUV.seekTo(iPoc*iFrameSizeInByte) == false )
        return false;
    int iReadBytes = m_cIOYUV.readOneFrame(m_puhYUVBuffer, (uint)iFrameSizeInByte);   ///read
    if( iReadBytes != iFrameSizeInByte )
    {
        AnalyzerMsgSender::getInstance()->msgOut("Read YUV Frame Error", GITL_MSG_ERROR);
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

            tempR = iY
                    + iV + (iV>>2) + (iV>>3) + (iV>>6)- 179;
            tempG = iY
                    - (iU>>2) - (iU>>4) - (iU>>5) + 44
                    - (iV>>1) - (iV>>3) - (iV>>4) - (iV>>6) + 91;
            tempB = iY
                    + iU + (iU>>1) + (iU>>2) - 227;


            VALUE_CLIP(0,255,tempR);
            VALUE_CLIP(0,255,tempG);
            VALUE_CLIP(0,255,tempB);

            iCurRgbPixelOffset = puhRGB+3*(iWidth*y+x);
            *(iCurRgbPixelOffset)     = tempR;
            *(iCurRgbPixelOffset + 1) = tempG;
            *(iCurRgbPixelOffset + 2) = tempB;


        }
    }
}
