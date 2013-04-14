#include "refreshscreencommand.h"
#include "model/modellocator.h"
#include <QPixmap>
RefreshScreenCommand::RefreshScreenCommand(QObject *parent) :
    AbstractCommand(parent)
{

}

bool RefreshScreenCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    int iPoc = pModel->getFrameBuffer().getPoc();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);
    pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcRespond.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));

    return true;
}
