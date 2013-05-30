#include <QCoreApplication>
#include <iostream>
#include <QtTest/QtTest>
#include <QTest>
#include "gitldef.h"
#include "gitlabstractcommand.h"
#include "gitlfrontcontroller.h"
using namespace std;

///
class TestCommand : public GitlAbstractCommand
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit TestCommand(QObject *parent = 0):GitlAbstractCommand(parent) {}
    Q_INVOKABLE bool execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
    {
        rcRespond.setParameter("test_para", true);
        return true;
    }

};


/// test case
class TestCase : public QObject
{
    Q_OBJECT

private slots:
    void basicTest()
    {
        GitlFrontController* pcFC = GitlFrontController::getInstance();

        pcFC->addCommand("test_command_name", &TestCommand::staticMetaObject);

        GitlCommandRequest cRequest; GitlCommandRespond cRespond;
        cRequest.setParameter("command_name", "test_command_name");
        pcFC->processRequest(cRequest, cRespond);

        QVERIFY(cRespond.getParameter("test_para").toBool());
    }



};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

