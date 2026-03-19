#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDialog>
#include "renamedialog.h"
#include <QFileDialog>
#include <QWindowStateChangeEvent>
#include <QTimer>
#include "setdialog.h"
#include "aboutdialog.h"
#include <Windows.h>
#include "helpdialog.h"
#include <QShortcut>
#include <QMessageBox>
#include <QDateTime>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_can_exit(false)
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0,3);
    ui->splitter->setStretchFactor(1,10);
    ui->splitter->setContentsMargins(0,3,0,0);

    QWidget* defaultWidget =ui->tabWidgetNote->widget(0);
    ui->tabWidgetNote->removeTab(0);
    delete defaultWidget;

    ui->tabWidgetNote->setTabsClosable(true);
    ui->tabWidgetNote->setMovable(true);
    ui->tabWidgetNote->installEventFilter(this);
    QCoreApplication::instance()->installEventFilter(this);
    connect(ui->tabWidgetNote,SIGNAL(tabCloseRequested(int)),this,SLOT(sltRemoveTab(int)));
    connect(ui->tabWidgetNote,SIGNAL(tabBarDoubleClicked(int)),this,SLOT(sltTabDoubleClicked(int)));

    ui->tabWidgetNote->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tabWidgetNote->tabBar(),SIGNAL(customContextMenuRequested(const QPoint &)),
            this,SLOT(sltTabMenuRequested(const QPoint &)));

    ui->actionSaveToHtml->setShortcut(tr("Ctrl+S"));
    connect(ui->actionSaveToHtml,SIGNAL(triggered()),this,SLOT(sltActionSaveToHtml()));

    ui->actionSaveToTxt->setShortcut(tr("Ctrl+Shift+S"));
    connect(ui->actionSaveToTxt,SIGNAL(triggered()),this,SLOT(sltActionSaveToText()));

    ui->actionFind->setShortcut(tr("Ctrl+F"));
    connect(ui->actionFind,SIGNAL(triggered()),this,SLOT(sltActionFind()));
    ui->actionHelp->setShortcut(tr("F1"));
    connect(ui->actionHelp,SIGNAL(triggered()),this,SLOT(sltActionHelp()));
    ui->actionInsertTable->setShortcut(tr("Ctrl+I"));
    connect(ui->actionInsertTable, SIGNAL(triggered()), this, SLOT(sltActionInsertTable()));

    m_action_exit = new QAction(tr("退出"),this);
    connect(m_action_exit, SIGNAL(triggered()), this,SLOT(sltExit()));

    m_pTrayIcon = new QSystemTrayIcon(this);
    m_pTrayIcon->setIcon(QIcon(":/images/app.ico"));
    m_pTrayIcon->setToolTip(tr("EasyNoteX"));
    connect(m_pTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this,SLOT(sltTrayActived(QSystemTrayIcon::ActivationReason)));

    QMenu* trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(m_action_exit);
    m_pTrayIcon->setContextMenu(trayIconMenu);
    m_pTrayIcon->show();

    m_action_new = new QAction(tr("新建"),this);
    m_action_new->setShortcut(tr("Ctrl+N"));
    connect(m_action_new,SIGNAL(triggered()), this,SLOT(sltActionNew()));
    QShortcut* shortcut_new = new QShortcut(QKeySequence("Ctrl+N"),this);
    connect(shortcut_new,SIGNAL(activated()), this,SLOT(sltActionNew()));

    m_action_rename = new QAction(tr("改名"),this);
    m_action_rename->setShortcut(tr("F2"));
    connect(m_action_rename,SIGNAL(triggered()), this,SLOT(sltActionRename()));
    QShortcut* shortcut_rename = new QShortcut(QKeySequence("F2"),this);
    connect(shortcut_rename,SIGNAL(activated()), this,SLOT(sltActionRename()));

    m_action_delete = new QAction(tr("删除"),this);
    m_action_delete->setShortcut(tr("Alt+Del"));
    connect(m_action_delete,SIGNAL(triggered()), this,SLOT(sltActionDelete()));
    QShortcut* shortcut_delete = new QShortcut(QKeySequence("Alt+Del"),this);
    connect(shortcut_delete,SIGNAL(activated()), this,SLOT(sltActionDelete()));

    m_action_close_left = new QAction(tr("关闭左边"),this);
    connect(m_action_close_left,SIGNAL(triggered()), this,SLOT(sltActionCloseLeft()));

    m_action_close_right = new QAction(tr("关闭右边"),this);
    connect(m_action_close_right,SIGNAL(triggered()), this,SLOT(sltActionCloseRight()));

    m_action_close_other = new QAction(tr("关闭其他"),this);
    connect(m_action_close_other,SIGNAL(triggered()), this,SLOT(sltActionCloseOther()));

    m_tabMenu = new QMenu(this);
    m_tabMenu->addAction(m_action_rename);
    m_tabMoveMenu = m_tabMenu->addMenu(tr("移至"));
    m_tabMenu->addAction(m_action_delete);
    m_tabMenu->addAction(m_action_close_left);
    m_tabMenu->addAction(m_action_close_right);
    m_tabMenu->addAction(m_action_close_other);

    m_combo_action_new = new QAction(tr("新建"),this);
    connect(m_combo_action_new,SIGNAL(triggered()), this,SLOT(sltComboActionNew()));

    m_combo_action_rename = new QAction(tr("改名"),this);
    connect(m_combo_action_rename,SIGNAL(triggered()), this,SLOT(sltComboActionRename()));

    m_combo_action_delete = new QAction(tr("删除"),this);
    connect(m_combo_action_delete,SIGNAL(triggered()), this,SLOT(sltComboActionDelete()));

    m_comboMenu = new QMenu(this);
    m_comboMenu->addAction(m_combo_action_new);
    m_comboMenu->addAction(m_combo_action_rename);
    m_comboMenu->addAction(m_combo_action_delete);

    connect(ui->comboBox,SIGNAL(customContextMenuRequested(const QPoint &)),
            this,SLOT(sltComboboxMenuRequested(const QPoint &)));

    m_list_action_delete = new QAction(tr("删除"),this);
    connect(m_list_action_delete,SIGNAL(triggered()), this, SLOT(sltListActonDelete()));

    m_listMenu = new QMenu(this);
    m_listMoveMenu = m_listMenu->addMenu(tr("移至"));
    m_listMenu->addAction(m_list_action_delete);

    connect(ui->listWidgetFile,SIGNAL(customContextMenuRequested(const QPoint &)),
            this,SLOT(sltListMenuRequested(const QPoint &)));

    m_findDlg = new FindDialog(this);
    m_tableDlg = new SetTableDialog(this);

    if (!QFile::exists("EasyNote.ini"))
    {
        m_setting = new QSettings("EasyNote.ini",QSettings::IniFormat,this);
        m_setting->setIniCodec("UTF-8");
        m_setting->setValue("hotkey","Alt+O");
        m_setting->setValue("last_open_notebook",tr("我的记事本"));
        QFont font("微软雅黑",14);
        m_setting->setValue("/Editor/font",font.toString());
        m_editor_font = font;
        m_hotkey = "Alt+O";
        m_sort_type = SORT_BY_NAME;
    }
    else
    {
        m_setting = new QSettings("EasyNote.ini",QSettings::IniFormat,this);
        m_setting->setIniCodec("UTF-8");
        ui->splitter->restoreState(m_setting->value("splitter_size").toByteArray());
        int iWidth = m_setting->value("Width").toInt();
        int iHeigth = m_setting->value("Heigth").toInt();
        if (iWidth && iHeigth)
        {
           this->resize(QSize(iWidth,iHeigth));
        }

        QString strFont = m_setting->value("/Editor/font").toString();
        m_editor_font.fromString(strFont);

        m_hotkey = m_setting->value("hotkey").toString();
        int keep_top = m_setting->value("keep_top").toInt(0);
        if (keep_top)
        {
            ui->actionTop->setChecked(true);
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        }

        m_sort_type = SortType(m_setting->value("sort_type", 0).toInt());
    }

    QDir dir;
    dir.mkdir("data");
    dir.setPath("data");
    QStringList folderList = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Time);
    if (folderList.isEmpty())
    {
        QDir temp;
        temp.mkdir("data\\我的记事本");
        ui->comboBox->addItem("我的记事本");
    }
    else
    {
        ui->comboBox->addItems(folderList);
    }

    QString noteBook = m_setting->value("last_open_notebook").toString();

    if (-1 != ui->comboBox->findText(noteBook))
    {
        ui->comboBox->setCurrentText(noteBook);
    }
    else
    {
        ui->comboBox->setCurrentText(0);
    }

    m_notebook = ui->comboBox->currentText();

    initNoteBook();

    refreshMenu();

    m_shortcut = new QxtGlobalShortcut(QKeySequence(),this);
    if (m_shortcut->setShortcut(QKeySequence(m_hotkey)))
    {
        connect(m_shortcut,SIGNAL(activated()),this,SLOT(sltHotKey()));
    }

    connect(ui->actionFont,SIGNAL(triggered()),this,SLOT(sltActionFontClick()));
    connect(ui->actionSet,SIGNAL(triggered()),this,SLOT(sltSet()));
    connect(ui->actionTop,SIGNAL(triggered()),this,SLOT(sltKeepTop()));
    connect(ui->actionDark, SIGNAL(triggered()), this, SLOT(sltDarkMode()));
    connect(ui->actionFontColor, SIGNAL(triggered()), this, SLOT(sltSetFontColor()));
    connect(ui->actionBgColor, SIGNAL(triggered()), this, SLOT(sltSetBgColor()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(sltAbout()));
    connect(ui->listWidgetFile,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(sltLeftDoubleClicked(QListWidgetItem *)));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(const QString&)),
            this,SLOT(sltCurrentIndexChanged(const QString&)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initNoteBook()
{
    ui->listWidgetFile->clear();
    int tabCount = ui->tabWidgetNote->count();
    for (int i = 0; i < tabCount; i++)
    {
        NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(0);
        ui->tabWidgetNote->removeTab(0);
        delete widget;
    }

    QString folderPath = QString("data\\%1").arg(m_notebook);

    QDir dir;
    dir.mkdir(folderPath);

    dir.setPath(folderPath);
    QStringList filters;
    filters << "*.enote";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList();
    if (fileList.isEmpty())
    {
        NoteWidget* defaultNote = new NoteWidget(ui->tabWidgetNote,m_notebook,tr("默认页"),m_editor_font);
        ui->tabWidgetNote->addTab(defaultNote,tr("默认页"));

        MyListWidgetItem* item = new MyListWidgetItem(ui->listWidgetFile);
        item->setData(FILE_CREATE_TIME, QDateTime::currentDateTime().toTime_t());
        item->setData(FILE_MODIFY_TIME, QDateTime::currentDateTime().toTime_t());
        item->setSortType(m_sort_type);
        item->setText(tr("默认页"));
        ui->listWidgetFile->addItem(item);
    }
    else
    {
        foreach (QFileInfo noteFile, fileList)
        {
            QString fileName = noteFile.completeBaseName();
            MyListWidgetItem* item = new MyListWidgetItem(ui->listWidgetFile);
            item->setData(FILE_CREATE_TIME, noteFile.birthTime().toTime_t());
            item->setData(FILE_MODIFY_TIME, noteFile.lastModified().toTime_t());
            item->setSortType(m_sort_type);
            item->setText(fileName);
            ui->listWidgetFile->addItem(item);
        }

        QString noteHistory = m_setting->value("History/" + m_notebook).toString();
        if (noteHistory.isEmpty())
        {
            newTab();
        }
        else
        {
            QWidget* curWidget = nullptr;
            QString currentNote = m_setting->value("last_open_tab").toString();

            QStringList noteList = noteHistory.split('|');
            foreach(QString fileName, noteList)
            {
                QString filePath = QString("data\\%1\\%2.enote").arg(m_notebook).arg(fileName);
                if (QFile::exists(filePath))
                {
                    NoteWidget* note = new NoteWidget(ui->tabWidgetNote,m_notebook,fileName,m_editor_font);
                    ui->tabWidgetNote->addTab(note,fileName);
                    if (fileName == currentNote)
                    {
                        curWidget = note;
                    }
                }
            }

            if (curWidget)
            {
                ui->tabWidgetNote->setCurrentWidget(curWidget);
            }
        }
    }

    if (m_sort_type == SORT_BY_CREAT_DORDER || m_sort_type == SORT_BY_MODIFY_DORDER)
        ui->listWidgetFile->sortItems(Qt::DescendingOrder);
    else
        ui->listWidgetFile->sortItems(Qt::AscendingOrder);
}

void MainWindow::refreshMenu()
{
    QDir dir("data");
    QStringList folderList = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Time);
    if (folderList.isEmpty())
    {
        return;
    }

    QList<QAction*> tab_lst = m_tabMoveMenu->actions();
    QList<QAction*> list_lst = m_listMoveMenu->actions();

    qDeleteAll(tab_lst);
    qDeleteAll(list_lst);

    m_tabMoveMenu->clear();
    m_listMoveMenu->clear();

    foreach(QString note, folderList)
    {
        if (note != m_notebook)
        {
            QAction* tabAction = new QAction(note,this);
            connect(tabAction,SIGNAL(triggered()),this,SLOT(sltTabActionMove()));
            m_tabMoveMenu->addAction(tabAction);

            QAction* listAction = new QAction(note,this);
            connect(listAction,SIGNAL(triggered()),this,SLOT(sltListActionMove()));
            m_listMoveMenu->addAction(listAction);
        }
    }
}

