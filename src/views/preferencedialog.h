#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include "gitlview.h"
namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog, public GitlView
{
    Q_OBJECT
    
public:
    explicit PreferenceDialog(QWidget *parent = 0);
    ~PreferenceDialog();
    
    virtual void onUIUpdate(GitlUpdateUIEvt &rcEvt);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::PreferenceDialog *ui;
};

#endif // PREFERENCEDIALOG_H
