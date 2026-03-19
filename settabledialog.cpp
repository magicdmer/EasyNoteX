#include "settabledialog.h"
#include "ui_settabledialog.h"
#include <QIntValidator>

SetTableDialog::SetTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetTableDialog)
{
    ui->setupUi(this);
    ui->lineEditRow->setValidator(new QIntValidator(ui->lineEditRow));
    ui->lineEditCol->setValidator(new QIntValidator(ui->lineEditCol));
    ui->lineEditPercent->setValidator(new QIntValidator(ui->lineEditPercent));

    ui->lineEditRow->setText("3");
    ui->lineEditCol->setText("3");
    ui->lineEditPercent->setText("80");
}

SetTableDialog::~SetTableDialog()
{
    delete ui;
}

void SetTableDialog::on_pushButtonCreate_clicked()
{
    m_row = ui->lineEditRow->text().toInt();
    m_col = ui->lineEditCol->text().toInt();
    m_percent = ui->lineEditPercent->text().toInt();
    accept();
}

void SetTableDialog::on_pushButtonCancel_clicked()
{
    reject();
}
