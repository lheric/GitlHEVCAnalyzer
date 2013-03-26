#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QLayout>
#include "gitlevent.h"

//
#include "modellocator.h"
#include "common/comdef.h"
#include "appgui/mainwindow.h"

#include "commands/appfrontcontroller.h"
#include "exceptions/nosequencefoundexception.h"
#include "io/analyzermsgsender.h"
using namespace std;





/*int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("HM_Analyzer_DB.s3db");
    if (!db.open())
        qDebug() << db.lastError();
    else {
        qDebug() << db.tables();
        db.close();
    }
}*/
//class QMyApplication: public QApplication
//{
//public:
//    QMyApplication(int &argc, char **argv):
//        QApplication(argc, argv)

//    {

//    }

//    bool QMyApplication::notify(QObject* receiver, QEvent* event)
//    {

//        try {
//            return QApplication::notify(receiver, event);
//        }
//        catch( const NoSequenceFoundException& rExp )
//        {
//            AnalyzerMsgSender::getInstance()->msgOut("No Video Sequence Found...", GITL_MSG_ERROR);
//        }
//        catch( const QException& rExp )
//        {
//            AnalyzerMsgSender::getInstance()->msgOut("Unknown Error Happened...", GITL_MSG_ERROR);
//        }
//        catch(...)
//        {
//            AnalyzerMsgSender::getInstance()->msgOut("Unknown Error Happened...", GITL_MSG_ERROR);
//        }

//        return false;
//    }
//};



int main(int argc, char *argv[])
{
    //

    AppFrontController::getInstance();


    //
#if ENABLE_GUI

    QApplication cApp(argc, argv);
    MainWindow cMainWin;
    cMainWin.centralWidget()->layout()->setContentsMargins(0,0,0,0);
    cMainWin.show();
    ModelLocator::getInstance();
    cApp.exec();


#endif

    return EXIT_SUCCESS;
/*
    //init
    unsigned int* piTemp = g_auiZscanToRaster;
    initZscanToRaster( g_iMaxCUDepth+1, 1, 0, piTemp);
    initRasterToZscan( g_iMaxCUSize, g_iMaxCUSize, g_iMaxCUDepth+1 );

    // args
    QtArgCmdLine cCmd( argc, argv );
    QtArg cInputFileArg( QtArgIface::NamesList() << "general", "Input File(The Original Output of HM Software)", true, true );
    QtArg cOutputFileArg( QtArgIface::FlagsList() << QLatin1Char('o'), "Output File(The Output HM Software)", false, true );
    QtArg cYUVSequencePathArg( QtArgIface::NamesList() << "yuvpath", "YUV Test Sequence Path", false, true );
    cCmd.addArg(cInputFileArg);
    cCmd.addArg(cOutputFileArg);
    cCmd.addArg(cYUVSequencePathArg);

    QString strInputFile;
    QString strOutFile;
    QString strYUVSequencePath;
    try {
        cCmd.parse();
        strInputFile = cInputFileArg.value().toString();
        strOutFile = cOutputFileArg.value().toString();
        strYUVSequencePath = cYUVSequencePathArg.value().toString();

        ModelLocator::getInstance()->setInputFile(strInputFile);
        ModelLocator::getInstance()->setYUVSequencePath(strYUVSequencePath);

        if( !QFile(strInputFile).exists() ) {
            throw QtArgBaseException("Input File Not Exist");
        }
    } catch( const QtArgBaseException & cErr ) {
        cerr << cErr.what() << endl;
        exit(-1);
    }


    //open file
    QFile cInputFile(strInputFile);
    QTextStream cInputStream;
    if( cInputFile.open(QFile::ReadOnly) ) {
        cInputStream.setDevice(&cInputFile);

    }else{
        cerr << "Open File Error" << endl;
        exit(-1);
    }

    //parse file
    GeneralParser cGeneralParser;
    ComSequence cComSequence;
    cGeneralParser.parseFile(&cInputStream, &cComSequence);

    //close fil
    cInputFile.close();
*/
}
