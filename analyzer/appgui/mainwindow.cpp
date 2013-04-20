#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include "model/modellocator.h"
#include "commands/appfrontcontroller.h"
#include "io/analyzermsgsender.h"
#include "commandrequest.h"
#include "bitstreamversionselector.h"
#include "events/eventnames.h"
#include "io/analyzermsgsender.h"
#include "commandrespond.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),    
    ui(new Ui::MainWindow)
{

    setModualName("main_window");

    ui->setupUi(this);    
    ui->msgDockWidget->widget()->layout()->setContentsMargins(0,0,0,0);
    ui->pluginFilterListDockWidget->widget()->layout()->setContentsMargins(0,0,0,0);
    ui->sequencesListDockWidget->widget()->layout()->setContentsMargins(0,0,0,0);

    listenToEvtByName(g_strCmdEndEvent);
    ModelLocator::getInstance();    ///init filters, etc..
}

MainWindow::~MainWindow()
{
    delete ui;
}
bool MainWindow::detonate(GitlEvent cEvt )
{

    if(!cEvt.getEvtData().hasParameter("respond"))
    {
        AnalyzerMsgSender::getInstance()->msgOut(QString("MainWindow Received Command Finish Event Without \"Respond\""));
        return false;
    }

    QVariant vValue;
    cEvt.getEvtData().getParameter("respond", vValue);
    const CommandRespond& cRespond = vValue.value<CommandRespond>();
    xRefreshUIByRespond(cRespond);
    return true;
}

/// Keyboard Event
void MainWindow::keyPressEvent ( QKeyEvent * event )
{
    if(event->key() == Qt::Key_Comma)
    {
        /// Next Frame
        on_previousFrame_clicked();
    }
    else if(event->key() == Qt::Key_Period )
    {
        /// Previous Frame
        on_nextFrame_clicked();
    }

}


void MainWindow::xRefreshUIByRespond( const CommandRespond& rcRespond )
{
    QVariant vValue;
    if( rcRespond.hasParameter("picture") )
    {
        rcRespond.getParameter("picture",vValue);
        QPixmap* pPixMap = (QPixmap*)(vValue.value<void *>());
        xPresentFrameBuffer(pPixMap);
    }

    if( rcRespond.hasParameter("total_frame_num") )
    {
        rcRespond.getParameter("total_frame_num",vValue);
        int iTotalFrameNum = vValue.toInt();
        ui->totalFrameNum->setText(QString("%1").arg(iTotalFrameNum));
    }

    if( rcRespond.hasParameter("current_frame_poc") )
    {
        rcRespond.getParameter("current_frame_poc",vValue);
        int iCurrentFrameNum = vValue.toInt() + 1;
        ui->currentFrameNum->setText(QString("%1").arg(iCurrentFrameNum));
    }



}




void MainWindow::on_previousFrame_clicked()
{
    /// invoke command
    CommandRequest cRequest;

    cRequest.setParameter("command_name", "prev_frame");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(&cEvt);
}

void MainWindow::on_nextFrame_clicked()
{
    /// invoke command
    CommandRequest cRequest;

    cRequest.setParameter("command_name", "next_frame");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(&cEvt);
}

void MainWindow::on_progressBar_actionTriggered(int action)
{
    int iBarPercent = int(100*double(ui->progressBar->sliderPosition()-ui->progressBar->minimum()) /
                                       (ui->progressBar->maximum()-ui->progressBar->minimum()));

    /// invoke command
    CommandRequest cRequest;

    cRequest.setParameter("command_name", "jumpto_percent");
    cRequest.setParameter("percent", iBarPercent);

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(&cEvt);

}


void MainWindow::xPresentFrameBuffer(QPixmap *pPixmap)
{
    /// show frame
    ui->imageView->getGraphicsPixmapItem().setPixmap(*pPixmap);
}















void MainWindow::on_actionOpen_bitstream_triggered()
{
    /// select file path
    QString strFilename;
    strFilename=QFileDialog::getOpenFileName(this,
                                          tr("Open Bitstream File"),
                                          "",
                                          tr("Text Files (*.*)"));
    if(strFilename.isEmpty() || !QFileInfo(strFilename).exists() )
    {
        AnalyzerMsgSender::getInstance()->msgOut( "File not found." );
        return;
    }

    /// select HM version
    BitstreamVersionSelector cBitstreamDig(this);
    if( cBitstreamDig.exec() == QDialog::Rejected )
        return;

    /// prepare & sent event to bus
    CommandRequest cRequest;
    cRequest.setParameter("command_name", "decode_bitstream");
    cRequest.setParameter("filename", strFilename);
    cRequest.setParameter("skip_decode", false);
    cRequest.setParameter("sequence", "testing_sequence");
    cRequest.setParameter("version", cBitstreamDig.getBitstreamVersion());
    cRequest.setParameter("decoder_folder", "decoders");
    cRequest.setParameter("output_folder", "testing_sequence");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(&cEvt);



}

