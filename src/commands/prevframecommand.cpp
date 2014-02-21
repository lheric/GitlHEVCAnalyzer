#include "prevframecommand.h"
#include <QPixmap>
PrevFrameCommand::PrevFrameCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool PrevFrameCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    ComSequence *pcCurSeq = pModel->getSequenceManager().getCurrentSequence();
    if(pcCurSeq == NULL)
        return false;

    int iCurBufPoc = pModel->getFrameBuffer().getFrameCount();
    int iPredPoc = iCurBufPoc - 1;
    if( iPredPoc < 0)
        return false;

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPredPoc);   ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(pcCurSeq, iPredPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcOutputArg.setParameter("current_frame_poc", iPredPoc);
    rcOutputArg.setParameter("total_frame_num", pcCurSeq->getTotalFrames());

    return true;
}
