#include "jumptoframecommand.h"
#include "gitlivkcmdevt.h"

JumpToFrameCommand::JumpToFrameCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool JumpToFrameCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    QVariant vValue = rcInputArg.getParameter("poc");
    int iPoc = vValue.toInt();
    ///
    ModelLocator* pModel = ModelLocator::getInstance();
    ComSequence *pcCurSeq = pModel->getSequenceManager().getCurrentSequence();
    if(pcCurSeq == NULL)
        return false;

    if( iPoc > pcCurSeq->getTotalFrames()-1 ||
        iPoc < 0 )
    {
        return false;
    }

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);   ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(pcCurSeq, iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcOutputArg.setParameter("current_frame_poc", iPoc);
    rcOutputArg.setParameter("total_frame_num", pcCurSeq->getTotalFrames());

    return true;
}
