#include "jumptoframecommand.h"


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
    if( iPoc > pModel->getSequenceManager().getCurrentSequence().getTotalFrames()-1 ||
        iPoc < 0 )
    {
        return false;
    }

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);       ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcOutputArg.setParameter("current_frame_poc", iPoc);
    rcOutputArg.setParameter("total_frame_num", pModel->getSequenceManager().getCurrentSequence().getTotalFrames());

    return true;
}
