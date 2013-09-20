#include "ioyuv.h"
#include <QDebug>
IOYUV::IOYUV(QObject *parent) :
    QObject(parent)
{

}

IOYUV::~IOYUV()
{
    m_cYUVFile.close();
}

bool IOYUV::openYUVFilePath(const QString& strYUVFilePath)
{
    m_cYUVFile.close();
    m_cYUVStream.setDevice(NULL);

    m_cYUVFile.setFileName(strYUVFilePath);
    if( m_cYUVFile.exists() )
    {
        if( m_cYUVFile.open(QIODevice::ReadOnly) )   ///< open
        {
            m_cYUVStream.setDevice(&m_cYUVFile);
            return true;
        }
        else
        {
            qWarning() << "YUV File Open Error";

        }
    }
    else
    {
        qWarning() << "YUV File Not Found";
    }


    return false;
}

bool IOYUV::seekTo(qint64 llOffset)
{
    return (m_cYUVFile.isOpen() && m_cYUVFile.seek(llOffset));
}

int IOYUV::readOneFrame(uchar* phuFrameBuffer, uint iLenInByte )
{
    return m_cYUVStream.readRawData((char*)phuFrameBuffer, iLenInByte);
}

int IOYUV::writeOneFrame(uchar* phuFrameBuffer, uint iLenInByte )
{
    return m_cYUVStream.writeRawData((char*)phuFrameBuffer, iLenInByte);
}




