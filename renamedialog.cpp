#include "renamedialog.h"
#include "ui_renamedialog.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

RenameDialog::RenameDialog(QString curName,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog)
{
    ui->setupUi(this);

    ui->lineEdit->setText(curName);
    ui->lineEdit->selectAll();

    connect(ui->pushButtonOk,SIGNAL(clicked()),this,SLOT(sltButtonOkClicked()));
    connect(ui->pushButtonCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

RenameDialog::RenameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog)
{
    ui->setupUi(this);

    connect(ui->pushButtonOk,SIGNAL(clicked()),this,SLOT(sltButtonOkClicked()));
    connect(ui->pushButtonCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

RenameDialog::~RenameDialog()
{
    delete ui;
}

void RenameDialog::sltButtonOkClicked()
{
    m_newName = ui->lineEdit->text();
    accept();
}
