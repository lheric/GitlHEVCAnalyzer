#include "sequencelistitem.h"
#include "ui_sequencelistitem.h"

SequenceListItem::SequenceListItem(const QString& strText, QButtonGroup& rcGroup, QWidget *parent) :
    QWidget(parent),
    m_pcSequence(NULL),
    ui(new Ui::SequenceListItem)
{
    ui->setupUi(this);
    ui->sequenceNameLabel->setText(strText);
    rcGroup.addButton(ui->radioButton);

}

SequenceListItem::~SequenceListItem()
{
    delete ui;
}


void SequenceListItem::mouseReleaseEvent ( QMouseEvent * e )
{
    ui->radioButton->setChecked(true);
    emit sequenceRadioButtonClicked(m_pcSequence);
}

void SequenceListItem::setChecked(bool bCheck)
{
    this->ui->radioButton->setChecked(bCheck);
}
