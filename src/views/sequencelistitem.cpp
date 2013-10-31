#include "sequencelistitem.h"
#include "ui_sequencelistitem.h"
#include <QDebug>

struct YUV_SELECTION
{
    int     iComboIndex;
    char*   phLableName;
    YUVRole eRole;
};

static YUV_SELECTION pasSelections[] =
{
    { 0,    "Reconstructed",    YUV_RECONSTRUCTED },
    { 1,    "Residual",         YUV_RESIDUAL      },
    { -1,   NULL,               YUV_NONE          },       /// end mark
};

SequenceListItem::SequenceListItem(const QString& strText, QButtonGroup& rcGroup, QWidget *parent) :
    QWidget(parent),
    m_pcSequence(NULL),
    ui(new Ui::SequenceListItem)
{
    ui->setupUi(this);
    ui->sequenceNameLabel->setText(strText);
    rcGroup.addButton(ui->radioButton);

    ///init box
    YUV_SELECTION* pSelection = pasSelections;
    while( pSelection->iComboIndex != -1 )
    {
        ui->yuvSelectionBox->insertItem(pSelection->iComboIndex, pSelection->phLableName);
        pSelection++;
    }



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

void SequenceListItem::setYUVSelectorVisible(bool bVisible)
{
    this->ui->yuvSelectionBox->setVisible(bVisible);
}

void SequenceListItem::on_yuvSelectionBox_currentIndexChanged(int index)
{
    if(ui->radioButton->isChecked())
    {
        m_pcSequence->setYUVRole(pasSelections[index].eRole);
        emit yuvSelectionBoxChanged(m_pcSequence);
    }
}
