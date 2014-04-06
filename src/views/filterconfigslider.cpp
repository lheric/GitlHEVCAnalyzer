#include "filterconfigslider.h"
#include "ui_filterconfigslider.h"

FilterConfigSlider::FilterConfigSlider(const QString& rLabel, double dMin, double dMax, double* pdValue, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterConfigSlider)
{
    ui->setupUi(this);
    m_dMax = dMax;
    m_dMin = dMin;
    m_pdValue = pdValue;
    ui->sliderLabel->setText(rLabel);
}

FilterConfigSlider::~FilterConfigSlider()
{
    delete ui;
}



void FilterConfigSlider::on_slider_valueChanged(int value)
{
    if(m_pdValue == NULL)
        return;

    double dSliderMax = ui->slider->maximum();
    double dSliderMin = ui->slider->minimum();
    *m_pdValue = (value-dSliderMin)/(dSliderMax-dSliderMin)*(m_dMax-m_dMin)+m_dMin;
    ui->sliderValue->setText(QString::number(*m_pdValue, 'f', 3));
}

void FilterConfigSlider::showEvent(QShowEvent *event)
{
    if(m_pdValue == NULL)
        return;
    double dSliderMax = ui->slider->maximum();
    double dSliderMin = ui->slider->minimum();
    double dClippedValue = VALUE_CLIP(m_dMin,m_dMax,*m_pdValue);
    int iPos = qRound((dClippedValue-m_dMin)/(m_dMax-m_dMin)*(dSliderMax-dSliderMin)+dSliderMin);
    ui->slider->setValue(iPos);

}
