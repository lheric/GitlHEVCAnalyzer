#include "opendecodergeneralcommand.h"
#include "model/modellocator.h"
OpenDecoderGeneralCommand::OpenDecoderGeneralCommand(QObject *parent) :
    AbstractCommand(parent)
{
}
bool OpenDecoderGeneralCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    /*QVariant vValue;
    rcRequest.getParameter("filename", vValue);
    QString strFilename = vValue.toString();

    ModelLocator* pModel = ModelLocator::getInstance();
    bool bOpen = pModel->openDecoderGeneralInfo(strFilename);
    if( bOpen == true)
    {
        pModel->renderFrame();
        void* pcPixelMap = (void*)(&(pModel->getPixMap()));
        rcRespond.setParameter("picture",  QVariant::fromValue(pcPixelMap));
        return true;
    }
    else
    {
        return false;
    }
    return true;*/
    return false;
}
