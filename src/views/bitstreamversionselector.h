#ifndef BITSTREAMVERSIONSELECTOR_H
#define BITSTREAMVERSIONSELECTOR_H

#include <QDialog>
#include "gitldef.h"

namespace Ui {
    class BitstreamVersionSelector;
}

class BitstreamVersionSelector : public QDialog
{
    Q_OBJECT

public:
    explicit BitstreamVersionSelector(QWidget *parent = 0);
    ~BitstreamVersionSelector();

    ADD_CLASS_FIELD(int, iBitstreamVersion, getBitstreamVersion, setBitstreamVersion)

    private slots:
    void on_version40_clicked();

    void on_buttonBox_accepted();

    void on_version52_clicked();

    void on_version100_clicked();
    void on_version120_clicked();

protected:
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent * event);
    void xSetDecoderVersion();

private:
    Ui::BitstreamVersionSelector *ui;
};

#endif // BITSTREAMVERSIONSELECTOR_H
