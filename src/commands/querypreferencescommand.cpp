#include "querypreferencescommand.h"
#include "model/modellocator.h"
QueryPreferencesCommand::QueryPreferencesCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool QueryPreferencesCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    ModelLocator* pModel = ModelLocator::getInstance();
    rcOutputArg.setParameter("cache_path",   pModel->getPreferences().getCacheFolder());
    rcOutputArg.setParameter("decoder_path", pModel->getPreferences().getDecoderFolder());
    return true;
}