void MainWindow::save()
{
    m_setting->setValue("splitter_size", ui->splitter->saveState());
    int keep_top = ui->actionTop->isChecked();
    m_setting->setValue("keep_top",keep_top);
    m_setting->setValue("last_open_notebook",ui->comboBox->currentText());
    m_setting->setValue("Width", width());
    m_setting->setValue("Heigth", height());

    int current = ui->tabWidgetNote->currentIndex();
    m_setting->setValue("last_open_tab", ui->tabWidgetNote->tabText(current));

    m_setting->beginGroup("History");
    QStringList noteList;
    for (int i = 0; i < ui->tabWidgetNote->count(); i++)
    {
        NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(i);
        if (!widget->isEmpty())
        {
            QString fileName = ui->tabWidgetNote->tabText(i);
            noteList.append(fileName);
        }
    }
    QString noteHistory = noteList.join('|');
    m_setting->setValue(m_notebook,noteHistory);
    m_setting->endGroup();

    m_setting->sync();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->tabWidgetNote)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            newTab();
            return true;
        }
    }
    else if (watched == this)
    {
        if (event->type() == QEvent::WindowStateChange)
        {
            if (this->windowState() & Qt::WindowMinimized)
            {
                int minToTray = m_setting->value("minimize_to_tray").toInt(0);
                if (minToTray)
                {
                    QTimer::singleShot(0, this, SLOT(hide()));
                }
            }
        }
        else if (event->type() == QEvent::Close)
        {
            save();
            int closeToTray = m_setting->value("close_to_tray").toInt(0);
            if (closeToTray && !m_can_exit)
            {
                event->ignore();
                this->hide();
                return true;
            }
        }
    }

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *me = static_cast<QKeyEvent*>(event);
        if (me->key() == Qt::Key_Escape)
        {
            int escToTray = m_setting->value("esc_to_tray").toInt(0);
            if (escToTray)
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                return true;
            }
            else
            {
                if (isActiveWindow())
                {
                    this->close();
                    return true;
                }
            }
        }
        else if (me->key() == Qt::Key_Backtab)
        {
            NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->currentWidget();
            if (widget)
            {
                widget->dealBackTab();
            }
            return true;
        }
    }

    return false;
}

