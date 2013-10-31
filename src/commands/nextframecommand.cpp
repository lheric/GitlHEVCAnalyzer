#include "nextframecommand.h"

NextFrameCommand::NextFrameCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{

}

bool NextFrameCommand::execute( GitlCommandParameter &rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    ComSequence *pcCurSeq = pModel->getSequenceManager().getCurrentSequence();
    if(pcCurSeq == NULL)
        return false;

    int iCurBufPoc = pModel->getFrameBuffer().getPoc();
    int iNextPoc = iCurBufPoc+1;
    if( iNextPoc >= pcCurSeq->getTotalFrames())
        return false;

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iNextPoc);   ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(pcCurSeq, iNextPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcOutputArg.setParameter("current_frame_poc", iNextPoc );
    rcOutputArg.setParameter("total_frame_num", pcCurSeq->getTotalFrames());

    return true;
}
