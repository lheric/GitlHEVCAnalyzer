#include "sequencelistitem.h"
#include "ui_sequencelistitem.h"
#include <QDebug>
SequenceListItem::SequenceListItem(const QString& strText, QButtonGroup& rcGroup, QWidget *parent) :
    QWidget(parent),
    m_pcSequence(NULL),
    ui(new Ui::SequenceListItem)
{
    ui->setupUi(this);
    ui->sequenceNameLabel->setText(strText);
    rcGroup.addButton(ui->radioButton);

    ///init box
    ui->yuvSelectionBox->insertItem(0, "Reconstructed", "decoder_yuv.yuv");
    ui->yuvSelectionBox->insertItem(1, "Residual",      "resi_yuv.yuv");
    ui->yuvSelectionBox->insertItem(2, "Predicted",     "pred_yuv.yuv");

}

SequenceListItem::~SequenceListItem()
{
    delete ui;
}


void SequenceListItem::mouseReleaseEvent ( QMouseEvent * e )
{
    int index = ui->yuvSelectionBox->currentIndex();

    bool bIs16Bit = (index == 1);
    ui->radioButton->setChecked(true);
    QString strItemString = ui->yuvSelectionBox->itemData(index).toString();
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
        bool bIs16Bit = (index == 1);
        QString strItemString = ui->yuvSelectionBox->itemData(index).toString();
        emit yuvSelectionBoxChanged(m_pcSequence, strItemString, bIs16Bit);
    }
}