void MainWindow::newTab()
{
    for (int i = 1; i < 100; i++)
    {
        QString newName = QString("新建页 %1").arg(i);
        QList<QListWidgetItem*> item_list = ui->listWidgetFile->findItems(newName,Qt::MatchExactly);
        if (item_list.isEmpty())
        {
            NoteWidget* note = new NoteWidget(ui->tabWidgetNote,m_notebook,newName,m_editor_font);
            ui->tabWidgetNote->addTab(note,newName);
            ui->tabWidgetNote->setCurrentWidget(note);

            MyListWidgetItem* item = new MyListWidgetItem(ui->listWidgetFile);
            item->setData(FILE_CREATE_TIME, QDateTime::currentDateTime().toTime_t());
            item->setData(FILE_MODIFY_TIME, QDateTime::currentDateTime().toTime_t());
            item->setSortType(m_sort_type);
            item->setText(newName);
            ui->listWidgetFile->addItem(item);

            break;
        }
    }
}

void MainWindow::renameTab(int tabIndex, QString& newName)
{
    QString oldName = ui->tabWidgetNote->tabText(tabIndex);
    if (oldName == newName)
    {
        return;
    }

    QString newNotePath = QString("data\\%1\\%2.enote").arg(m_notebook).arg(newName);
    if (QFile::exists(newNotePath))
    {
        QMessageBox::information(this,tr("提示"),tr("命名笔记已存在，请换一个名字"));
        return;
    }

    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(tabIndex);
    widget->rename(newName);
    ui->tabWidgetNote->setTabText(tabIndex,newName);
    QList<QListWidgetItem*> item_list = ui->listWidgetFile->findItems(oldName,Qt::MatchExactly);
    if (!item_list.isEmpty())
    {
        item_list[0]->setText(newName);
    }
}

