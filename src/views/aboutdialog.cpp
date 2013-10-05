#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "model/common/comrom.h"
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    /// version number
    ui->versionLabel->setText(ANALYZER_VERSION);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
