#ifndef PLUGINFILTERLIST_H
#define PLUGINFILTERLIST_H
#include "gitlmodual.h"
#include <QListWidget>

class PluginFilterList : public QListWidget, public GitlModual
{
    Q_OBJECT
public:
    explicit PluginFilterList(QWidget *parent = 0);
    virtual bool detonate( GitlEvent cEvt );
signals:
    
public slots:
    
};

#endif // PLUGINFILTERLIST_H
