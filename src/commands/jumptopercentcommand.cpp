#include "jumptopercentcommand.h"
#include "model/modellocator.h"
JumpToPercentCommand::JumpToPercentCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool JumpToPercentCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    ComSequence *pcCurSeq = pModel->getSequenceManager().getCurrentSequence();
    if(pcCurSeq == NULL)
        return false;
    QVariant vValue = rcInputArg.getParameter("percent");
    int iPercent = vValue.toInt();
    int iMaxPOC = pcCurSeq->getTotalFrames()-1;
    int iMinPOC = 0;
    int iPoc = int(iPercent*iMaxPOC/100.0+0.5);

    /// [Optimization] if poc is not changed, do nothing
    if( iPoc == pModel->getFrameBuffer().getFrameCount() )
        return true;

    if( iPoc > iMaxPOC || iPoc < iMinPOC )
    {
        return false;
    }

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);       ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(pcCurSeq, iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcOutputArg.setParameter("current_frame_poc", iPoc);
    rcOutputArg.setParameter("total_frame_num", pcCurSeq->getTotalFrames());

    return true;
}
