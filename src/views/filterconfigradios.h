#ifndef FILTERCONFIGRADIOS_H
#define FILTERCONFIGRADIOS_H

#include <QWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include <QList>
#include "gitldef.h"

namespace Ui {
class FilterConfigRadios;
}

class FilterConfigRadios : public QWidget
{
    Q_OBJECT

public:
    explicit FilterConfigRadios(const QStringList& rcList, int* piIndex, QWidget *parent = 0);
    ~FilterConfigRadios();
public slots:
    void radioToggled(bool checked);
protected:
    void showEvent(QShowEvent *event);
private:
    Ui::FilterConfigRadios *ui;
    QButtonGroup m_cGroup;
    QList<QRadioButton*> m_Btns;

    ADD_CLASS_FIELD(int*, piIndex, getIndex, setIndex)
};

#endif // FILTERCONFIGRADIOS_H