bool MainWindow::find(QString &text,QTextDocument::FindFlags flags)
{
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->currentWidget();
    if (!widget) return false;

    if (!widget->find(text,flags))
    {
        return false;
    }

    return true;
}

void MainWindow::sortFileList()
{
    ui->listWidgetFile->clear();


    QString folderPath = QString("data\\%1").arg(m_notebook);

    QDir dir;
    dir.mkdir(folderPath);

    dir.setPath(folderPath);
    QStringList filters;
    filters << "*.enote";
    dir.setNameFilters(filters);
    QFileInfoList fileList = dir.entryInfoList();
    if (!fileList.isEmpty())
    {
        foreach (QFileInfo noteFile, fileList)
        {
            QString fileName = noteFile.completeBaseName();
            MyListWidgetItem* item = new MyListWidgetItem(ui->listWidgetFile);
            item->setData(FILE_CREATE_TIME, noteFile.birthTime().toTime_t());
            item->setData(FILE_MODIFY_TIME, noteFile.lastModified().toTime_t());
            item->setSortType(m_sort_type);
            item->setText(fileName);
            ui->listWidgetFile->addItem(item);
        }

        if (m_sort_type == SORT_BY_CREAT_DORDER || m_sort_type == SORT_BY_MODIFY_DORDER)
            ui->listWidgetFile->sortItems(Qt::DescendingOrder);
        else
            ui->listWidgetFile->sortItems(Qt::AscendingOrder);
    }
}

