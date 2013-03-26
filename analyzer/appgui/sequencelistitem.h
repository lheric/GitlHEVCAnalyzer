#ifndef SEQUENCELISTITEM_H
#define SEQUENCELISTITEM_H

#include <QWidget>
#include <QButtonGroup>
#include "common/comsequence.h"
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
protected:
    virtual void mouseReleaseEvent ( QMouseEvent * e );


signals:
    void sequenceRadioButtonClicked(ComSequence*);
    ADD_CLASS_FIELD(ComSequence*, pcSequence, getSequence, setSequence)

    
private:
    Ui::SequenceListItem *ui;
};


#endif
