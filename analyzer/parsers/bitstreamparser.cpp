#include "bitstreamparser.h"
#include "io/analyzermsgsender.h"
#include "events/eventnames.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>
BitstreamParser::BitstreamParser(QObject *parent)
{
    connect(&m_cDecoderProcess, SIGNAL(readyRead()), this, SLOT(displayDecoderOutput()));
}

bool BitstreamParser::parseFile(QString strDecoderFolder,
                                int iEncoderVersion,
                                QString strBitstreamPath,
                                QString strTempOutputPath,
                                ComSequence* pcSequence)
{

    pcSequence->setFileName(strBitstreamPath);
    pcSequence->setDecodingFolder(strTempOutputPath);


    QDir cOutDir = QDir::current();
    if(!cOutDir.exists(strTempOutputPath))
    {
        cOutDir.mkdir(strTempOutputPath);
    }
    m_cDecoderProcess.setWorkingDirectory(strTempOutputPath);
    QString strStandardOutputFile = strTempOutputPath+"/decoder_general.txt";
    m_cStdOutputFile.setFileName(strStandardOutputFile);
    m_cStdOutputFile.open(QIODevice::WriteOnly);
    QString strDecoderCmd = strDecoderFolder + QString("/HM_%1.exe -b %2 -o decoder_yuv.yuv").arg(iEncoderVersion).arg(strBitstreamPath);
    m_cDecoderProcess.start(strDecoderCmd);
    m_cDecoderProcess.waitForFinished(-1);
    m_cStdOutputFile.close();

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
            GitlEvent evt( g_strCmdInfoEvent );
            int iPoc = cMatchTarget.cap(1).toInt();
            QString strText = QString("%1 Frame Decoded").arg(iPoc);
            evt.getEvtData().setParameter("message", strText);

            dispatchEvt(&evt);

        }
    }

}
