#include "zoomframecommand.h"
#include "model/modellocator.h"
ZoomFrameCommand::ZoomFrameCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool ZoomFrameCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{
    QVariant vValue;
    double dScale = 0.0;
    if( rcRequest.hasParameter("scale") )
    {
        vValue = rcRequest.getParameter("scale");
        dScale = vValue.toDouble();
    }
    else
    {
        return false;
    }
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getDrawEngine().setScale(dScale);
    int iPoc = pModel->getFrameBuffer().getPoc();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);
    pcFramePixmap = pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcRespond.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));

    return true;

}
