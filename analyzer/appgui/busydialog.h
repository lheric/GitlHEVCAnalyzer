#ifndef BUSYDIALOG_H
#define BUSYDIALOG_H

#include <QDialog>
#include <gitlmodual.h>
namespace Ui {
class BusyDialog;
}

class BusyDialog : public QDialog, public GitlModual
{
    Q_OBJECT
    
public:
    explicit BusyDialog(QWidget *parent = 0);
    ~BusyDialog();

public slots:
    void setHintText(QString& str);
    virtual bool detonate(GitlEvent cEvt);
    
private:
    Ui::BusyDialog *ui;
};

#endif // BUSYDIALOG_H
