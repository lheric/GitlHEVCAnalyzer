#include "switchsequencecommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"

SwitchSequenceCommand::SwitchSequenceCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool SwitchSequenceCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ComSequence* pcSequence = (ComSequence*)(rcInputArg.getParameter("sequence").value<void*>());
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getSequenceManager().setCurrentSequence(pcSequence);

    /// read YUV
    GitlIvkCmdEvt cSwitchYUV("switch_yuv");
    cSwitchYUV.setParameter("sequence", QVariant::fromValue((void*)pcSequence));
    cSwitchYUV.dispatch();

    /// re-load & re-init filters
    pModel->getDrawEngine().getFilterLoader().reinitAllFilters();

    /// refresh screen
    GitlIvkCmdEvt cRefresh("refresh_screen");
    cRefresh.dispatch();

    /// notify UI update
    QVector<ComSequence*>* ppcSequences = &(pModel->getSequenceManager().getAllSequences());
    rcOutputArg.setParameter("sequences",QVariant::fromValue((void*)ppcSequences));
    rcOutputArg.setParameter("current_sequence",QVariant::fromValue((void*)pcSequence));

    return true;
}
