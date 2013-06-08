#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QLayout>
#include <QDir>
#include <iostream>
#include <QTextCodec>
#include <winsparkle.h>
#include "gitlevent.h"
#include "model/modellocator.h"
#include "appgui/mainwindow.h"
#include "commands/appfrontcontroller.h"
#include "exceptions/nosequencefoundexception.h"
#include "io/analyzermsgsender.h"
#include "common/comrom.h"
using namespace std;
//static bool xSetLibPath()
//{
//    QStringList paths = QCoreApplication::instance()->libraryPaths();
//    QDir cAppDir = QDir(QCoreApplication::instance()->applicationDirPath());

//    if( !cAppDir.cd("runtime") )
//    {
//        qCritical() << "Runtime libraries missing..Some function may not work!";
//        return false;
//    }
//    paths << cAppDir.absolutePath();
//    QCoreApplication::instance()->setLibraryPaths(paths);
//    return true;
//}


static bool xReadStylesheet()
{
    QFile cFile(":/UI/UI/style_sheet.qss");
    if( !cFile.open(QFile::ReadOnly) )
        return false;

    QString strStyleSheet = QLatin1String(cFile.readAll());
    qApp->setStyleSheet(strStyleSheet);
    cFile.close();
    return true;
}


static void xMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &strMsg)
{
    QByteArray localMsg = strMsg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "[Info]: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "[Warn]: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[Crit]: %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "[Fata]: %s\n", localMsg.constData());        
    }
    fflush(stdout);
    fflush(stderr);

    if(type == QtFatalMsg)
        abort();

    AnalyzerMsgSender::getInstance()->msgOut(strMsg);
}

void xCheckUpdate()
{
    win_sparkle_set_appcast_url("http://winsparkle.org/example/appcast.xml");
    win_sparkle_init();
}

void xCleanUpdate()
{
    win_sparkle_cleanup();
}

int main(int argc, char *argv[])
{
//    /// set run-time dynamic linked library (dll) path
//    xSetLibPath();


    /// intall message handler
    qInstallMessageHandler(xMessageOutput);

    /// check update
    xCheckUpdate();

    /// Register Commands
    AppFrontController::getInstance();

    /// Stylesheet
    QApplication cApp(argc, argv);
    if( !xReadStylesheet())
        qWarning() << "Stylesheet Loading Failed";  ///stylesheet

    /// Show main win
    MainWindow cMainWin;
    cMainWin.centralWidget()->layout()->setContentsMargins(0,0,0,0);    /// layout hack (maxmize central widget)
    cMainWin.show();
    ModelLocator::getInstance();
    cApp.exec();

    /// clean update
    xCleanUpdate();

    return EXIT_SUCCESS;

}
