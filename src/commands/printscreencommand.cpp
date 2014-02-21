#include "printscreencommand.h"
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
PrintScreenCommand::PrintScreenCommand()
{
}

bool PrintScreenCommand::execute( GitlCommandParameter& rcInputArg, GitlCommandParameter& rcOutputArg )
{
    ModelLocator* pModel = ModelLocator::getInstance();
    ComSequence *pcCurSeq = pModel->getSequenceManager().getCurrentSequence();
    if(pcCurSeq == NULL)
        return false;

    int iCurBufPoc = pModel->getFrameBuffer().getFrameCount();
    QPixmap* pcFramePixmap = pModel->getFrameBuffer().getFrame(iCurBufPoc);   ///< Read Frame Buffer
    if( pcFramePixmap == NULL )
        return false;
    pcFramePixmap = pModel->getDrawEngine().drawFrame(pcCurSeq, iCurBufPoc, pcFramePixmap);  ///< Draw Frame Buffer
    rcOutputArg.setParameter("snapshot",  QVariant::fromValue((void*)(pcFramePixmap)));
    return true;


}
