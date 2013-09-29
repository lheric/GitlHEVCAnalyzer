#ifndef PREFERENCES_H
#define PREFERENCES_H
#include "gitldef.h"
#include <QString>
#include <QSettings>




class Preferences
{
public:
    Preferences();


    void setCacheFolder(const QString& strCacheFolder);
    void setDecoderFolder(const QString& strDecoderFolder);

    ADD_CLASS_FIELD_NOSETTER(QString, strCacheFolder, getCacheFolder)          /// for temp decoded sequences
    ADD_CLASS_FIELD_NOSETTER(QString, strDecoderFolder, getDecoderFolder)    /// decoder location

    ADD_CLASS_FIELD_PRIVATE(QSettings, cSettings)    /// for save onto disk
};

#endif // PREFERENCES_H
