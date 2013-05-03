#include "configfiltercommand.h"
#include "model/modellocator.h"

ConfigFilterCommand::ConfigFilterCommand(QObject *parent) :
    AbstractCommand(parent)
{
}

bool ConfigFilterCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    QVariant vValue;
    rcRequest.getParameter("filter", vValue);
    AbstractFilter* pcFilter = (AbstractFilter*)(vValue.value<void*>());
    pModel->getDrawEngine().getFilterLoader().config(pcFilter);

    int iPoc = pModel->getFrameBuffer().getPoc();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);       ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer
    rcRespond.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    return true;
}
