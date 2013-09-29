#include "modifypreferencescommand.h"
#include "model/modellocator.h"
#include <QDir>
#include <QDebug>
ModifyPreferencesCommand::ModifyPreferencesCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool ModifyPreferencesCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    ModelLocator* pModel = ModelLocator::getInstance();
    if( rcInputArg.hasParameter("cache_path") )
    {
        QString strCacheFolder = rcInputArg.getParameter("cache_path").toString();
        QDir cCacheFolder(strCacheFolder);
        if( !cCacheFolder.exists() )
        {
            qWarning() << QString("%1 NOT existed!").arg(strCacheFolder);
            return false;
        }
        /// set to sub-folder 'cache'
        strCacheFolder = cCacheFolder.absoluteFilePath("cache");
        qDebug() << QString("Cache directory changed to %1...").arg(strCacheFolder);
        pModel->getPreferences().setCacheFolder(strCacheFolder);
    }

    return true;
}
