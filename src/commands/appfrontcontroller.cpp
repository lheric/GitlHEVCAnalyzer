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
#include "commands/querypreferencescommand.h"
#include "commands/modifypreferencescommand.h"


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
    { "modify_pref",      &ModifyPreferencesCommand::staticMetaObject  },
    { "query_pref",       &QueryPreferencesCommand::staticMetaObject   },
    { "",                 NULL                                         }
};


AppFrontController::AppFrontController()
{
    xInitCommand();
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


void AppFrontController::run()
{

    forever
    {

        /// do command & exception handling
        try
        {
            GitlMTFrontController::run();
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
