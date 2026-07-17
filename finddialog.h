#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

    QString findText() const;
    void setFindText(const QString &text);
    bool caseSensitive() const;
    void setCaseSensitive(bool on);
    bool findBackward() const;
    void setFindBackward(bool on);

public slots:
    void sltPushFindClicked();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
