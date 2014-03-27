#include "filterconfigdialog.h"
#include "ui_filterconfigdialog.h"

FilterConfigDialog::FilterConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterConfigDialog)
{
    ui->setupUi(this);
}

FilterConfigDialog::~FilterConfigDialog()
{
    foreach(QWidget* pcWidget, m_apcComponents)
    {
        delete pcWidget;    /// TODO BUG Memory Leak non-virtual destructor
    }
    m_apcComponents.clear();
    delete ui;
}

void FilterConfigDialog::addSlider(const QString& rLabel, double dMin, double dMax, double* pdValue)
{
    FilterConfigSlider* pcSlider = new FilterConfigSlider(rLabel, dMin, dMax, pdValue);
    m_apcComponents.push_back(pcSlider);
    ui->contentLayout->addWidget(pcSlider);
}

void FilterConfigDialog::addCheckbox(const QString& rLabel, const QString& rDiscription, bool* pbValue)
{
    FilterConfigCheckBox* pcCheckbox = new FilterConfigCheckBox(rLabel, rDiscription, pbValue);
    m_apcComponents.push_back(pcCheckbox);
    ui->contentLayout->addWidget(pcCheckbox);
}

void FilterConfigDialog::addColorPicker(const QString &rLabel, QColor *pcColor)
{
    GitlColorPicker* pcPicker = new GitlColorPicker(rLabel, pcColor);
    m_apcComponents.push_back(pcPicker);
    ui->contentLayout->addWidget(pcPicker);
}

void FilterConfigDialog::addRadioButtons(const QStringList& rList, int* piIndex)
{
    FilterConfigRadios* pcRadios = new FilterConfigRadios(rList, piIndex);
    m_apcComponents.push_back(pcRadios);
    ui->contentLayout->addWidget(pcRadios);
}

void FilterConfigDialog::addComboBox(const QStringList& rList, int* piIndex, const QString& strTitle)
{
    FilterConfigComboBox* pcComboBox = new FilterConfigComboBox(rList, piIndex, strTitle);
    m_apcComponents.push_back(pcComboBox);
    ui->contentLayout->addWidget(pcComboBox);
}
