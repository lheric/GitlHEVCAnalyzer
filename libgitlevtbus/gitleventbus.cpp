#include "gitleventbus.h"


SINGLETON_PATTERN_IMPLIMENT(GitlEventBus)

GitlEventBus::GitlEventBus()
{
}

/*! connect a modual to the event bus
  */
bool GitlEventBus::registerModual(GitlModualDelegate* pcModual)
{
    QMutexLocker cModualLocker(&m_cModualQueMutex);
    qRegisterMetaType<GitlEvent>("GitlEvent");
    connect(this, SIGNAL(eventTriggered(GitlEvent)),
            pcModual, SLOT(detonate(GitlEvent)), Qt::AutoConnection );

    return true;
}



/*! send event to event bus
  */
void GitlEventBus::post(GitlEvent* pcEvt)
{
    QMutexLocker cModualLocker(&m_cModualDispMutex);
    /// notify moduals
    emit eventTriggered(*pcEvt);

}

