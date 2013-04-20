#include "ioyuv.h"
#include "../common/comanalyzerdef.h"
#include "../io/analyzermsgsender.h"


IOYUV::IOYUV(QObject *parent) :
    QObject(parent)
{
}

IOYUV::~IOYUV()
{
    m_cYUVFile.close();
}

bool IOYUV::setYUVFilePath(const QString& strYUVFilePath)
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
            AnalyzerMsgSender::getInstance()->msgOut("YUV File Open Error", GITL_MSG_ERROR);

        }
    }
    else
    {
        AnalyzerMsgSender::getInstance()->msgOut("YUV File Not Found", GITL_MSG_ERROR);
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




