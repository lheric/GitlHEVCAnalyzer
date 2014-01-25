#include "gitlcolorpicker.h"
#include "ui_gitlcolorpicker.h"

GitlColorPicker::GitlColorPicker(const QString &rLabel, QColor *pcColor, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GitlColorPicker)
{
    ui->setupUi(this);
    ui->label->setText(rLabel);
    m_pcColor = pcColor;
}

GitlColorPicker::~GitlColorPicker()
{
    delete ui;
}

void GitlColorPicker::setLabel(const QString &strLabel)
{
    ui->label->setText(strLabel);
}

QString GitlColorPicker::getLabel() const
{
    return ui->label->text();
}

void GitlColorPicker::on_selectColorBtn_clicked()
{
    if(m_pcColor==NULL)
        return;
    m_cColorDialog.setCurrentColor(*m_pcColor);
    m_cColorDialog.exec();
    *m_pcColor = m_cColorDialog.selectedColor();
    ui->selectColorBtn->setStyleSheet(QString("background-color: %1").arg(m_pcColor->name()));
}

void GitlColorPicker::showEvent(QShowEvent *event)
{
    if(m_pcColor!=NULL && this->isVisible())
        ui->selectColorBtn->setStyleSheet(QString("background-color: %1").arg(m_pcColor->name()));
}

