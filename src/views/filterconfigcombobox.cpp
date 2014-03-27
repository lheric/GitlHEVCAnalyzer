#include "filterconfigcombobox.h"
#include "ui_filterconfigcombobox.h"

FilterConfigComboBox::FilterConfigComboBox(const QStringList &rcList, int* piIndex, QString strTitle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterConfigComboBox)
{
    ui->setupUi(this);
    m_piIndex = piIndex;
    ui->comboBox->addItems(rcList);
    ui->label->setText(strTitle);
}

FilterConfigComboBox::~FilterConfigComboBox()
{
    delete ui;
}

void FilterConfigComboBox::on_comboBox_currentIndexChanged(int index)
{
    *m_piIndex = index;
}
