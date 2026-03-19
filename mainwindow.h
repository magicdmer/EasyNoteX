#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "notewidget.h"
#include <QVector>
#include <QSettings>
#include <QFont>
#include <QListWidgetItem>
#include "finddialog.h"
#include <QSystemTrayIcon>
#include "qxtglobalshortcut.h"
#include "settabledialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum {
    FILE_CREATE_TIME = Qt::UserRole + 1,
    FILE_MODIFY_TIME
};

enum SortType {
    SORT_BY_NAME,
    SORT_BY_CREAT_AORDER,
    SORT_BY_CREAT_DORDER,
    SORT_BY_MODIFY_AORDER,
    SORT_BY_MODIFY_DORDER
};

class MyListWidgetItem : public QListWidgetItem
{
public:
    MyListWidgetItem(QListWidget *parent=0):QListWidgetItem(parent){}
    bool operator<(const QListWidgetItem &other) const
    {
        bool ret = false;

        switch (m_sort_type)
        {
            case SORT_BY_NAME:
            {
                ret = QListWidgetItem::operator<(other);
            }
            break;
            case SORT_BY_CREAT_AORDER:
            case SORT_BY_CREAT_DORDER:
            {
                uint32_t a, b;
                a = this->data(FILE_CREATE_TIME).toUInt();
                b = other.data(FILE_CREATE_TIME).toUInt();
                ret = a < b;
            }
            break;
            case SORT_BY_MODIFY_AORDER:
            case SORT_BY_MODIFY_DORDER:
            {
                uint32_t a, b;
                a = this->data(FILE_MODIFY_TIME).toUInt();
                b = other.data(FILE_MODIFY_TIME).toUInt();
                ret = a < b;
            }
            break;
            default:
            {
                ret = QListWidgetItem::operator<(other);
            }
            break;
        }

        return(ret);
    }

    void setSortType(SortType t)
    {
        m_sort_type = t;
    }

private:
    SortType m_sort_type;
};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

public slots:
    void sltActionFontClick();
    void sltLeftDoubleClicked(QListWidgetItem *item);
    void sltRemoveTab(int index);
    void sltTabDoubleClicked(int index);
    void sltActionFind();
    void sltActionSaveToHtml();
    void sltActionSaveToText();
    void sltHotKey();
    void sltTrayActived(QSystemTrayIcon::ActivationReason reason);
    void sltExit();
    void sltSet();
    void sltKeepTop();
    void sltAbout();
    void sltTabMenuRequested(const QPoint &pos);
    void sltActionDelete();
    void sltActionRename();
    void sltActionCloseLeft();
    void sltActionCloseRight();
    void sltActionCloseOther();
    void sltActionNew();
    void sltActionHelp();
    void sltComboboxMenuRequested(const QPoint &pos);
    void sltComboActionNew();
    void sltComboActionRename();
    void sltComboActionDelete();
    void sltCurrentIndexChanged(const QString &text);
    void sltListActonDelete();
    void sltListMenuRequested(const QPoint &pos);
    void sltListActionMove();
    void sltTabActionMove();
    void sltActionInsertTable();
    void sltDarkMode();
    void sltSetFontColor();
    void sltSetBgColor();

public:
    bool find(QString& text,QTextDocument::FindFlags flags);
    void newTab();
    void renameTab(int tabIndex, QString& newName);
    void save();
    void initNoteBook();
    void refreshMenu();
    void sortFileList();

private:
    Ui::MainWindow *ui;
    QVector<NoteWidget*> m_vecNoteWidget;
    QSettings* m_setting;
    QFont m_editor_font;
    FindDialog* m_findDlg;
    QSystemTrayIcon* m_pTrayIcon;
    QxtGlobalShortcut* m_shortcut;
    QAction* m_action_exit;
    QString m_hotkey;
    bool m_can_exit;
    QAction* m_action_new;
    QAction* m_action_rename;
    QAction* m_action_delete;
    QAction* m_action_close_left;
    QAction* m_action_close_right;
    QAction* m_action_close_other;
    QMenu* m_tabMenu;
    QMenu* m_comboMenu;
    QAction* m_combo_action_new;
    QAction* m_combo_action_rename;
    QAction* m_combo_action_delete;
    QString m_notebook;
    QMenu* m_listMenu;
    QAction* m_list_action_delete;
    QMenu* m_listMoveMenu;
    QMenu* m_tabMoveMenu;
    SetTableDialog* m_tableDlg;
    SortType m_sort_type;
};
#endif // MAINWINDOW_H
