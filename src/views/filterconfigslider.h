#ifndef FILTERCONFIGSLIDER_H
#define FILTERCONFIGSLIDER_H

#include <QtWidgets/QWidget>
#include "gitldef.h"

namespace Ui {
class FilterConfigSlider;
}

class FilterConfigSlider : public QWidget
{
    Q_OBJECT

public:
    explicit FilterConfigSlider(const QString& rLabel, double dMin, double dMax, double* pdValue, QWidget *parent = 0);
    ~FilterConfigSlider();

private slots:
    void on_slider_valueChanged(int value);

protected:
    virtual void showEvent(QShowEvent * event);

private:
    Ui::FilterConfigSlider *ui;

    ADD_CLASS_FIELD(double, dMax, getMax, setMax)
    ADD_CLASS_FIELD(double, dMin, getMin, setMin)
    ADD_CLASS_FIELD(double*, pdValue, getValue, setValue)
};

#endif // FILTERCONFIGSLIDER_H
