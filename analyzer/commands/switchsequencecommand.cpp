#include "switchsequencecommand.h"
#include "io/analyzermsgsender.h"
#include "modellocator.h"

SwitchSequenceCommand::SwitchSequenceCommand(QObject *parent) :
    AbstractCommand(parent)
{
}

bool SwitchSequenceCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    QVariant vValue;
    rcRequest.getParameter("sequence", vValue);
    ComSequence* pcSequence = (ComSequence*)vValue.value<void*>();
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getSequenceManager().setCurrentSequence(pcSequence);

    //
    int iWidth = pcSequence->getWidth();
    int iHeight = pcSequence->getHeight();
    pModel->getFrameBuffer().setYUVFile(pcSequence->getDecodingFolder()+"/decoder_yuv.yuv", iWidth, iHeight);
    int iPoc = pModel->getFrameBuffer().getPoc();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);   ///< Read Frame Buffer
    pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer
    //
    rcRespond.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));

    return true;
}
