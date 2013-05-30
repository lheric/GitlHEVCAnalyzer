#include "openyuvcommand.h"
#include "model/modellocator.h"

OpenYUVCommand::OpenYUVCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool OpenYUVCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
{
    /*QVariant vValue;
    rcRequest.getParameter("filename", vValue);
    QString strFilename = vValue.toString();
    rcRequest.getParameter("resolution_x", vValue);
    int iResX = vValue.toInt();
    rcRequest.getParameter("resolution_y", vValue);
    int iResY = vValue.toInt();

    //
    ModelLocator* pModel = ModelLocator::getInstance();
    pModel->openYUVSequence(strFilename, iResX, iResY);
    pModel->readFrameToBuf(0);
    pModel->renderFrame();

    //
    int iCurrentFrame = pModel->getBufferPoc() + 1;
    int iTotalFrame = pModel->getSequence().getTotalFrames();
    void* pcPixelMap = (void*)(&(pModel->getPixMap()));
    rcRespond.setParameter("picture",  QVariant::fromValue(pcPixelMap));
    rcRespond.setParameter("current_frame_poc", iCurrentFrame);
    rcRespond.setParameter("total_frame_num", iTotalFrame);

    return true;*/

    return false;
}
