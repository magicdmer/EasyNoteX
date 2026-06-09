#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QColor>
#include <QFont>

class QToolButton;

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
    void sltPickDefaultFont();
    void sltPickDefaultPen();
    void sltPickDefaultPaper();

public:
    int m_closeToTray;
    int m_minToTray;
    int m_escToTray;
    int m_tabWidth;
    QString m_shortcut;
    int m_sort_type;

    QFont m_defaultFont;
    QColor m_defaultPen;
    QColor m_defaultPaper;

private:
    void updateColorButton(QToolButton* button, const QColor& color);

    Ui::SetDialog *ui;
    QSettings* m_setting;
};

#endif // SETDIALOG_H
