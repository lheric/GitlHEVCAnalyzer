#include "sequencelist.h"
#include "ui_sequencelist.h"
#include "model/common/comsequence.h"
#include "gitlivkcmdevt.h"
#include <QFileInfo>


SequenceList::SequenceList(QWidget *parent) :
    QListWidget(parent),
    ui(new Ui::SequenceList)
{
    ui->setupUi(this);

    setModualName("sequence_list");
    ///set listener
    listenToParams(QStringList()<<"sequences"<<"current_sequence",
                   MAKE_CALLBACK(SequenceList::onSequenceChanged) );

    ///
    addNoSquenceRadioBtn();
}

SequenceList::~SequenceList()
{
    delete ui;
}

void SequenceList::addNoSquenceRadioBtn()
{
    QListWidgetItem* pcItem = new QListWidgetItem();
    this->addItem(pcItem);
    SequenceListItem* seqRadioBtn = new SequenceListItem("No Sequence", m_cButtonGroup);
    seqRadioBtn->setChecked(true);
    seqRadioBtn->setYUVSelectorVisible(false);
    pcItem->setSizeHint(seqRadioBtn->sizeHint());
    setItemWidget(pcItem, seqRadioBtn);
}

void SequenceList::clearAllRadioBtn()
{
    while(this->count())
    {
        delete this->takeItem(0);
    }

}

void SequenceList::onSequenceChanged(GitlUpdateUIEvt &rcEvt)
{
    QVariant vValue;

    clearAllRadioBtn();
    vValue = rcEvt.getParameter("sequences");
    QVector<ComSequence*>* ppcSequences = (QVector<ComSequence*>*)vValue.value<void*>();
    vValue = rcEvt.getParameter("current_sequence");
    ComSequence* pcCurrentSequence = (ComSequence*)vValue.value<void*>();
    if(ppcSequences->size() != 0)
    {
        for(int i = 0; i < ppcSequences->size(); i++)
        {
            //
            QListWidgetItem* pcItem = new QListWidgetItem();
            this->addItem(pcItem);
            ComSequence* pcSequence = ppcSequences->at(i);
            QFileInfo cFileInfo(pcSequence->getFileName());
            SequenceListItem* seqRadioBtn = new SequenceListItem(cFileInfo.fileName(), m_cButtonGroup);
            seqRadioBtn->setSequence(pcSequence);
            seqRadioBtn->setChecked(pcCurrentSequence == pcSequence);
            pcItem->setSizeHint(seqRadioBtn->sizeHint());
            setItemWidget(pcItem, seqRadioBtn);

            connect(seqRadioBtn, SIGNAL(sequenceRadioButtonClicked(ComSequence*, QString, bool)),
                    this, SLOT(sequenceRadioButtonClicked(ComSequence*, QString, bool)));
            connect(seqRadioBtn, SIGNAL(yuvSelectionBoxChanged(ComSequence*,QString,bool)),
                    this, SLOT(yuvSelectionBoxChanged(ComSequence*,QString,bool)) );

        }
    }
    else
    {
        addNoSquenceRadioBtn();
    }


}

void SequenceList::sequenceRadioButtonClicked(ComSequence* pcSequence, QString strYUVFileName, bool bIs16Bit)
{
    GitlIvkCmdEvt cRequest("switch_sequence");
    cRequest.setParameter("command_name", "switch_sequence");
    cRequest.setParameter("sequence", QVariant::fromValue((void*)pcSequence));
    cRequest.setParameter("YUV_filename", strYUVFileName);
    cRequest.setParameter("is_16_bit", bIs16Bit);
    cRequest.dispatch();
}

void SequenceList::yuvSelectionBoxChanged(ComSequence* pcSequence, QString strYUVFileName, bool bIs16Bit)
{
    GitlIvkCmdEvt cRequest("switch_yuv");
    cRequest.setParameter("sequence", QVariant::fromValue((void*)pcSequence));
    cRequest.setParameter("YUV_filename", strYUVFileName);
    cRequest.setParameter("is_16_bit", bIs16Bit);
    cRequest.dispatch();
}
