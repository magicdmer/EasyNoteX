#ifndef SETTABLEDIALOG_H
#define SETTABLEDIALOG_H

#include <QDialog>

namespace Ui {
class SetTableDialog;
}

class SetTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetTableDialog(QWidget *parent = nullptr);
    ~SetTableDialog();

private slots:
    void on_pushButtonCreate_clicked();
    void on_pushButtonCancel_clicked();

public:
    int m_row;
    int m_col;
    int m_percent;

private:
    Ui::SetTableDialog *ui;
};

#endif // SETTABLEDIALOG_H