void MainWindow::on_actionOpen_benchmark_bitstream_triggered()
{
    QString strFilename;
    strFilename=QFileDialog::getOpenFileName(this,
                                          tr("Open Bitstream File"),
                                          "",
                                          tr("Text Files (*.*)"));
    if(strFilename.isEmpty() || !QFileInfo(strFilename).exists() )
    {
        AnalyzerMsgSender::getInstance()->msgOut( "File not found." );
        return;
    }

    //select bitstream version
    BitstreamVersionSelector cBitstreamDig(this);
    cBitstreamDig.exec();

    CommandRequest cRequest;

    cRequest.setParameter("command_name", "decode_bitstream");
    cRequest.setParameter("filename", strFilename);
    cRequest.setParameter("skip_decode", false);
    cRequest.setParameter("sequence", "benchmark_sequence");
    cRequest.setParameter("version", cBitstreamDig.getBitstreamVersion());
    cRequest.setParameter("decoder_folder", "decoders");
    cRequest.setParameter("output_folder", "benchmark_sequence");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(&cEvt);
}












//void MainWindow::on_showTesting_clicked()
//{
//    /// invoke command
//    CommandRequest cRequest;
//    CommandRespond cRespond;
//    cRequest.setParameter("command_name", "switch_sequence");
//    cRequest.setParameter("sequence", "testing_sequence");
////    if( AppFrontController::getInstance()->processRequest( cRequest, cRespond ) )
////    {
////        /// result
////        xRefreshUIByRespond(cRespond);
////    }
//}

//void MainWindow::on_showBenchmark_clicked()
//{
//    /// invoke command
//    CommandRequest cRequest;
//    CommandRespond cRespond;
//    cRequest.setParameter("command_name", "switch_sequence");
//    cRequest.setParameter("sequence", "benchmark_sequence");
////    if( AppFrontController::getInstance()->processRequest( cRequest, cRespond ) )
////    {
////        /// result
////        xRefreshUIByRespond(cRespond);
////    }
//}







void MainWindow::on_actionOpen_bitstream_info_folder_triggered()
{
    QString strFilename;
    strFilename=QFileDialog::getExistingDirectory(this,
                                          tr("Open Bitstream Sequence Info Folder"),
                                          "",
                                          QFileDialog::ShowDirsOnly);
    if(strFilename.isEmpty() || !QFileInfo(strFilename).exists() )
    {
        AnalyzerMsgSender::getInstance()->msgOut( "File not found." );
        return;
    }


    CommandRequest cRequest;
    CommandRespond cRespond;
    cRequest.setParameter("command_name", "decode_bitstream");
    cRequest.setParameter("filename", strFilename);
    cRequest.setParameter("skip_decode", true);
    cRequest.setParameter("sequence", "testing_sequence");
    cRequest.setParameter("version", -1);
    cRequest.setParameter("decoder_folder", "decoders");
    cRequest.setParameter("output_folder", strFilename);

//    if( AppFrontController::getInstance()->processRequest( cRequest, cRespond ) )
//    {
//        /// result
//        xRefreshUIByRespond(cRespond);
//        this->ui->actionOpen_benchmark_bitstream->setEnabled(true);
//        this->ui->actionOpen_benchmark_info_folder->setEnabled(true);
//    }

}

void MainWindow::on_actionOpen_benchmark_info_folder_triggered()
{
    QString strFilename;
    strFilename=QFileDialog::getExistingDirectory(this,
                                          tr("Open Benchmark Sequence Info Folder"),
                                          "",
                                          QFileDialog::ShowDirsOnly);
    if(strFilename.isEmpty() || !QFileInfo(strFilename).exists() )
    {
        AnalyzerMsgSender::getInstance()->msgOut( "File not found." );
        return;
    }


    CommandRequest cRequest;
    CommandRespond cRespond;
    cRequest.setParameter("command_name", "decode_bitstream");
    cRequest.setParameter("filename", strFilename);
    cRequest.setParameter("skip_decode", true);
    cRequest.setParameter("sequence", "benchmark_sequence");
    cRequest.setParameter("version", -1);
    cRequest.setParameter("decoder_folder", "decoders");
    cRequest.setParameter("output_folder", strFilename);

//    if( AppFrontController::getInstance()->processRequest( cRequest, cRespond ) )
//    {
//        /// result
//        xRefreshUIByRespond(cRespond);
//        this->ui->showTesting->setEnabled(true);
//        this->ui->showBenchmark->setEnabled(true);
//        this->ui->showDiff->setEnabled(true);
//        this->ui->showDifferDetail->setEnabled(true);
//    }
}


void MainWindow::on_printScreenBtn_clicked()
{

    CommandRequest cRequest;
    CommandRespond cRespond;
    cRequest.setParameter("command_name", "print_screen");

//    if( AppFrontController::getInstance()->processRequest( cRequest, cRespond ) )
//    {
//        /// result
//        xRefreshUIByRespond(cRespond);
//    }
}








void MainWindow::on_actionAbout_triggered()
{
    m_cAboutDialog.show();
}




