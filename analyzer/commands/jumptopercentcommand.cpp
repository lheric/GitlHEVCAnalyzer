#include "jumptopercentcommand.h"
#include "modellocator.h"
JumpToPercentCommand::JumpToPercentCommand(QObject *parent) :
    AbstractCommand(parent)
{
}


bool JumpToPercentCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    //TODO
    ModelLocator* pModel = ModelLocator::getInstance();
    QVariant vValue;
    rcRequest.getParameter("percent", vValue);
    int iPercent = vValue.toInt();
    int iMaxPOC = pModel->getSequenceManager().getCurrentSequence().getTotalFrames()-1;
    int iMinPOC = 0;
    int iPoc = int(iPercent*iMaxPOC/100.0+0.5);

    if( iPoc > iMaxPOC || iPoc < iMinPOC )
    {
        return false;
    }

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);       ///< Read Frame Buffer
    pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcRespond.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcRespond.setParameter("current_frame_poc", iPoc);
    rcRespond.setParameter("total_frame_num", pModel->getSequenceManager().getCurrentSequence().getTotalFrames());

    return true;
}
