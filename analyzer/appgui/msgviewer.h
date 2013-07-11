#ifndef MSGVIEWER_H
#define MSGVIEWER_H

#include <QTextBrowser>
#include <QMessageBox>
#include <gitlmodual.h>
#include <gitlevent.h>
#include <gitldef.h>

class MsgViewer : public QTextBrowser, public GitlModual
{
public:
    MsgViewer(QWidget *parent = 0);
    virtual bool detonate( GitlEvent& cEvt );

    ADD_CLASS_FIELD_PRIVATE(QMessageBox, cWarningBox)
};

#endif // MSGVIEWER_H
