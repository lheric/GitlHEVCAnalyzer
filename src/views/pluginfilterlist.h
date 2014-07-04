#ifndef PLUGINFILTERLIST_H
#define PLUGINFILTERLIST_H
#include "gitlview.h"
#include "gitlupdateuievt.h"
#include <QListWidget>
#include <QStringList>

class PluginFilterList : public QListWidget, public GitlView
{
    Q_OBJECT
public:
    explicit PluginFilterList( QWidget *parent = 0 );
    virtual void onSequenceChanged(GitlUpdateUIEvt &rcEvt);

protected:
    void dropEvent(QDropEvent * event);
    QStringList xGetFilterOrder();

};

#endif // PLUGINFILTERLIST_H
