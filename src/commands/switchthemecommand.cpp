#include "switchthemecommand.h"
#include <QFile>
struct ThemeItem
{
    QString strThemeName;
    QString strThemeFile;
};

static ThemeItem themeItemList[] =
{
    {"default", ":/UI/UI/default.qss"},
    {"dark",    ":/UI/UI/style_sheet.qss"},
    {"",        ""}   ///end mark
};

SwitchThemeCommand::SwitchThemeCommand(QObject *parent) :
    GitlAbstractCommand(parent)
{
}

bool SwitchThemeCommand::execute(GitlCommandParameter &rcInputArg, GitlCommandParameter &rcOutputArg)
{
    QString strThemeName = rcInputArg.getParameter ("theme_name").toString();
    QString strThemeFile;
    const ThemeItem* pstrThemeItem = themeItemList;
    do
    {
        if( pstrThemeItem->strThemeName == strThemeName )
        {
            strThemeFile = pstrThemeItem->strThemeFile;
            break;
        }
        pstrThemeItem++;
    }
    while(!pstrThemeItem->strThemeName.isEmpty());

    if(strThemeFile.isEmpty())
    {
        qWarning() << QString("Theme '%1' NOT found!").arg(strThemeName);
        return false;
    }
    QFile cFile(strThemeFile);
    if( !cFile.open(QFile::ReadOnly) )
    {
        qWarning() << QString("Unable to open '%1'!").arg(strThemeFile);
        return false;
    }
    QString strStyleSheet = QLatin1String(cFile.readAll());
    cFile.close();

    rcOutputArg.setParameter("theme_stylesheet", strStyleSheet);

    return true;
}
