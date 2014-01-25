#include "filterconfigradios.h"
#include "ui_filterconfigradios.h"

FilterConfigRadios::FilterConfigRadios(const QStringList &rcList, int* piIndex, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterConfigRadios)
{
    ui->setupUi(this);
    m_piIndex = piIndex;
    for(int i = 0; i < rcList.size(); i++)
    {
        QRadioButton* pcBtn = new QRadioButton(rcList.at(i));
        connect(pcBtn, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        m_Btns.push_back(pcBtn);
        pcBtn->setChecked(i==*piIndex);
        ui->content->addWidget(pcBtn);
    }
}


FilterConfigRadios::~FilterConfigRadios()
{
    delete ui;
    foreach(QRadioButton* pcRadioBtn, m_Btns)
        delete pcRadioBtn;
}

void FilterConfigRadios::radioToggled(bool checked)
{
    for(int i = 0; i < m_Btns.size(); i++)
        if(m_Btns.at(i)->isChecked())
        {
            *m_piIndex = i;
            return;
        }
    return;
}

void FilterConfigRadios::showEvent(QShowEvent *event)
{
    if(m_piIndex == NULL || *m_piIndex >= m_Btns.size())
        return;
    m_Btns.at(*m_piIndex)->setChecked(true);
}
