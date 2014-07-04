#ifndef PLUGINFILTERITEM_H
#define PLUGINFILTERITEM_H

#include <QWidget>
#include <QString>
#include "gitldef.h"
#include "model/drawengine/abstractfilter.h"

namespace Ui {
class PluginFilterItem;
}

class PluginFilterItem : public QWidget
{
    Q_OBJECT
    
public:
    explicit PluginFilterItem(const QString& strFilterName,
                              const bool bEnable,
                              QWidget *parent = 0 );
    ~PluginFilterItem();

    ADD_CLASS_FIELD_NOSETTER(QString, strFilterName, getFilterName)

private slots:
    void on_enableCheckBox_clicked();

    void on_configBtn_clicked();

private:
    Ui::PluginFilterItem *ui;

};

#endif // PLUGINFILTERITEM_H
