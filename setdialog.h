#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SetDialog;
}

class SetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetDialog(QWidget *parent = nullptr);
    ~SetDialog();

public slots:
    void sltButtonOkClicked();

public:
    int m_closeToTray;
    int m_minToTray;
    int m_escToTray;
    int m_tabWidth;
    QString m_shortcut;
    int m_sort_type;

private:
    Ui::SetDialog *ui;
    QSettings* m_setting;
};

#endif // SETDIALOG_H