void MainWindow::sltActionFontClick()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,m_editor_font,this);
    if (ok)
    {
        for (int i = 0; i < ui->tabWidgetNote->count(); i++)
        {
            NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(i);
            if (widget)
            {
                widget->setCurrentFont(font);
            }
        }

        m_setting->setValue("/Editor/font",font.toString());
    }
}

void MainWindow::sltLeftDoubleClicked(QListWidgetItem *item)
{
    QString fileName = item->text();
    NoteWidget* noteItem = ui->tabWidgetNote->findChild<NoteWidget*>(fileName);
    if (noteItem)
    {
        ui->tabWidgetNote->setCurrentWidget(noteItem);
    }
    else
    {
        NoteWidget* note = new NoteWidget(ui->tabWidgetNote,m_notebook,fileName,m_editor_font);
        ui->tabWidgetNote->addTab(note,fileName);
        ui->tabWidgetNote->setCurrentWidget(note);
    }
}

void MainWindow::sltRemoveTab(int index)
{
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(index);
    ui->tabWidgetNote->removeTab(index);
    if (widget->isEmpty())
    {
        QList<QListWidgetItem*> item_list = ui->listWidgetFile->findItems(widget->objectName(),Qt::MatchExactly);
        if (!item_list.isEmpty())
        {
            int row = ui->listWidgetFile->row(item_list[0]);
            ui->listWidgetFile->takeItem(row);
            delete item_list[0];
        }
    }
    delete widget;
}

void MainWindow::sltTabDoubleClicked(int index)
{
    if (index == -1)
    {
        return;
    }

    QString tabName = ui->tabWidgetNote->tabText(index);

    RenameDialog dlg(tabName,this);
    if (dlg.exec() == QDialog::Accepted)
    {
        renameTab(index, dlg.m_newName);
    }
}

void MainWindow::sltActionFind()
{
    m_findDlg->show();
}

