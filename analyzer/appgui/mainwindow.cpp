#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include "model/modellocator.h"
#include "commands/appfrontcontroller.h"
#include "io/analyzermsgsender.h"
#include "gitlcommandrequest.h"
#include "bitstreamversionselector.h"
#include "events/eventnames.h"
#include "io/analyzermsgsender.h"
#include "gitlcommandrespond.h"
#include "common/comrom.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),    
    ui(new Ui::MainWindow)
{

    setModualName("main_window");

    ui->setupUi(this);    
    ui->msgDockWidget->widget()->layout()->setContentsMargins(0,0,0,0);
    ui->pluginFilterListDockWidget->widget()->layout()->setContentsMargins(0,0,0,0);
    ui->sequencesListDockWidget->widget()->layout()->setContentsMargins(0,0,0,0);

    subscribeToEvtByName(g_strCmdEndEvent);
    ModelLocator::getInstance();    ///init filters, etc..
}

MainWindow::~MainWindow()
{
    delete ui;
}
bool MainWindow::detonate(GitlEvent cEvt )
{

    if(!cEvt.hasParameter("respond"))
    {
        qWarning() << QString("MainWindow Received Command Finish Event Without \"Respond\"");
        return false;
    }

    QVariant vValue = cEvt.getParameter("respond");
    const GitlCommandRespond& cRespond = vValue.value<GitlCommandRespond>();
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


void MainWindow::xRefreshUIByRespond( const GitlCommandRespond& rcRespond )
{    
    QVariant vValue;

    /// draw current frame to screen
    if( rcRespond.hasParameter("picture") )
    {
        vValue = rcRespond.getParameter("picture");
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
            vValue = rcRespond.getParameter("total_frame_num");
            iTotalFrameNum = vValue.toInt();
            ui->totalFrameNum->setText(QString("%1").arg(iTotalFrameNum));
        }

        /// total frame number
        if( rcRespond.hasParameter("current_frame_poc") )
        {
            vValue = rcRespond.getParameter("current_frame_poc");
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
        vValue = rcRespond.getParameter("snapshot");
        QPixmap* pcPixMap = (QPixmap*)(vValue.value<void *>());
        xSaveSnapshot(pcPixMap);
    }


}




void MainWindow::on_previousFrame_clicked()
{
    /// invoke command
    GitlCommandRequest cRequest;

    cRequest.setParameter("command_name", "prev_frame");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}

void MainWindow::on_nextFrame_clicked()
{
    /// invoke command
    GitlCommandRequest cRequest;

    cRequest.setParameter("command_name", "next_frame");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}

void MainWindow::on_progressBar_actionTriggered(int action)
{
    int iBarPercent = int(100*double(ui->progressBar->sliderPosition()-ui->progressBar->minimum()) /
                                       (ui->progressBar->maximum()-ui->progressBar->minimum()));

    /// invoke command
    GitlCommandRequest cRequest;

    cRequest.setParameter("command_name", "jumpto_percent");
    cRequest.setParameter("percent", iBarPercent);

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);

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
    QString strLastPath = g_cAppSetting.value("snapshot_saving_path",".").toString();
    strFilename=QFileDialog::getSaveFileName(this,
                                          tr("Save Snapshot"),
                                          strLastPath,
                                          tr("Images (*.png)"));
    if(!strFilename.isEmpty())
    {
        g_cAppSetting.setValue("snapshot_saving_path",strFilename);
        if( pcPixmap->save(strFilename) )
            qDebug() << QString("Snapshot Has Been Saved to %1 !").arg(strFilename);
        else
            qWarning() <<"Snapshot Saving Failed!";
    }


}


void MainWindow::on_actionOpen_bitstream_triggered()
{
    /// select file path
    QString strFilename;
    QString strLastPath = g_cAppSetting.value("open_bitstream_path",".").toString();
    strFilename=QFileDialog::getOpenFileName(this,
                                          tr("Open Bitstream File"),
                                          strLastPath,
                                          tr("All Files (*.*)"));

    if(!strFilename.isEmpty())
        g_cAppSetting.setValue("open_bitstream_path",strFilename);

    if(strFilename.isEmpty() || !QFileInfo(strFilename).exists() )
    {
        qWarning() << "File not found.";
        return;
    }

    /// select HM version
    BitstreamVersionSelector cBitstreamDig(this);
    if( cBitstreamDig.exec() == QDialog::Rejected )
        return;

    /// prepare & sent event to bus
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "decode_bitstream");
    cRequest.setParameter("filename", strFilename);
    cRequest.setParameter("skip_decode", false);
    cRequest.setParameter("version", cBitstreamDig.getBitstreamVersion());
    cRequest.setParameter("decoder_folder", "decoders");
    cRequest.setParameter("output_folder", "decoded_sequences");

    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}


void MainWindow::on_printScreenBtn_clicked()
{
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "print_screen");
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}



void MainWindow::on_actionAbout_triggered()
{
    m_cAboutDialog.show();
}




void MainWindow::on_openBitstreamBtn_clicked()
{
    on_actionOpen_bitstream_triggered();
}


/*
void MainWindow::on_actionOpen_bitstream_info_folder_triggered()
{
    QString strFilename;
    strFilename=QFileDialog::getExistingDirectory(this,
                                          tr("Open Bitstream Sequence Info Folder"),
                                          ".",
                                          QFileDialog::ShowDirsOnly);
    if(strFilename.isEmpty() || !QFileInfo(strFilename).exists() )
    {
        qWarning() << "File not found.";
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
*/
void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

void MainWindow::on_actionCheckUpdate_triggered()
{
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "check_update");
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}

void MainWindow::on_actionReloadPluginsFilters_triggered()
{
    GitlCommandRequest cRequest;
    cRequest.setParameter("command_name", "reload_filter");
    GitlEvent cEvt( g_strCmdSentEvent  );
    cEvt.setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}
