#include "bitstreamparser.h"
#include "io/analyzermsgsender.h"
#include "exceptions/decodernotfoundexception.h"
#include "exceptions/bitstreamnotfoundexception.h"
#include "gitlupdateuievt.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>

BitstreamParser::BitstreamParser(QObject *parent)
{
    connect(&m_cDecoderProcess, SIGNAL(readyRead()), this, SLOT(displayDecoderOutput()));
}

bool BitstreamParser::parseFile(QString strDecoderFolder,
                                int iEncoderVersion,
                                QString strBitstreamFilePath,
                                QString strOutputPath,
                                ComSequence* pcSequence)
{
    QDir cCurDir = QDir::current();
    /// check if decoder exist
    QString strDecoderFilePath = strDecoderFolder + QString("/HM_%1.exe").arg(iEncoderVersion);
    if(!cCurDir.exists(strDecoderFilePath))
    {
        throw DecoderNotFoundException();
    }
    /// check if bitstream file exist or
    if( (!cCurDir.exists(strBitstreamFilePath)) ||
        (!cCurDir.isAbsolutePath(strBitstreamFilePath)) )
    {
        throw BitstreamNotFoundException();
    }

    /// check if output folder exist
    if( !cCurDir.exists(strOutputPath) )
    {
        cCurDir.mkpath(strOutputPath);
    }

    m_cDecoderProcess.setWorkingDirectory(strOutputPath);
    QString strStandardOutputFile = strOutputPath+"/decoder_general.txt";
    m_cStdOutputFile.setFileName(strStandardOutputFile);
    m_cStdOutputFile.open(QIODevice::WriteOnly);
    QString strDecoderCmd = strDecoderFolder + QString("\"/HM_%1.exe\" -b %2 -o decoder_yuv.yuv").arg(iEncoderVersion).arg(strBitstreamFilePath);

    m_cDecoderProcess.start(strDecoderCmd);
    m_cDecoderProcess.waitForFinished(-1);
    m_cStdOutputFile.close();

    pcSequence->setFileName(strBitstreamFilePath);
    pcSequence->setDecodingFolder(strOutputPath);

    return (m_cDecoderProcess.exitCode() == 0);
}

void BitstreamParser::displayDecoderOutput()
{



    while( m_cDecoderProcess.canReadLine() )
    {
        // write to file
        QString strLine = m_cDecoderProcess.readLine();
        m_cStdOutputFile.write(strLine.toStdString().c_str());


        // display progress text as event
        QRegExp cMatchTarget;
        cMatchTarget.setPattern("POC *([0-9]+)");

        if( cMatchTarget.indexIn(strLine) != -1 )
        {
            GitlUpdateUIEvt evt;
            int iPoc = cMatchTarget.cap(1).toInt();
            QString strText = QString("%1 Frame Decoded").arg(iPoc);
            evt.setParameter("decoding_progress", strText);
            dispatchEvt(evt);
        }
    }

}
