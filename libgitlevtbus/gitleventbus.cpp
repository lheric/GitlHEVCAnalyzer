#include "gitleventbus.h"
#include "gitliomsg.h"

SINGLETON_PATTERN_IMPLIMENT(GitlEventBus)

GitlEventBus::GitlEventBus()
{    
}

GitlEventBus::~GitlEventBus()
{
}


/*! connect a modual to the event bus
  */
bool GitlEventBus::registerModual(GitlModualDelegate* pcModual)
{
    QMutexLocker cModualLocker(&m_cModualQueMutex);
    qRegisterMetaType<GitlEvent>("GitlEvent");
    connect(&m_cBlockEvtDispatcher,    SIGNAL(eventTriggered(GitlEvent)), pcModual, SLOT(detonate(GitlEvent)));

    return true;
}



/*! send event to event bus
  */
void GitlEventBus::post(GitlEvent* pcEvt)
{
    QMutexLocker cModualLocker(&m_cModualDispMutex);

    m_cBlockEvtDispatcher.postEvent(pcEvt);


}

