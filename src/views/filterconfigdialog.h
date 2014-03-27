#ifndef FILTERCONFIGDIALOG_H
#define FILTERCONFIGDIALOG_H


#include <QList>
#include <QtWidgets/QDialog>
#include "filterconfigslider.h"
#include "filterconfigcheckbox.h"
#include "filterconfigradios.h"
#include "filterconfigcombobox.h"
#include "gitlcolorpicker.h"

namespace Ui {
class FilterConfigDialog;
}

class FilterConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterConfigDialog(QWidget *parent = 0);
    ~FilterConfigDialog();

    void addSlider(const QString& rLabel, double dMin, double dMax, double* pdValue);
    void addCheckbox(const QString& rLabel, const QString& rDiscription, bool* pbValue);
    void addColorPicker(const QString& rLabel, QColor* pcColor);
    void addRadioButtons(const QStringList& rList, int* piIndex);
    void addComboBox(const QStringList& rList, int* piIndex, const QString& strTitle);
private:
    Ui::FilterConfigDialog *ui;
    QList<QWidget*> m_apcComponents;

};

#endif // FILTERCONFIGDIALOG_H