void MainWindow::sltActionInsertTable()
{
    int index = ui->tabWidgetNote->currentIndex();
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(index);
    if (widget == nullptr)
    {
        QMessageBox::information(this, tr("提示"), tr("请打开一个笔记插入"));
        return;
    }

    if (m_tableDlg->exec() == QDialog::Accepted)
    {
        widget->insertTable(m_tableDlg->m_row, m_tableDlg->m_col, m_tableDlg->m_percent);
    }
}

void MainWindow::sltActionSaveToHtml()
{
    int index = ui->tabWidgetNote->currentIndex();
    QString tabName = ui->tabWidgetNote->tabText(index);

    QString filePath = QFileDialog::getSaveFileName(this,tr("文件另存为"),tabName,tr("Html file (*.html)"));
    if (filePath.isEmpty())
    {
        return;
    }

    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(index);
    widget->save(filePath);
}

void MainWindow::sltActionSaveToText()
{
    int index = ui->tabWidgetNote->currentIndex();
    QString tabName = ui->tabWidgetNote->tabText(index);

    QString filePath = QFileDialog::getSaveFileName(this,tr("文件另存为"),tabName,tr("Text file (*.txt)"));
    if (filePath.isEmpty())
    {
        return;
    }

    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(index);
    widget->save(filePath, 1);
}

void MainWindow::sltHotKey()
{
    if (this->isHidden())
    {
        this->show();

        this->setWindowState(this->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    }
    else
    {
        this->hide();
    }
}

void MainWindow::sltTrayActived(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        sltHotKey();
    }
}

void MainWindow::sltExit()
{
    m_can_exit = true;
    close();
}

void MainWindow::sltSet()
{
    SetDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        if (dlg.m_shortcut != m_hotkey)
        {
            m_hotkey = dlg.m_shortcut;
            if (m_shortcut->setShortcut(QKeySequence(m_hotkey)))
            {
                m_setting->setValue("hotkey",m_hotkey);
            }
        }

        for (int i = 0; i < ui->tabWidgetNote->count(); i++)
        {
            NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(i);
            if (widget)
            {
                widget->setTabWidth(dlg.m_tabWidth);
            }
        }

        if (int(m_sort_type) != dlg.m_sort_type)
        {
            m_sort_type = SortType(dlg.m_sort_type);
            sortFileList();
        }
    }
}

