#ifndef GITLCOLORPICKER_H
#define GITLCOLORPICKER_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QColorDialog>
#include "gitldef.h"

namespace Ui {
class GitlColorPicker;
}

class GitlColorPicker : public QWidget
{
    Q_OBJECT

public:
    explicit GitlColorPicker(const QString& rLabel, QColor* pcColor, QWidget *parent = 0);
    ~GitlColorPicker();

    void setLabel(const QString& strLabel );
    QString getLabel() const;

private slots:
    void on_selectColorBtn_clicked();

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::GitlColorPicker *ui;

    ADD_CLASS_FIELD_PRIVATE(QColorDialog, cColorDialog)
    ADD_CLASS_FIELD(QColor*, pcColor, getColor, setColor)
};

#endif // GITLCOLORPICKER_H
