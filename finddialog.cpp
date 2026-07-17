#include "finddialog.h"
#include "ui_finddialog.h"
#include "mainwindow.h"
#include <QTextDocument>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent, Qt::Tool),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);

    connect(ui->pushButtonFind,SIGNAL(clicked()),this,SLOT(sltPushFindClicked()));
}

FindDialog::~FindDialog()
{
    delete ui;
}

QString FindDialog::findText() const
{
    return ui->lineEdit->text();
}

void FindDialog::setFindText(const QString &text)
{
    ui->lineEdit->setText(text);
}

bool FindDialog::caseSensitive() const
{
    return ui->checkBoxCaseSensitive->isChecked();
}

void FindDialog::setCaseSensitive(bool on)
{
    ui->checkBoxCaseSensitive->setChecked(on);
}

bool FindDialog::findBackward() const
{
    return ui->radioButtonBackward->isChecked();
}

void FindDialog::setFindBackward(bool on)
{
    if (on)
    {
        ui->radioButtonBackward->setChecked(true);
    }
    else
    {
        ui->radioButtonForward->setChecked(true);
    }
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
