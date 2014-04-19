#ifndef SEQUENCELISTITEM_H
#define SEQUENCELISTITEM_H

#include <QWidget>
#include <QButtonGroup>
#include "model/common/comsequence.h"
namespace Ui {
class SequenceListItem;
}

class SequenceListItem : public QWidget
{
    Q_OBJECT
    
public:
    explicit SequenceListItem(const QString& strText, QButtonGroup& rcGroup, QWidget *parent = 0);
    ~SequenceListItem();
    void setChecked(bool bCheck);
    void setYUVSelectorStatus(YUVRole eRole);
    void setYUVSelectorVisible(bool bVisible);
    void setCloseBtnVisible(bool bVisible);
protected:
    virtual void mouseReleaseEvent ( QMouseEvent * e );


signals:
    void sequenceRadioButtonClicked(ComSequence*);
    void yuvSelectionBoxChanged(ComSequence*, YUVRole eRole);
    void closeSequenceButtonClicked(ComSequence*);

    ADD_CLASS_FIELD(ComSequence*, pcSequence, getSequence, setSequence)

    
private slots:

    void on_yuvSelectionBox_currentIndexChanged(int index);

    void on_closeBtn_clicked();

private:
    Ui::SequenceListItem *ui;
};


#endif
