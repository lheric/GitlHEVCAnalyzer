#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "gitlcommandrespond.h"
#include "gitlmodual.h"
#include "busydialog.h"
#include "aboutdialog.h"
#include "sequencelist.h"
#include "gitlview.h"
namespace Ui {
    class MainWindow;
}

class MainWindow :  public QMainWindow, public GitlView
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    void onUIUpdate(GitlUpdateUIEvt& rcEvt);
    //bool detonate( GitlEvent&  cEvt );

protected:
    virtual void keyPressEvent ( QKeyEvent * event );
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

protected:

    void xPresentFrameBuffer(QPixmap *pcPixmap);

    void xSaveSnapshot(QPixmap* pcPixmap);

private slots:

    void on_previousFrame_clicked();

    void on_nextFrame_clicked();

    void on_progressBar_actionTriggered(int action);

    void on_actionOpen_bitstream_triggered();

    //void on_actionOpen_bitstream_info_folder_triggered();



    void on_printScreenBtn_clicked();

    void on_actionAbout_triggered();


    void on_openBitstreamBtn_clicked();

    void on_actionExit_triggered();

    void on_actionCheckUpdate_triggered();

    void on_actionReloadPluginsFilters_triggered();

private:
    Ui::MainWindow *ui;


private:
    ADD_CLASS_FIELD_PRIVATE(BusyDialog, cBusyDialog)
    ADD_CLASS_FIELD_PRIVATE(AboutDialog, cAboutDialog)

};

#endif // MAINWINDOW_H
