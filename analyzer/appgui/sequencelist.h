#ifndef SEQUENCELIST_H
#define SEQUENCELIST_H

#include <QWidget>
#include <QListWidget>
#include <QButtonGroup>
#include <QListWidgetItem>
#include "gitlview.h"
#include "common/comsequence.h"
#include "sequencelistitem.h"
namespace Ui {
class SequenceList;
}



class SequenceList : public QListWidget, public GitlView
{
    Q_OBJECT
    
public:
    explicit SequenceList(QWidget *parent = 0);
    void onUIUpdate(GitlUpdateUIEvt &rcEvt);
    ~SequenceList();

    void addNoSquenceRadioBtn();
    void clearAllRadioBtn();
    
private slots:
    void sequenceRadioButtonClicked(ComSequence* pcSequence, QString, bool);
    void yuvSelectionBoxChanged(ComSequence*, QString, bool);
private:
    Ui::SequenceList *ui;
    ADD_CLASS_FIELD_PRIVATE(QButtonGroup, cButtonGroup)
};

#endif // SEQUENCELIST_H
