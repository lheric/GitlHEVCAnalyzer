#ifndef PREFERENCES_H
#define PREFERENCES_H
#include "gitldef.h"
#include <QString>
#include <QSettings>

struct PreferencesOption
{
    QString strKey;
    QString strValue;

};


class Preferences
{
public:
    Preferences();


    void setCacheFolder(const QString& strCacheFolder);
    void setThemeName(const QString& strThemeName);

protected:
    void xCreateIfNotExist(QString strPath);

    ADD_CLASS_FIELD_NOSETTER(QString, strCacheFolder, getCacheFolder)       /// for temp decoded sequences
    ADD_CLASS_FIELD_NOSETTER(QString, strThemeName, getThemeName)           /// theme name

    ADD_CLASS_FIELD_PRIVATE(QSettings, cSettings)    /// for save onto disk


};

#endif // PREFERENCES_H
