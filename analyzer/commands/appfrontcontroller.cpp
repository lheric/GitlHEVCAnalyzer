#include "appfrontcontroller.h"
#include "exceptions/nosequencefoundexception.h"
#include "exceptions/invaildfilterindexexception.h"
#include "events/eventnames.h"

#include "commands/decodebitstreamcommand.h"
#include "commands/openencodergeneralcommand.h"
#include "commands/openyuvcommand.h"
#include "commands/opencupucommand.h"
#include "commands/openmecommand.h"
#include "commands/prevframecommand.h"
#include "commands/nextframecommand.h"
#include "commands/jumptoframecommand.h"
#include "commands/jumptopercentcommand.h"
#include "commands/printscreencommand.h"
#include "commands/switchsequencecommand.h"
#include "commands/refreshscreencommand.h"
#include "io/analyzermsgsender.h"


SINGLETON_PATTERN_IMPLIMENT(AppFrontController)

static CommandFormat s_sCmdTable[] =
{
    { "decode_bitstream", &DecodeBitstreamCommand::staticMetaObject },
    { "open_yuv",       &OpenYUVCommand::staticMetaObject },
    { "open_general",   &OpenEncoderGeneralCommand::staticMetaObject },
    { "open_cupu",      &OpenCUPUCommand::staticMetaObject },
    { "open_me",        &OpenMECommand::staticMetaObject },
    { "next_frame",     &NextFrameCommand::staticMetaObject },
    { "prev_frame",     &PrevFrameCommand::staticMetaObject },
    { "jumpto_frame",   &JumpToFrameCommand::staticMetaObject },
    { "jumpto_percent", &JumpToPercentCommand::staticMetaObject },
    { "switch_sequence",&SwitchSequenceCommand::staticMetaObject },
    { "print_screen",   &PrintScreenCommand::staticMetaObject },
    { "refresh_screen", &RefreshScreenCommand::staticMetaObject },
    { "",               NULL                                      }
};


AppFrontController::AppFrontController()
{
    m_iMaxEvtInQue = 1000;
    xInitCommand();
    setModualName("app_front_controller");
    listenToEvtByName(g_strCmdSentEvent);
    this->start();
}

bool AppFrontController::xInitCommand()
{
    /// register commands

    CommandFormat* psCMD = s_sCmdTable;
    while( psCMD->pMetaObject != NULL )
    {
        addCommand(*psCMD);
        psCMD++;
    }
    return true;
}


bool AppFrontController::detonate( GitlEvent cEvt )
{    

    m_cEvtQueMutex.lock();
    if( m_cEvtQue.size() >= m_iMaxEvtInQue )
    {
        AnalyzerMsgSender::getInstance()->msgOut(QString("Too Many Events Pending...Waiting..."), GITL_MSG_DEBUG);
        m_cEvtQueNotFull.wait(&m_cEvtQueMutex);
        AnalyzerMsgSender::getInstance()->msgOut(QString("Event Queue Not Full...Moving on..."), GITL_MSG_DEBUG);
    }
    m_cEvtQue.push_back(cEvt);
    m_cEvtQueMutex.unlock();
    m_cEvtQueNotEmpty.wakeAll();
    return true;

}


void AppFrontController::run()
{

    forever
    {
        /// get one event from the waiting queue
        m_cEvtQueMutex.lock();
        if( m_cEvtQue.empty() )
        {
            m_cEvtQueNotEmpty.wait(&m_cEvtQueMutex);
        }
        GitlEvent cEvt = m_cEvtQue.front();
        m_cEvtQue.pop_front();
        m_cEvtQueMutex.unlock();
        m_cEvtQueNotFull.wakeAll();

        /// deal with the event
        //AnalyzerMsgSender::getInstance()->msgOut(QString("Next Event Fetched!"), GITL_MSG_DEBUG);

            ///check if has request parameter
        if( !cEvt.getEvtData().hasParameter("request") )
        {
            AnalyzerMsgSender::getInstance()->msgOut(QString("Front Controller Received Command Event Without \"Request\", Command Exit!"), GITL_MSG_ERROR);
            continue;
        }

            /// prepare request and respond
        CommandRequest cRequest;
        CommandRespond cRespond;
        QVariant vValue;
        cEvt.getEvtData().getParameter("request", vValue);
        cRequest = vValue.value<CommandRequest>();

        GitlEvent cCmdStartEvt( g_strCmdStartEvent );               ///
        cCmdStartEvt.getEvtData().setParameter("request", vValue);  /// start command event
        dispatchEvt(&cCmdStartEvt);                                 ///

        /// do command & exception handling
        try
        {
            processRequest(cRequest, cRespond);
        }
        catch( const NoSequenceFoundException& )
        {
            AnalyzerMsgSender::getInstance()->msgOut("No Video Sequence Found...", GITL_MSG_ERROR);
        }
        catch( const InvaildFilterIndexException* )
        {
            AnalyzerMsgSender::getInstance()->msgOut("Invalid Filter Index...", GITL_MSG_ERROR);
        }
        catch( const QException& )
        {
            AnalyzerMsgSender::getInstance()->msgOut("Unknown Error Happened...", GITL_MSG_ERROR);
        }

        GitlEvent cCmdEndEvt( g_strCmdEndEvent );                                       ///
        cCmdEndEvt.getEvtData().setParameter("respond", QVariant::fromValue(cRespond)); /// end command event
        dispatchEvt(&cCmdEndEvt);                                                       ///

    }


}
