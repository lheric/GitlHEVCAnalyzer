#include "querypreferencescommand.h"
#include "model/modellocator.h"
#include <QDir>
QueryPreferencesCommand::QueryPreferencesCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}


bool QueryPreferencesCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    ModelLocator* pModel = ModelLocator::getInstance();
    QString strCacheFolder = pModel->getPreferences().getCacheFolder();
    QDir cCacheFolder(strCacheFolder);
    Q_ASSERT(cCacheFolder.exists());
    cCacheFolder.cdUp();
    Q_ASSERT(cCacheFolder.exists());
    strCacheFolder = cCacheFolder.absolutePath();
    rcOutputArg.setParameter("cache_path",   strCacheFolder);
    return true;
}