void MainWindow::sltKeepTop()
{
    if (ui->actionTop->isChecked())
    {
        ::SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else
    {
        ::SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

}

void MainWindow::sltDarkMode()
{
    static QPalette lightPalette;

    if (ui->actionDark->isChecked())
    {
        lightPalette = qApp->palette();

        // set style
        qApp->setStyle(QStyleFactory::create("Fusion"));

        // modify palette to dark
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window,QColor(53,53,53));
        darkPalette.setColor(QPalette::WindowText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
        darkPalette.setColor(QPalette::Base,QColor(42,42,42));
        darkPalette.setColor(QPalette::AlternateBase,QColor(66,66,66));
        darkPalette.setColor(QPalette::ToolTipBase,Qt::white);
        darkPalette.setColor(QPalette::ToolTipText,Qt::white);
        darkPalette.setColor(QPalette::Text,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
        darkPalette.setColor(QPalette::Dark,QColor(35,35,35));
        darkPalette.setColor(QPalette::Shadow,QColor(20,20,20));
        darkPalette.setColor(QPalette::Button,QColor(53,53,53));
        darkPalette.setColor(QPalette::ButtonText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
        darkPalette.setColor(QPalette::BrightText,Qt::red);
        darkPalette.setColor(QPalette::Link,QColor(42,130,218));
        darkPalette.setColor(QPalette::Highlight,QColor(42,130,218));
        darkPalette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
        darkPalette.setColor(QPalette::HighlightedText,Qt::white);
        darkPalette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));

        qApp->setPalette(darkPalette);
    }
    else
    {
        qApp->setStyle(QStyleFactory::create("windowsvista"));
        qApp->setPalette(lightPalette);
    }
}

void MainWindow::sltSetFontColor()
{
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->currentWidget();
    if (!widget) return;

    QColor color = QColorDialog::getColor(Qt::white,this);
    if(color.isValid())
        widget->setFontColor(color);
}

void MainWindow::sltSetBgColor()
{
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->currentWidget();
    if (!widget) return;

    QColor color = QColorDialog::getColor(Qt::white,this);
    QPalette palette;
    palette.setColor(QPalette::Base,color);
    if(color.isValid())
        widget->setBgColor(palette);
}

void MainWindow::sltAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::sltActionDelete()
{
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->currentWidget();
    int index = ui->tabWidgetNote->currentIndex();
    ui->tabWidgetNote->removeTab(index);
    QList<QListWidgetItem*> item_list = ui->listWidgetFile->findItems(widget->objectName(),Qt::MatchExactly);
    if (!item_list.isEmpty())
    {
        int row = ui->listWidgetFile->row(item_list[0]);
        ui->listWidgetFile->takeItem(row);
        delete item_list[0];
    }
    widget->deletefile();
    delete widget;
}

void MainWindow::sltActionRename()
{
    int index = ui->tabWidgetNote->currentIndex();
    QString tabName = ui->tabWidgetNote->tabText(index);

    RenameDialog dlg(tabName,this);
    if (dlg.exec() == QDialog::Accepted)
    {
        renameTab(index, dlg.m_newName);
    }
}

void MainWindow::sltActionNew()
{
    newTab();
}

void MainWindow::sltActionCloseLeft()
{
    int index = ui->tabWidgetNote->currentIndex();

    int delIndex = 0;
    for (int i = 0; i < index; i++)
    {
        sltRemoveTab(delIndex);
    }
}

void MainWindow::sltActionCloseRight()
{
    int index = ui->tabWidgetNote->currentIndex();
    int count = ui->tabWidgetNote->count();

    int delIndex = index + 1;
    for (int i = index + 1 ; i < count; i++)
    {
        sltRemoveTab(delIndex);
    }
}

void MainWindow::sltActionCloseOther()
{
    sltActionCloseLeft();
    sltActionCloseRight();
}

void MainWindow::sltComboActionNew()
{
    RenameDialog dlg(this);
    dlg.setWindowTitle("新建");
    if (dlg.exec() == QDialog::Accepted)
    {
        QString noteFolder = QString("data\\%1").arg(dlg.m_newName);
        if (QFile::exists(noteFolder))
        {
            QMessageBox::information(this,tr("提示"),tr("记事本已经存在"));
            return;
        }

        QDir dir;
        dir.mkdir(noteFolder);

        ui->comboBox->addItem(dlg.m_newName);
        ui->comboBox->setCurrentText(dlg.m_newName);
    }
}

void MainWindow::sltComboActionRename()
{
    RenameDialog dlg(ui->comboBox->currentText(),this);
    dlg.setWindowTitle("改名");
    if (dlg.exec() == QDialog::Accepted)
    {
        QString oldPath = QString("data\\%1").arg(ui->comboBox->currentText());
        QString newPath = QString("data\\%1").arg(dlg.m_newName);

        if (QFile::exists(newPath))
        {
            QMessageBox::information(this,tr("提示"),tr("命名记事本已存在，请换一个名字"));
            return;
        }

        QFile::rename(oldPath,newPath);

        QString keyName = QString("/History/%1").arg(ui->comboBox->currentText());
        QString history = m_setting->value(keyName).toString();
        m_setting->remove(keyName);
        keyName = QString("/History/%1").arg(dlg.m_newName);
        m_setting->setValue(keyName,history);

        m_notebook = dlg.m_newName;

        QAction* renameAction = m_tabMoveMenu->findChild<QAction*>(ui->comboBox->currentText());
        if (renameAction)
        {
            renameAction->setText(dlg.m_newName);
        }

        renameAction = m_listMoveMenu->findChild<QAction*>(ui->comboBox->currentText());
        if (renameAction)
        {
            renameAction->setText(dlg.m_newName);
        }

        int curIndex = ui->comboBox->currentIndex();
        ui->comboBox->setItemText(curIndex,dlg.m_newName);
    }
}

void MainWindow::sltComboActionDelete()
{
    if (ui->comboBox->count() == 1)
    {
        QMessageBox::information(this,tr("提示"),tr("请至少留下一个记事本"));
        return;
    }

    if (QMessageBox::Yes == QMessageBox::information(this,tr("提示"),
                             tr("你将删除该记事本下的所有笔记，是否继续？"),
                             QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes))
    {
        QString notePath = QString("data\\%1").arg(ui->comboBox->currentText());
        QDir dir(notePath);
        dir.removeRecursively();
        int index = ui->comboBox->currentIndex();
        ui->comboBox->removeItem(index);
        QString keyName = QString("/History/%1").arg(ui->comboBox->currentText());
        m_setting->remove(keyName);
    }
}

void MainWindow::sltListActonDelete()
{
    QString noteTitle = ui->listWidgetFile->currentItem()->text();
    NoteWidget* noteItem = ui->tabWidgetNote->findChild<NoteWidget*>(noteTitle);
    if (noteItem)
    {
        delete ui->listWidgetFile->takeItem(ui->listWidgetFile->currentRow());

        int index = ui->tabWidgetNote->indexOf(noteItem);
        ui->tabWidgetNote->removeTab(index);

        noteItem->deletefile();
        delete noteItem;
    }
    else
    {
        delete ui->listWidgetFile->takeItem(ui->listWidgetFile->currentRow());

        QString filePath = QString("data\\%1\\%2.enote").arg(m_notebook).arg(noteTitle);
        QFile::remove(filePath);
    }
}

void MainWindow::sltCurrentIndexChanged(const QString &text)
{
    save();
    m_notebook = text;
    initNoteBook();
    refreshMenu();
}

void MainWindow::sltActionHelp()
{
    HelpDialog dlg(this);
    dlg.exec();
}

void MainWindow::sltTabActionMove()
{
    QAction* action = static_cast<QAction*>(sender());
    QString noteBookName = action->text();

    NoteWidget* noteItem = (NoteWidget*)ui->tabWidgetNote->currentWidget();
    if (!noteItem)
    {
        QMessageBox::information(this,tr("提示"),tr("请选择一个笔记进行移动"));
        return;
    }

    QString noteTitle = noteItem->objectName();

    QString srcPath = QString("data\\%1\\%2.enote").arg(m_notebook).arg(noteTitle);
    QString dstPath = QString("data\\%1\\%2.enote").arg(noteBookName).arg(noteTitle);

    if (QFile::exists(dstPath))
    {
        QMessageBox::information(this,tr("提示"),tr("目标记事本已经包含有同名笔记，无法移动"));
        return;
    }

    QList<QListWidgetItem*> item_list = ui->listWidgetFile->findItems(noteTitle,Qt::MatchExactly);
    if (!item_list.isEmpty())
    {
        int row = ui->listWidgetFile->row(item_list[0]);
        ui->listWidgetFile->takeItem(row);
        delete item_list[0];
    }

    int index = ui->tabWidgetNote->indexOf(noteItem);
    ui->tabWidgetNote->removeTab(index);
    delete noteItem;

    QFile::rename(srcPath,dstPath);
}

void MainWindow::sltListActionMove()
{
    QAction* action = static_cast<QAction*>(sender());
    QString noteBookName = action->text();

    QListWidgetItem* item = ui->listWidgetFile->currentItem();
    if (!item)
    {
        QMessageBox::information(this,tr("提示"),tr("请选择一个笔记进行移动"));
        return;
    }

    QString noteTitle = item->text();

    QString srcPath = QString("data\\%1\\%2.enote").arg(m_notebook).arg(noteTitle);
    QString dstPath = QString("data\\%1\\%2.enote").arg(noteBookName).arg(noteTitle);

    if (QFile::exists(dstPath))
    {
        QMessageBox::information(this,tr("提示"),tr("目标记事本已经包含有同名笔记，无法移动"));
        return;
    }

    NoteWidget* noteItem = ui->tabWidgetNote->findChild<NoteWidget*>(noteTitle);
    if (noteItem)
    {
        int index = ui->tabWidgetNote->indexOf(noteItem);
        ui->tabWidgetNote->removeTab(index);
        delete noteItem;
    }

    delete ui->listWidgetFile->takeItem(ui->listWidgetFile->currentRow());


    QFile::rename(srcPath,dstPath);
}

void MainWindow::sltTabMenuRequested(const QPoint& pos)
{
    m_tabMenu->popup(QCursor::pos());
}

void MainWindow::sltComboboxMenuRequested(const QPoint &pos)
{
    m_comboMenu->popup(QCursor::pos());
}

void MainWindow::sltListMenuRequested(const QPoint &pos)
{
    m_listMenu->popup(QCursor::pos());
}
