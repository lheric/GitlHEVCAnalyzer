#include "refreshscreencommand.h"
#include "model/modellocator.h"
#include <QPixmap>
RefreshScreenCommand::RefreshScreenCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{

}

bool RefreshScreenCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    ComSequence *pcCurSeq = pModel->getSequenceManager().getCurrentSequence();
    if(pcCurSeq == NULL)
        return false;

    int iPoc = pModel->getFrameBuffer().getPoc();
    int iMaxPoc = pcCurSeq->getTotalFrames()-1;
    int iMinPoc = 0;
    iPoc = VALUE_CLIP(iMinPoc, iMaxPoc, iPoc);

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);
    pcFramePixmap = pModel->getDrawEngine().drawFrame(pcCurSeq, iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcOutputArg.setParameter("current_frame_poc", iPoc);
    rcOutputArg.setParameter("total_frame_num", pcCurSeq->getTotalFrames());

    return true;
}
