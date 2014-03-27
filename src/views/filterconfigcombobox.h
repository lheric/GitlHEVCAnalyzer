#ifndef FILTERCONFIGCOMBOBOX_H
#define FILTERCONFIGCOMBOBOX_H

#include <QtWidgets/QWidget>
#include "gitldef.h"
namespace Ui {
class FilterConfigComboBox;
}

class FilterConfigComboBox : public QWidget
{
    Q_OBJECT

public:
    explicit FilterConfigComboBox(const QStringList &rcList, int* piIndex, QString strTitle, QWidget *parent = 0);
    ~FilterConfigComboBox();

private slots:
    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::FilterConfigComboBox *ui;

    ADD_CLASS_FIELD(int*, piIndex, getIndex, setIndex)
};

#endif // FILTERCONFIGCOMBOBOX_H
