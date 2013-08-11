#include "switchsequencecommand.h"
#include "model/modellocator.h"

SwitchSequenceCommand::SwitchSequenceCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool SwitchSequenceCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    bool bIs16Bit = false;
    if(rcInputArg.hasParameter("is_16_bit"))
    {
        bIs16Bit = rcInputArg.getParameter("is_16_bit").toBool();
    }

    QString strYUVFilename = rcInputArg.getParameter("YUV_filename").toString();
    ComSequence* pcSequence = (ComSequence*)(rcInputArg.getParameter("sequence").value<void*>());
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->getSequenceManager().setCurrentSequence(pcSequence);

    //
    int iWidth = pcSequence->getWidth();
    int iHeight = pcSequence->getHeight();
    pModel->getFrameBuffer().openYUVFile(pcSequence->getDecodingFolder()+"/"+strYUVFilename, iWidth, iHeight, bIs16Bit);
    int iPoc = pModel->getFrameBuffer().getPoc();

    int iMaxPoc = pModel->getSequenceManager().getCurrentSequence().getTotalFrames()-1;
    int iMinPoc = 0;

    iPoc = VALUE_CLIP(iMinPoc, iMaxPoc, iPoc);

    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iPoc);       ///< Read Frame Buffer
    pcFramePixmap = pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    rcOutputArg.setParameter("picture",  QVariant::fromValue((void*)(pcFramePixmap)));
    rcOutputArg.setParameter("current_frame_poc", iPoc);
    rcOutputArg.setParameter("total_frame_num", pModel->getSequenceManager().getCurrentSequence().getTotalFrames());



    return true;
}
