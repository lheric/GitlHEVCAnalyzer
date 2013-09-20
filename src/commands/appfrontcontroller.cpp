#include "appfrontcontroller.h"
#include "exceptions/nosequencefoundexception.h"
#include "exceptions/invaildfilterindexexception.h"
#include "exceptions/decodernotfoundexception.h"
#include "exceptions/decodingfailexception.h"
#include "exceptions/bitstreamnotfoundexception.h"
#include "commands/decodebitstreamcommand.h"
#include "commands/prevframecommand.h"
#include "commands/nextframecommand.h"
#include "commands/jumptoframecommand.h"
#include "commands/jumptopercentcommand.h"
#include "commands/printscreencommand.h"
#include "commands/switchsequencecommand.h"
#include "commands/switchyuvcommand.h"
#include "commands/refreshscreencommand.h"
#include "commands/checkupdatecommand.h"
#include "commands/filterorderdowncommand.h"
#include "commands/filterorderupcommand.h"
#include "commands/zoomframecommand.h"
#include "commands/configfiltercommand.h"
#include "commands/reloadfilterscommand.h"
#include "commands/switchfiltercommand.h"


SINGLETON_PATTERN_IMPLIMENT(AppFrontController)

static struct
{
    char* strCommandName;
    const QMetaObject* pMetaObject;
}
s_sCmdTable[] =
{
    { "decode_bitstream", &DecodeBitstreamCommand::staticMetaObject    },
    { "next_frame",       &NextFrameCommand::staticMetaObject          },
    { "prev_frame",       &PrevFrameCommand::staticMetaObject          },
    { "jumpto_frame",     &JumpToFrameCommand::staticMetaObject        },
    { "jumpto_percent",   &JumpToPercentCommand::staticMetaObject      },
    { "switch_sequence",  &SwitchSequenceCommand::staticMetaObject     },
    { "switch_yuv",       &SwitchYUVCommand::staticMetaObject          },
    { "print_screen",     &PrintScreenCommand::staticMetaObject        },
    { "refresh_screen",   &RefreshScreenCommand::staticMetaObject      },
    { "zoom_frame",       &ZoomFrameCommand::staticMetaObject          },
    { "reload_filter",    &ReloadFiltersCommand::staticMetaObject      },
    { "config_filter",    &ConfigFilterCommand::staticMetaObject       },
    { "moveup_filter",    &FilterOrderUpCommand::staticMetaObject      },
    { "movedown_filter",  &FilterOrderDownCommand::staticMetaObject    },
    { "switch_filter",    &SwitchFilterCommand::staticMetaObject       },
    { "check_update",     &CheckUpdateCommand::staticMetaObject        },
    { "",                 NULL                                         }
};


AppFrontController::AppFrontController()
{
    m_iMaxEvtInQue = 1000;
    xInitCommand();
    setModualName("app_front_controller");
    this->start();
}

bool AppFrontController::xInitCommand()
{
    /// register commands

    auto* psCMD = s_sCmdTable;
    while( psCMD->pMetaObject != NULL )
    {
        registerCommand(psCMD->strCommandName, psCMD->pMetaObject);
        psCMD++;
    }
    return true;
}


bool AppFrontController::detonate(GitlEvent &cEvt )
{    

    m_cEvtQueMutex.lock();
    if( m_pcEvtQue.size() >= m_iMaxEvtInQue )
    {
        qCritical() << QString("Too Many Events Pending...Waiting...");
        m_cEvtQueNotFull.wait(&m_cEvtQueMutex);
        qDebug() << QString("Event Queue Not Full...Moving on...");
    }
    m_pcEvtQue.push_back(cEvt.clone());
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
        if( m_pcEvtQue.empty() )
        {
            m_cEvtQueNotEmpty.wait(&m_cEvtQueMutex);
        }
        GitlEvent* pcEvt = m_pcEvtQue.front();
        m_pcEvtQue.pop_front();
        m_cEvtQueMutex.unlock();
        m_cEvtQueNotFull.wakeAll();


        /// do command & exception handling
        try
        {
            GitlFrontController::detonate(*pcEvt);
            delete pcEvt;
        }
        catch( const NoSequenceFoundException& )
        {
            qCritical() << "No Video Sequence Found...";
        }
        catch( const InvaildFilterIndexException& )
        {
            qWarning() << "Invalid Filter Index...";
        }
        catch( const DecoderNotFoundException& )
        {
            qCritical() << "Decoder NOT Found...";
        }
        catch( const DecodingFailException& )
        {
            qCritical() << "Bitstream decoding FAILED!\n"
                           "Following may be the reasons for this failure:\n"
                           "1. Corrupted HEVC bitstream?\n"
                           "2. Bitstream-decoder version mismatch?\n"
                           "3. Non-English characters in bitstream path or analyzer path?\n";
        }
        catch( const BitstreamNotFoundException& )
        {
            qWarning() << "Bitstream NOT Found...";
        }
        catch( const QException& )
        {
            qCritical() << "Unknown Error Happened... :(";
        }

    }


}
