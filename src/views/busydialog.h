#ifndef BUSYDIALOG_H
#define BUSYDIALOG_H

#include <QDialog>
#include "gitlview.h"
namespace Ui {
class BusyDialog;
}

class BusyDialog : public QDialog, public GitlView
{
    Q_OBJECT
    
public:
    explicit BusyDialog(QWidget *parent = 0);
    ~BusyDialog();

public slots:
    void setHintText(QString& str);
    
private:
    Ui::BusyDialog *ui;
};

#endif // BUSYDIALOG_H
