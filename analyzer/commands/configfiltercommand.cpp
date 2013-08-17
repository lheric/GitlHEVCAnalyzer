#include "configfiltercommand.h"
#include "model/modellocator.h"
#include <QString>

ConfigFilterCommand::ConfigFilterCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool ConfigFilterCommand::execute( GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    QString strFiltername = rcInputArg.getParameter("filter_name").toString();
    AbstractFilter* pcFilter = pModel->getDrawEngine().getFilterLoader().getFilterByName(strFiltername);
    if(pcFilter == NULL)
        return false;
    pModel->getDrawEngine().getFilterLoader().config(pcFilter);
    int iPoc = pModel->getFrameBuffer().getPoc();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);       ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    return true;
}
