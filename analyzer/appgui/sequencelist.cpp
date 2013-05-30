#include "sequencelist.h"
#include "ui_sequencelist.h"
#include "events/eventnames.h"
#include "common/comsequence.h"
#include "commandrequest.h"
#include "io/analyzermsgsender.h"
#include <QFileInfo>

SequenceList::SequenceList(QWidget *parent) :
    QListWidget(parent),
    ui(new Ui::SequenceList)
{
    ui->setupUi(this);
    subscribeToEvtByName(g_strSquencesListChanged);
}

SequenceList::~SequenceList()
{
    delete ui;
}

void SequenceList::addNoSquenceRadioBtn()
{
    SequenceListItem* noSeqRadioBtn = new SequenceListItem("No Sequence", m_cButtonGroup);
    this->layout()->addWidget(noSeqRadioBtn);
}

void SequenceList::clearAllRadioBtn()
{
    while(this->count())
    {
        delete this->takeItem(0);
    }

}

bool SequenceList::detonate( GitlEvent cEvt )
{

    clearAllRadioBtn();

    QVariant vValue;
    if(cEvt.getEvtData().hasParameter("sequences"))
    {
        vValue = cEvt.getEvtData().getParameter("sequences");
        QVector<ComSequence*>* ppcSequences = (QVector<ComSequence*>*)vValue.value<void*>();
        vValue = cEvt.getEvtData().getParameter("current_sequence");
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

                connect(seqRadioBtn, SIGNAL(sequenceRadioButtonClicked(ComSequence*)),
                        this, SLOT(sequenceRadioButtonClicked(ComSequence*)));                


            }
            return true;
        }
    }

    addNoSquenceRadioBtn();


    return true;
}

void SequenceList::sequenceRadioButtonClicked(ComSequence* pcSequence)
{
    GitlEvent cEvt( g_strCmdSentEvent  );
    CommandRequest cRequest;
    cRequest.setParameter("command_name", "switch_sequence");
    cRequest.setParameter("sequence", QVariant::fromValue((void*)pcSequence));
    cEvt.getEvtData().setParameter("request", QVariant::fromValue(cRequest));
    dispatchEvt(cEvt);
}
