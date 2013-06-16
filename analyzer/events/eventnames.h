#ifndef EVENTNAMES_H
#define EVENTNAMES_H
#include <QString>

/**
 * @brief List of events identifications
 */
const QString g_strStatusMsgEvent = "STATUS_MSG_EVENT";     ///< current status of app
const QString g_strCmdSentEvent   = "CMD_SENT_EVENT";       ///< a command has been sent to event bus
const QString g_strCmdStartEvent  = "CMD_START_EVENT";      ///< a command started
const QString g_strCmdInfoEvent   = "CMD_INFO_EVENT";       ///< informations during running a command
const QString g_strCmdEndEvent    = "CMD_END_EVENT";        ///< a command finished

//TODO refactor: remove this
const QString g_strSquencesListChanged = "SQUENCES_LIST_CHANGED_EVENT";  ///< Sequence added or removed

#endif // EVENTNAMES_H
