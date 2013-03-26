#include "opencupucommand.h"

#include "../modellocator.h"

OpenCUPUCommand::OpenCUPUCommand(QObject *parent) :
    AbstractCommand(parent)
{
}

bool OpenCUPUCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    /*QVariant vValue;
    rcRequest.getParameter("filename", vValue);
    QString strFilename = vValue.toString();

    ModelLocator* pModel = ModelLocator::getInstance();
    bool bOpen = pModel->openCUInfo(strFilename);

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
