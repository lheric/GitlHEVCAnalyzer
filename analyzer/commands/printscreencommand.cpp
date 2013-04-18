#include "printscreencommand.h"

PrintScreenCommand::PrintScreenCommand()
{
}

bool PrintScreenCommand::execute( CommandRequest& rcRequest, CommandRespond& rcRespond )
{
    ModelLocator* pModel = ModelLocator::getInstance();

    int iCurBufPoc = pModel->getFrameBuffer().getPoc();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iCurBufPoc);   ///< Read Frame Buffer
    if( pcFramePixmap == NULL )
        return false;
    pcFramePixmap = pModel->getDrawEngine().drawFrame(&(pModel->getSequenceManager().getCurrentSequence()), iCurBufPoc, pcFramePixmap);  ///< Draw Frame Buffer

    ///
    return pcFramePixmap->save("snapshot.png");
}
