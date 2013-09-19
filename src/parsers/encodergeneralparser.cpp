#include "encodergeneralparser.h"
#include <QRegExp>

EncoderGeneralParser::EncoderGeneralParser(QObject *parent) :
    QObject(parent)
{
}


bool EncoderGeneralParser::parseFile(QTextStream* pcInputStream, ComSequence* pcSequence)
{
    Q_ASSERT( pcSequence != NULL );

    QString strOneLine;
    QRegExp cMatchTarget;


    //HM software: Encoder Version [5.2][Windows][VS 1500][32 bit]
    cMatchTarget.setPattern("HM software: Encoder Version.*");
    while( !pcInputStream->atEnd() ) {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            QString strEncoderVersion = cMatchTarget.cap(1);
            pcSequence->setEncoderVersion(strEncoderVersion);
            break;
        }
    }



    //Input          File          : F:\Learning\test_sequence\QCIF\176x144_ACTIVE_300_coastguard_qcif.yuv
    cMatchTarget.setPattern("Input *File *: .*([0-9a-zA-Z_ ]*.yuv)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            QString strSequenceName = cMatchTarget.cap(1);
            pcSequence->setFileName(strSequenceName);
            break;
        }
    }

    //Real     Format              : 176x144 30Hz
    cMatchTarget.setPattern("Real     Format              : ([0-9]+)x([0-9]+).*");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iWidth = cMatchTarget.cap(1).toInt();
            int iHeight = cMatchTarget.cap(2).toInt();
            pcSequence->setWidth(iWidth);
            pcSequence->setHeight(iHeight);
            break;
        }
    }

    //Frame index                  : 0 - 299 (300 frames)
    cMatchTarget.setPattern("Frame index.*\\(([0-9]*) frames\\)");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            int iTotalFrames = cMatchTarget.cap(1).toInt();
            pcSequence->setTotalFrames(iTotalFrames);
            break;
        }
    }


    //POC  159 TId: 0 ( B-SLICE, nQP 35 QP 35 )        520 bits [Y 30.2295 dB    U 41.1685 dB    V 43.4480 dB] [ET     5 ] [L0 158 156 152 148 ] [L1 158 156 152 148 ] [LC 158 156 152 148 ] [MD5:386454b94b02a56d02dda17af7c537c5]
    // read one frame
    ComFrame *pcFrame = NULL;
    cMatchTarget.setPattern("POC *([0-9]+).*([0-9]+) bits.*Y ([0-9.]+) dB.*\\[ET *([0-9.]+) *\\]");
    while( !pcInputStream->atEnd() )
    {

        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            pcFrame = new ComFrame(pcSequence);
            pcFrame->setPoc(cMatchTarget.cap(1).toInt());
            pcFrame->setBitrate(cMatchTarget.cap(2).toDouble());
            pcFrame->setPSNR(cMatchTarget.cap(3).toDouble());
            pcFrame->setTotalEncTime(cMatchTarget.cap(4).toDouble());
            pcSequence->getFrames().push_back(pcFrame);

            // frames finish
            if(pcFrame->getPoc() == pcSequence->getTotalFrames()-1 )
                break;
        }

    }


    //SUMMARY --------------------------------------------------------
    //Total Frames |  Bitrate    Y-PSNR    U-PSNR    V-PSNR
    //      300    a     54.3416   31.1111   41.2675   43.3390
    cMatchTarget.setPattern(" *[0-9]+ *a *([0-9.]+) *([0-9.]+) *[0-9.]+ *[0-9.]+");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            pcSequence->setBitrate(cMatchTarget.cap(1).toDouble());
            pcSequence->setPSNR(cMatchTarget.cap(2).toDouble());
            break;
        }
    }


    //Bytes written to file: 77534 (62.027 kbps)

    // Total Time:     2070.335 sec.

    cMatchTarget.setPattern(" Total Time: *([0-9.]+).*");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            pcSequence->setTotalEncTime(cMatchTarget.cap(1).toDouble());
            break;
        }
    }

    /*
    //Same CU Mode Percentage : 0.909259
    //Mean CU Depth Error : 11.0519
    cMatchTarget.setPattern("Same CU Mode Percentage : *([0-9.]+).*");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            pcSequence->setSameCUModePercent(cMatchTarget.cap(1).toDouble());
            break;
        }
    }

    cMatchTarget.setPattern("Mean CU Depth Error : *([0-9.]+).*");
    while( !pcInputStream->atEnd() )
    {
        strOneLine = pcInputStream->readLine();
        if( cMatchTarget.indexIn(strOneLine) != -1 ) {
            pcSequence->setMeanCUDepthError(cMatchTarget.cap(1).toDouble());
            break;
        }
    }
    */
    return true;


}
