#ifndef PLUGINFILTERLIST_H
#define PLUGINFILTERLIST_H
#include "gitlmodual.h"
#include <QListWidget>
#include <gitlcommandrespond.h>
class PluginFilterList : public QListWidget, public GitlModual
{
    Q_OBJECT
public:
    explicit PluginFilterList( QWidget *parent = 0 );
    virtual bool detonate( GitlEvent cEvt );

protected:
    void xRefreshListByRespond(const GitlCommandRespond &rcRespond);

signals:
    
public slots:
    
};

#endif // PLUGINFILTERLIST_H
