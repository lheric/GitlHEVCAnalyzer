#include "cleancachecommand.h"
#include "model/modellocator.h"
#include "gitlivkcmdevt.h"
#include <QDir>

CleanCacheCommand::CleanCacheCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool CleanCacheCommand::execute( GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg )
{
    ModelLocator *pModel = ModelLocator::getInstance();
    QString strCacheFolder = pModel->getPreferences().getCacheFolder();
    QDir cCacheFolder(strCacheFolder);
    return cCacheFolder.removeRecursively();
}
