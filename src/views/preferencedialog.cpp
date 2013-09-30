#include "preferencedialog.h"
#include "ui_preferencedialog.h"
#include "gitlivkcmdevt.h"
#include <QFileDialog>
PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    GitlIvkCmdEvt cEvt("query_pref");
    cEvt.dispatch();
}

void PreferenceDialog::onUIUpdate(GitlUpdateUIEvt &rcEvt)
{
    if( rcEvt.hasParameter("cache_path"))
    {
        ui->cacheFolderEdit->setText(rcEvt.getParameter("cache_path").toString());
    }
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}


void PreferenceDialog::showEvent(QShowEvent * event)
{
    GitlIvkCmdEvt cEvt("query_pref");
    cEvt.dispatch();
}

void PreferenceDialog::on_buttonBox_accepted()
{
    GitlIvkCmdEvt cEvt("modify_pref");
    cEvt.setParameter("cache_path", ui->cacheFolderEdit->text());
    cEvt.dispatch();
    this->hide();
}

void PreferenceDialog::on_buttonBox_rejected()
{
    this->hide();
}

void PreferenceDialog::on_cacheFolderBrowseBtn_clicked()
{
    QString strFolder=QFileDialog::getExistingDirectory(this,
                                          tr("Select Cache Location"),
                                          ui->cacheFolderEdit->text() );
    if(!strFolder.isEmpty())
        ui->cacheFolderEdit->setText(strFolder);
}
