#include "closebitstreamcommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
CloseBitstreamCommand::CloseBitstreamCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool CloseBitstreamCommand::execute( GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg )
{
    QString strSequencePath = rcInputArg.getParameter("sequence_path").toString();
    ModelLocator* pModel = ModelLocator::getInstance();
    SequenceManager& rcSequenceManager  = pModel->getSequenceManager();
    ComSequence* pcSequence = rcSequenceManager.getSequenceByFilename(strSequencePath);
    if( rcSequenceManager.getAllSequences().size() > 1 &&   /// TODO do not allow close if there is only one sequence
        rcSequenceManager.delSequence(pcSequence) )
    {
        ComSequence* pcLatestSequence = rcSequenceManager.getAllSequences().back();
        GitlIvkCmdEvt cSwitchSeq("switch_sequence");
        cSwitchSeq.setParameter("sequence", QVariant::fromValue((void*)pcLatestSequence));
        cSwitchSeq.dispatch();
        return true;
    }
    else
    {
        qDebug() << "Cannot close the last bitstream...";
    }
    return false;
}
