#include "switchyuvcommand.h"
#include "model/modellocator.h"
#include <QDebug>

static struct YUVInfo
{
    YUVRole eRole;
    QString strRoleName;
    QString strYUVName;
    bool    bIs16Bit;
}s_aYUVInfo[] =
{
    {YUV_PREDICTED,     "predicted",    "pred_yuv.yuv"},
    {YUV_RESIDUAL,      "residual",     "resi_yuv.yuv"},
    {YUV_RECONSTRUCTED, "reconstructed","decoder_yuv.yuv"},
    {YUV_NONE,          "",             ""}             /// end mark
};



SwitchYUVCommand::SwitchYUVCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool SwitchYUVCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
      bool bIs16Bit = false;
//    if(rcInputArg.hasParameter("is_16_bit"))
//    {
//        bIs16Bit = rcInputArg.getParameter("is_16_bit").toBool();
//    }

//    QString strYUVFilename = rcInputArg.getParameter("YUV_filename").toString();

      //TODO YUV Role should be passed as arg, not changed outside this command
    ComSequence* pcSequence = (ComSequence*)rcInputArg.getParameter("sequence").value<void*>();
    QString strYUVFilename = xGetYUVFilenameByRole(pcSequence->getYUVRole());
    if(pcSequence->getYUVRole() == YUV_RESIDUAL)
        bIs16Bit = true;
    ModelLocator* pModel = ModelLocator::getInstance();
    if( pcSequence != &(pModel->getSequenceManager().getCurrentSequence()) )
    {
        qWarning() << "Not current displaying sequence, cannot switch to residual";
        return false;
    }

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

QString SwitchYUVCommand::xGetYUVFilenameByRole(YUVRole eRole)
{
    YUVInfo*p = s_aYUVInfo;
    while(p->eRole != YUV_NONE)
    {
        if(p->eRole == eRole)
            return p->strYUVName;
        p++;
    }
    qWarning() << "Invalid YUV Role";
    return QString();
}
