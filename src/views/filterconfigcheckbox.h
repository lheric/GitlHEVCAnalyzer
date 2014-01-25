#ifndef FILTERCONFIGCHECKBOX_H
#define FILTERCONFIGCHECKBOX_H

#include <QtWidgets/QWidget>
#include "gitldef.h"
namespace Ui {
class FilterConfigCheckBox;
}

class FilterConfigCheckBox : public QWidget
{
    Q_OBJECT

public:
    explicit FilterConfigCheckBox(const QString& rcLabel, const QString& rcDiscription, bool* pbValue, QWidget *parent = 0);
    ~FilterConfigCheckBox();
    virtual void showEvent(QShowEvent * event);

    void setDiscription(const QString& rcDiscription);
    QString getDiscription();

    void setLabel(const QString& rcLabel);
    QString getLabel();




private slots:
    void on_checkBox_toggled(bool checked);

private:
    Ui::FilterConfigCheckBox *ui;

    ADD_CLASS_FIELD(bool*, pbChecked, getChecked, setChecked)
};

#endif // FILTERCONFIGCHECKBOX_H
