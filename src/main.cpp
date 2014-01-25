#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QLayout>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <QTextCodec>
#include "gitlevent.h"
#include "model/modellocator.h"
#include "views/mainwindow.h"
#include "commands/appfrontcontroller.h"
#include "exceptions/nosequencefoundexception.h"
#include "model/common/comrom.h"
using namespace std;


static void xMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &strMsg)
{
    QByteArray localMsg = strMsg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "[Debug]: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "[Warn]: %s\n",  localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[Crit]: %s\n",  localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "[Fata]: %s\n",  localMsg.constData());
    }
    fflush(stdout);
    fflush(stderr);

    GitlUpdateUIEvt cEvt;
    cEvt.setParameter("msg_detail", strMsg);
    cEvt.setParameter("msg_level",(int)type);
    cEvt.dispatch();

    if(type == QtFatalMsg)
        abort();


}

#include "views/gitlcolorpicker.h"
int main(int argc, char *argv[])
{
    /// intall message handler
    qInstallMessageHandler(xMessageOutput);

    /// Register Commands
    AppFrontController::getInstance();

    QApplication cApp(argc, argv);

    /// Show main win
    MainWindow cMainWin;
    cMainWin.centralWidget()->layout()
            ->setContentsMargins(0,0,0,0);          /// layout hack (maxmize central widget)
    cMainWin.show();
    ModelLocator::getInstance();                    /// init model
    cApp.exec();


    return EXIT_SUCCESS;

}
