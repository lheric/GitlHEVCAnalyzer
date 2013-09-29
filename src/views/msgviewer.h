#ifndef MSGVIEWER_H
#define MSGVIEWER_H

#include <QTextBrowser>
#include <QMessageBox>
#include <QColor>
#include "gitlview.h"
#include "gitlevent.h"
#include "gitldef.h"

class MsgViewer : public QTextBrowser, public GitlView
{
public:
    MsgViewer(QWidget *parent = 0);
    virtual void onUIUpdate(GitlUpdateUIEvt &rcEvt);

    ADD_CLASS_FIELD_PRIVATE(QMessageBox, cWarningBox)
    ADD_CLASS_FIELD_PRIVATE(QColor, cDefalutTextColor)
};

#endif // MSGVIEWER_H
