#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class RenameDialog;
}

class RenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameDialog(QString curName="", QWidget *parent = 0);
    RenameDialog(QWidget *parent = 0);
    ~RenameDialog();

public slots:
    void sltButtonOkClicked();

public:
    QString m_newName;

private:
    Ui::RenameDialog *ui;
};

#endif // RENAMEDIALOG_H
