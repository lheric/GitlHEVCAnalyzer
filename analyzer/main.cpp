#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QLayout>
#include <iostream>
#include "gitlevent.h"

//
#include "model/modellocator.h"
#include "appgui/mainwindow.h"

#include "commands/appfrontcontroller.h"
#include "exceptions/nosequencefoundexception.h"
#include "io/analyzermsgsender.h"
using namespace std;




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

int main(int argc, char *argv[])
{
    /// Register Commands
    AppFrontController::getInstance();


    /// Stylesheet
    QApplication cApp(argc, argv);
    if( !xReadStylesheet())
        AnalyzerMsgSender::getInstance()->msgOut("Stylesheet Loading Failed");  ///stylesheet

    /// Show main win
    MainWindow cMainWin;
    cMainWin.centralWidget()->layout()->setContentsMargins(0,0,0,0);    /// layout hack (maxmize central widget)
    cMainWin.show();
    ModelLocator::getInstance();
    cApp.exec();

    return EXIT_SUCCESS;

}
