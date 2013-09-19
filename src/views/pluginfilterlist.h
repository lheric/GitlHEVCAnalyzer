#ifndef PLUGINFILTERLIST_H
#define PLUGINFILTERLIST_H
#include "gitlview.h"
#include "gitlupdateuievt.h"
#include <QListWidget>

class PluginFilterList : public QListWidget, public GitlView
{
    Q_OBJECT
public:
    explicit PluginFilterList( QWidget *parent = 0 );
    virtual void onUIUpdate(GitlUpdateUIEvt &rcEvt);


};

#endif // PLUGINFILTERLIST_H
