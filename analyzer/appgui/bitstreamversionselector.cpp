#include "bitstreamversionselector.h"
#include "ui_bitstreamversionselector.h"

BitstreamVersionSelector::BitstreamVersionSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BitstreamVersionSelector)
{
    ui->setupUi(this);
}

BitstreamVersionSelector::~BitstreamVersionSelector()
{
    delete ui;
}

void BitstreamVersionSelector::on_version40_clicked()
{
    m_iBitstreamVersion = 40;
}


void BitstreamVersionSelector::on_buttonBox_accepted()
{
    if(ui->version40->isChecked())
    {
        m_iBitstreamVersion = 40;
    }
    else if(ui->version52->isChecked() )
    {
        m_iBitstreamVersion = 52;
    }
}

void BitstreamVersionSelector::on_version52_clicked()
{
    m_iBitstreamVersion = 52;
}


