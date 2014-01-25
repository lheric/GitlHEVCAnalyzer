#include "filterconfigcheckbox.h"
#include "ui_filterconfigcheckbox.h"

FilterConfigCheckBox::FilterConfigCheckBox(const QString& rcLabel, const QString& rcDiscription, bool* pbValue, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterConfigCheckBox)
{
    ui->setupUi(this);
    m_pbChecked = pbValue;
    ui->checkBox->setText(rcLabel);
    ui->discriptionLabel->setText(rcDiscription);
}

FilterConfigCheckBox::~FilterConfigCheckBox()
{
    delete ui;
}

void FilterConfigCheckBox::showEvent(QShowEvent *event)
{
    if(m_pbChecked!=NULL && this->isVisible())
        ui->checkBox->setChecked(*m_pbChecked);
}

void FilterConfigCheckBox::setDiscription(const QString &rcDiscription)
{
    ui->discriptionLabel->setText(rcDiscription);
}

QString FilterConfigCheckBox::getDiscription()
{
    return ui->discriptionLabel->text();
}

void FilterConfigCheckBox::setLabel(const QString& rcLabel)
{
    ui->checkBox->setText(rcLabel);
}

QString FilterConfigCheckBox::getLabel()
{
    return ui->checkBox->text();
}


void FilterConfigCheckBox::on_checkBox_toggled(bool checked)
{
    if(m_pbChecked == NULL)
        return;
    *m_pbChecked = checked;

}
