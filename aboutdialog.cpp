#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QApplication>
#include <QTabBar>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);
    ui->labelVersion->setText(tr("版本：%1").arg(qApp->applicationVersion()));

#ifndef ENABLE_SPONSOR
    const int sponsorIndex = ui->tabWidget->indexOf(ui->tabSponsor);
    if (sponsorIndex >= 0)
    {
        ui->tabWidget->removeTab(sponsorIndex);
        delete ui->tabSponsor;
    }
    ui->tabWidget->tabBar()->hide();
    ui->tabWidget->setGeometry(0, 0, 420, 270);
    setFixedSize(420, 270);
#endif
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
