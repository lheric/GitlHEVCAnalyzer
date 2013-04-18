#include "jumptoframecommand.h"


JumpToFrameCommand::JumpToFrameCommand(QObject *parent) :
    AbstractCommand(parent)
{
}


bool JumpToFrameCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    QVariant vValue;
    rcRequest.getParameter("poc", vValue);
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
    rcRespond.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcRespond.setParameter("current_frame_poc", iPoc);
    rcRespond.setParameter("total_frame_num", pModel->getSequenceManager().getCurrentSequence().getTotalFrames());

    return true;
}
