#ifndef SEQUENCELIST_H
#define SEQUENCELIST_H

#include <QWidget>
#include <QListWidget>
#include <QButtonGroup>
#include <QListWidgetItem>
#include "gitlmodual.h"
#include "common/comsequence.h"
#include "sequencelistitem.h"
namespace Ui {
class SequenceList;
}



class SequenceList : public QListWidget, public GitlModual
{
    Q_OBJECT
    
public:
    explicit SequenceList(QWidget *parent = 0);
    bool detonate( GitlEvent cEvt );
    ~SequenceList();

    void addNoSquenceRadioBtn();
    void clearAllRadioBtn();
    
private slots:
    void sequenceRadioButtonClicked(ComSequence* pcSequence);

private:
    Ui::SequenceList *ui;
    ADD_CLASS_FIELD_PRIVATE(QButtonGroup, cButtonGroup)
};

#endif // SEQUENCELIST_H
