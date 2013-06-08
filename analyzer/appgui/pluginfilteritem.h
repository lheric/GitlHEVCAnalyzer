#ifndef PLUGINFILTERITEM_H
#define PLUGINFILTERITEM_H

#include <QWidget>
#include "gitldef.h"
#include "drawengine/abstractfilter.h"
#include "gitlmodual.h"

namespace Ui {
class PluginFilterItem;
}

class PluginFilterItem : public QWidget, public GitlModual
{
    Q_OBJECT
    
public:
    explicit PluginFilterItem(AbstractFilter* pcFilter, QWidget *parent = 0 );
    ~PluginFilterItem();

    ADD_CLASS_FIELD_PRIVATE(AbstractFilter*, pcFilter)

private slots:
    void on_enableCheckBox_clicked();

void on_configBtn_clicked();

void on_upBtn_clicked();

private:
    Ui::PluginFilterItem *ui;

    //ADD_CLASS_FIELD(AbstractFilter*, pFilter, getFilter, setFilter)
};

#endif // PLUGINFILTERITEM_H
