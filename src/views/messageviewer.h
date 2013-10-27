#ifndef MESSAGEVIEWER_H
#define MESSAGEVIEWER_H

#include <QWidget>
#include <QColor>
#include <QMessageBox>
#include "gitlview.h"

namespace Ui {
class MessageViewer;
}

class MessageViewer : public QWidget, GitlView
{
    Q_OBJECT
    
public:
    explicit MessageViewer(QWidget *parent = 0);
    ~MessageViewer();

    ///
    void onMessageArrived(GitlUpdateUIEvt &rcEvt);
    
private slots:
    void on_clearMsgBtn_clicked();

private:
    Ui::MessageViewer *ui;

    ADD_CLASS_FIELD_PRIVATE(QMessageBox, cWarningBox)
    ADD_CLASS_FIELD_PRIVATE(QColor, cDefalutTextColor)
};

#endif // MESSAGEVIEWER_H
