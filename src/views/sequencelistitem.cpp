#include "sequencelistitem.h"
#include "ui_sequencelistitem.h"
#include <QDebug>

struct YUV_SELECTION
{
    int   iComboIndex;
    char* phLableName;
    char* phFileName;
    bool  bIs16Bit;
};

static YUV_SELECTION pasSelections[] =
{
    { 0, "Reconstructed", "decoder_yuv.yuv", false },
    { 1, "Residual",      "resi_yuv.yuv",    true  },
    //{2, "Predicted",     "pred_yuv.yuv",    false},
    { -1, NULL,            NULL,             false }
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
    int index = ui->yuvSelectionBox->currentIndex();

    bool bIs16Bit = pasSelections[index].bIs16Bit;
    QString strItemString = pasSelections[index].phFileName;
    ui->radioButton->setChecked(true);

    emit sequenceRadioButtonClicked(m_pcSequence, strItemString, bIs16Bit);
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
        bool bIs16Bit = pasSelections[index].bIs16Bit;
        QString strItemString = pasSelections[index].phFileName;
        emit yuvSelectionBoxChanged(m_pcSequence, strItemString, bIs16Bit);
    }
}
