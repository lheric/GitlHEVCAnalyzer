#include "zoomframecommand.h"
#include "model/modellocator.h"
ZoomFrameCommand::ZoomFrameCommand(QObject *parent) :
    AbstractCommand(parent)
{
}

bool ZoomFrameCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    QVariant vValue;
    double dScale = 0.0;
    if( rcRequest.getParameter("scale", vValue) )
    {
        dScale = vValue.toDouble();
    }
    else
    {
        return false;
    }
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getDrawEngine().setFrameScale(dScale);
    int iPoc = pModel->getFrameBuffer().getPoc();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);
    pcFramePixmap = pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcRespond.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));

    return true;

}
