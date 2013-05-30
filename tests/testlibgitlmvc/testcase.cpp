#include <QCoreApplication>
#include <iostream>
#include <QtTest/QtTest>
#include <QTest>
#include "gitldef.h"

using namespace std;

///
class TestModual : public GitlModual
{
public:
    TestModual()
    {
        this->m_bNotified = false;
    }

    virtual bool detonate( GitlEvent cEvt )
    {
        this->m_bNotified = true;
        return true;
    }

    ADD_CLASS_FIELD_NOSETTER(bool, bNotified, getNotified)
};


/// test case
class TestCase : public QObject
{
    Q_OBJECT

private slots:
    void oneSendAnotherListen()
    {

        TestModual cSender;
        TestModual cListener;
        cListener.subscribeToEvtByName("TEST_EVENT_1");
        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cListener.getNotified());
    }

    void selfSendAndListen()
    {
        TestModual cSender;
        cSender.subscribeToEvtByName("TEST_EVENT_1");
        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cSender.getNotified());
    }

    void unsubscribe()
    {
        TestModual cSender;
        cSender.subscribeToEvtByName("TEST_EVENT_1");
        cSender.unsubscribeToEvtByName("TEST_EVENT_1");
        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(!cSender.getNotified());
    }



    void oneToMany()
    {
        TestModual cSender;
        TestModual cListener1;
        TestModual cListener2;
        TestModual cListener3;
        cListener1.subscribeToEvtByName("TEST_EVENT_1");
        cListener2.subscribeToEvtByName("TEST_EVENT_1");
        cListener3.subscribeToEvtByName("TEST_EVENT_2");

        GitlEvent cEvt("TEST_EVENT_1");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cListener1.getNotified());
        QVERIFY(cListener2.getNotified());
        QVERIFY(!cListener3.getNotified());

        cEvt.setEvtName("TEST_EVENT_2");
        cSender.dispatchEvt(cEvt);
        QVERIFY(cListener3.getNotified());

    }



};


/// test main
QTEST_MAIN(TestCase)
#include "testcase.moc"

