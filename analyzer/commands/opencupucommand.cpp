#include "opencupucommand.h"

#include "model/modellocator.h"

OpenCUPUCommand::OpenCUPUCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool OpenCUPUCommand::execute( GitlCommandRequest& rcRequest, GitlCommandRespond& rcRespond )
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
