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

    /// draw current frame to screen
    if( rcRespond.hasParameter("picture") )
    {
        rcRespond.getParameter("picture",vValue);
        QPixmap* pcPixMap = (QPixmap*)(vValue.value<void *>());
        xPresentFrameBuffer(pcPixMap);
    }


    /// change frame number, total frame number & slide bar position in GUI.
    if( rcRespond.hasParameter("total_frame_num") || rcRespond.hasParameter("current_frame_poc") )
    {
        static int iTotalFrameNum = -1;
        static int iCurrentFrameNum = -1;

        /// frame number
        if( rcRespond.hasParameter("total_frame_num") )
        {
            rcRespond.getParameter("total_frame_num",vValue);
            iTotalFrameNum = vValue.toInt();
            ui->totalFrameNum->setText(QString("%1").arg(iTotalFrameNum));
        }

        /// total frame number
        if( rcRespond.hasParameter("current_frame_poc") )
        {
            rcRespond.getParameter("current_frame_poc",vValue);
            iCurrentFrameNum = vValue.toInt() + 1;
            ui->currentFrameNum->setText(QString("%1").arg(iCurrentFrameNum));
        }

        /// slide bar position
        int iMin = ui->progressBar->minimum();
        int iMax = ui->progressBar->maximum();
        int iPos = (iCurrentFrameNum-1)*(iMax-iMin)/(iTotalFrameNum-1);
        ui->progressBar->setValue(iPos);

    }


    if( rcRespond.hasParameter("snapshot") )
    {
        rcRespond.getParameter("snapshot",vValue);
        QPixmap* pcPixMap = (QPixmap*)(vValue.value<void *>());
        xSaveSnapshot(pcPixMap);
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


void MainWindow::xPresentFrameBuffer(QPixmap *pcPixmap)
{
    /// show frame
    ui->imageView->getGraphicsPixmapItem().setPixmap(*pcPixmap);
}

void MainWindow::xSaveSnapshot(QPixmap *pcPixmap)
{
    ///
    QString strFilename;
    strFilename=QFileDialog::getSaveFileName(this,
                                          tr("Save Snapshot"),
                                          ".",
                                          tr("Images (*.png)"));

    pcPixmap->save(strFilename);
}


void MainWindow::on_actionOpen_bitstream_triggered()
{
    /// select file path
    QString strFilename;
    strFilename=QFileDialog::getOpenFileName(this,
                                          tr("Open Bitstream File"),
                                          ".",
                                          tr("All Files (*.*)"));

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
    cRequest.setParameter("version", cBitstreamDig.getBitstreamVersion());
    cRequest.setParameter("decoder_folder", "decoders");
    cRequest.setParameter("output_folder", "decoded_sequences");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(&cEvt);
}


void MainWindow::on_printScreenBtn_clicked()
{
    CommandRequest cRequest;
    cRequest.setParameter("command_name", "print_screen");
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(&cEvt);
}








void MainWindow::on_actionAbout_triggered()
{
    m_cAboutDialog.show();
}




void MainWindow::on_openBitstreamBtn_clicked()
{
    on_actionOpen_bitstream_triggered();
}



void MainWindow::on_actionOpen_bitstream_info_folder_triggered()
{
    QString strFilename;
    strFilename=QFileDialog::getExistingDirectory(this,
                                          tr("Open Bitstream Sequence Info Folder"),
                                          ".",
                                          QFileDialog::ShowDirsOnly);
    if(strFilename.isEmpty() || !QFileInfo(strFilename).exists() )
    {
        AnalyzerMsgSender::getInstance()->msgOut( "File not found." );
        return;
    }

    CommandRequest cRequest;
    cRequest.setParameter("command_name", "decode_bitstream");
    cRequest.setParameter("filename", strFilename);
    cRequest.setParameter("skip_decode", true);
    cRequest.setParameter("sequence", "testing_sequence");
    cRequest.setParameter("version", -1);
    cRequest.setParameter("decoder_folder", "decoders");
    cRequest.setParameter("output_folder", strFilename);
}

void MainWindow::on_actionExit_triggered()
{
    exit(0);
}
