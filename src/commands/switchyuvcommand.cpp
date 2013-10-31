#include "switchyuvcommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
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
    ModelLocator* pModel = ModelLocator::getInstance();
    ComSequence *pcCurSeq = pModel->getSequenceManager().getCurrentSequence();
    if(pcCurSeq == NULL)
        return false;

    ComSequence* pcSequence = (ComSequence*)rcInputArg.getParameter("sequence").value<void*>();
    if(rcInputArg.hasParameter("yuv_type"))
    {
        YUVRole eRole = rcInputArg.getParameter("yuv_type").value<YUVRole>();;
        pcSequence->setYUVRole(eRole);
    }

    QString strYUVFilename = xGetYUVFilenameByRole(pcSequence->getYUVRole());

    bool bIs16Bit = false;
    if(pcSequence->getYUVRole() == YUV_RESIDUAL)
        bIs16Bit = true;

    if( pcSequence == pcCurSeq )
    {
        //
        int iWidth = pcSequence->getWidth();
        int iHeight = pcSequence->getHeight();
        pModel->getFrameBuffer().openYUVFile(pcSequence->getDecodingFolder()+"/"+strYUVFilename, iWidth, iHeight, bIs16Bit);

        /// refresh
        GitlIvkCmdEvt cRefreshEvt("refresh_screen");
        cRefreshEvt.dispatch();
    }
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
