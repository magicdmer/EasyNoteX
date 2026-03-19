#include "finddialog.h"
#include "ui_finddialog.h"
#include "mainwindow.h"
#include <QTextDocument>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);

    connect(ui->pushButtonFind,SIGNAL(clicked()),this,SLOT(sltPushFindClicked()));
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::sltPushFindClicked()
{
    ui->labelError->clear();

    MainWindow* parent = (MainWindow*)this->parent();
    QString text = ui->lineEdit->text();

    QTextDocument::FindFlags flags;
    if (ui->checkBoxCaseSensitive->isChecked())
    {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (ui->radioButtonBackward->isChecked())
    {
        flags |= QTextDocument::FindBackward;
    }

    if (!parent->find(text,flags))
    {
        ui->labelError->setText(tr("未找到"));
    }
}
