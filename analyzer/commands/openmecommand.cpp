#include "openmecommand.h"
#include "model/modellocator.h"

OpenMECommand::OpenMECommand()
{
}


bool OpenMECommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    /*QVariant vValue;
    rcRequest.getParameter("filename", vValue);
    QString strFilename = vValue.toString();

    ModelLocator* pModel = ModelLocator::getInstance();
    bool bOpen = pModel->openMEInfo(strFilename);

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
    }*/
    return false;

}
