#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helpfunc.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "renamedialog.h"
#include <QFileDialog>
#include <QWindowStateChangeEvent>
#include <QTimer>
#include "setdialog.h"
#include "aboutdialog.h"
#include "helpdialog.h"
#include <QShortcut>
#include <QMessageBox>
#include <QDateTime>
#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>
#include <QStyle>
#include <QTextDocument>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_can_exit(false)
    , m_isUos(isUos())
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
    m_listGroupMoveMenu = m_listMenu->addMenu(tr("改分组"));
    m_listMoveMenu = m_listMenu->addMenu(tr("移至记事本"));
    m_listMenu->addAction(m_list_action_delete);

    m_group_action_new_note = new QAction(tr("在此分组新建笔记"),this);
    connect(m_group_action_new_note,SIGNAL(triggered()), this, SLOT(sltGroupActionNewNote()));
    m_group_action_rename = new QAction(tr("重命名分组"),this);
    connect(m_group_action_rename,SIGNAL(triggered()), this, SLOT(sltGroupActionRename()));
    m_group_action_delete = new QAction(tr("删除分组"),this);
    connect(m_group_action_delete,SIGNAL(triggered()), this, SLOT(sltGroupActionDelete()));

    m_groupMenu = new QMenu(this);
    m_groupMenu->addAction(m_group_action_new_note);
    m_groupMenu->addAction(m_group_action_rename);
    m_groupMenu->addAction(m_group_action_delete);

    m_blank_action_new_note = new QAction(tr("新建笔记"),this);
    connect(m_blank_action_new_note,SIGNAL(triggered()), this, SLOT(sltActionNew()));
    m_blank_action_new_group = new QAction(tr("新建分组"),this);
    connect(m_blank_action_new_group,SIGNAL(triggered()), this, SLOT(sltGroupActionNew()));

    m_blankMenu = new QMenu(this);
    m_blankMenu->addAction(m_blank_action_new_note);
    m_blankMenu->addAction(m_blank_action_new_group);

    connect(ui->treeWidgetFile,SIGNAL(customContextMenuRequested(const QPoint &)),
            this,SLOT(sltListMenuRequested(const QPoint &)));
    connect(ui->treeWidgetFile,SIGNAL(noteDropped(QTreeWidgetItem*,QString)),
            this,SLOT(sltNoteDropped(QTreeWidgetItem*,QString)));

    keepTreeAlwaysActive();

    m_findDlg = new FindDialog(this);
    m_tableDlg = new SetTableDialog(this);

    const QString settingsPath = settingsFile();
    if (!QFile::exists(settingsPath))
    {
        m_setting = new QSettings(settingsPath,QSettings::IniFormat,this);
        m_setting->setIniCodec("UTF-8");
        m_setting->setValue("hotkey","Alt+O");
        m_setting->setValue("last_open_notebook",tr("我的记事本"));
        QFont font("微软雅黑",14);
        m_setting->setValue("/Editor/font",font.toString());
        m_editor_font = font;
        m_default_pen = QColor(Qt::black);
        m_default_paper = QColor(Qt::white);
        m_hotkey = "Alt+O";
        m_sort_type = SORT_BY_NAME;
    }
    else
    {
        m_setting = new QSettings(settingsPath,QSettings::IniFormat,this);
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

        // 新建便签的默认笔色/纸色（缺省为黑字白底）。
        m_default_pen = QColor(m_setting->value("/Editor/pen_color").toString());
        if (!m_default_pen.isValid())
        {
            m_default_pen = QColor(Qt::black);
        }
        m_default_paper = QColor(m_setting->value("/Editor/paper_color").toString());
        if (!m_default_paper.isValid())
        {
            m_default_paper = QColor(Qt::white);
        }

        m_hotkey = m_setting->value("hotkey").toString();
        int keep_top = m_setting->value("keep_top").toInt(0);
        if (keep_top)
        {
            ui->actionTop->setChecked(true);
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        }

        m_sort_type = SortType(m_setting->value("sort_type", 0).toInt());
    }

    restoreWindowPlacement();

    QDir dir(notesRoot());
    QStringList folderList = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Time);
    if (folderList.isEmpty())
    {
        dir.mkdir(QString::fromUtf8("我的记事本"));
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
        ui->comboBox->setCurrentIndex(0);
    }

    m_notebook = ui->comboBox->currentText();

    initNoteBook();

    refreshMenu();

    m_shortcut = nullptr;
    if (!m_isUos)
    {
        m_shortcut = new QxtGlobalShortcut(QKeySequence(),this);
        if (m_shortcut->setShortcut(QKeySequence(m_hotkey)))
        {
            connect(m_shortcut,SIGNAL(activated()),this,SLOT(sltHotKey()));
        }
    }

    connect(ui->actionSet,SIGNAL(triggered()),this,SLOT(sltSet()));
    connect(ui->actionTop,SIGNAL(triggered()),this,SLOT(sltKeepTop()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(sltAbout()));
    connect(ui->treeWidgetFile,SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),this,SLOT(sltTreeItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(const QString&)),
            this,SLOT(sltCurrentIndexChanged(const QString&)));
            
    // 初始化搜索框动画
    m_loadingLabel = new QLabel(ui->lineEditSearch);
    m_loadingMovie = new QMovie(":/images/loading.gif");
    m_loadingMovie->setScaledSize(QSize(16, 16));
    m_loadingLabel->setMovie(m_loadingMovie);
    m_loadingLabel->setFixedSize(16, 16);
    m_loadingLabel->hide();

    // 布局动画到搜索框右侧
    QHBoxLayout* searchLayout = new QHBoxLayout(ui->lineEditSearch);
    searchLayout->addStretch();
    searchLayout->addWidget(m_loadingLabel);
    searchLayout->setContentsMargins(0, 0, 5, 0);
    ui->lineEditSearch->setTextMargins(0, 0, 25, 0); // 为动画预留空间

    // 移除默认的底部高亮边框，使其更加纯粹扁平
    ui->lineEditSearch->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #E5E5E5;"
        "  border-radius: 2px;"
        "  padding: 3px;"
        "  background: white;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid #CCCCCC;"
        "}"
    );

    // 初始化防抖定时器
    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);

    // 连接搜索相关信号槽
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::sltSearchTextChanged);
    connect(m_filterTimer, &QTimer::timeout, this, &MainWindow::sltStartFiltering);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeAllTabs()
{
    int tabCount = ui->tabWidgetNote->count();
    for (int i = 0; i < tabCount; i++)
    {
        NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(0);
        ui->tabWidgetNote->removeTab(0);
        delete widget;
    }
}

static bool noteLessThan(const QFileInfo& a, const QFileInfo& b, SortType type)
{
    switch (type)
    {
        case SORT_BY_CREAT_AORDER: return a.birthTime() < b.birthTime();
        case SORT_BY_CREAT_DORDER: return a.birthTime() > b.birthTime();
        case SORT_BY_MODIFY_AORDER: return a.lastModified() < b.lastModified();
        case SORT_BY_MODIFY_DORDER: return a.lastModified() > b.lastModified();
        case SORT_BY_NAME:
        default:
            return a.completeBaseName().localeAwareCompare(b.completeBaseName()) < 0;
    }
}

QString MainWindow::tabKey(const QString& groupName, const QString& noteName)
{
    return groupName.isEmpty() ? noteName : (groupName + QLatin1Char('/') + noteName);
}

NoteWidget *MainWindow::findOpenNoteWidget(const QString& groupName, const QString& noteName) const
{
	return ui->tabWidgetNote->findChild<NoteWidget*>(tabKey(groupName, noteName));
}

QString MainWindow::readNotePlainText(const QString& groupName, const QString& noteName) const
{
	NoteWidget* noteWidget = findOpenNoteWidget(groupName, noteName);
	if (noteWidget)
	{
		return noteWidget->plainText();
	}

	QFile file(noteFilePath(m_notebook, groupName, noteName));
	if (!file.open(QIODevice::ReadOnly))
	{
		return QString();
	}

	QString html = QString::fromUtf8(file.readAll());
	file.close();

	QTextDocument document;
	document.setHtml(html);
	return document.toPlainText();
}

bool MainWindow::noteMatchesSearch(const QString& groupName, const QString& noteName, const QString& text) const
{
	if (noteName.contains(text, Qt::CaseInsensitive))
	{
		return true;
	}

	QString plainText = readNotePlainText(groupName, noteName);
	return plainText.contains(text, Qt::CaseInsensitive);
}

QTreeWidgetItem* MainWindow::findGroupItem(const QString& groupName) const
{
    if (groupName.isEmpty()) return nullptr;
    QTreeWidget* tree = ui->treeWidgetFile;
    for (int i = 0; i < tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = tree->topLevelItem(i);
        if (item->data(0, ITEM_KIND_ROLE).toInt() == ITEM_GROUP && item->text(0) == groupName)
            return item;
    }
    return nullptr;
}

QTreeWidgetItem* MainWindow::findNoteItem(const QString& groupName, const QString& noteName) const
{
    QTreeWidget* tree = ui->treeWidgetFile;
    if (groupName.isEmpty())
    {
        for (int i = 0; i < tree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* item = tree->topLevelItem(i);
            if (item->data(0, ITEM_KIND_ROLE).toInt() == ITEM_NOTE && item->text(0) == noteName)
                return item;
        }
        return nullptr;
    }

    QTreeWidgetItem* group = findGroupItem(groupName);
    if (!group) return nullptr;
    for (int i = 0; i < group->childCount(); i++)
    {
        if (group->child(i)->text(0) == noteName) return group->child(i);
    }
    return nullptr;
}

QString MainWindow::currentGroup() const
{
    QTreeWidgetItem* item = ui->treeWidgetFile->currentItem();
    if (!item) return QString();
    if (item->data(0, ITEM_KIND_ROLE).toInt() == ITEM_GROUP) return item->text(0);
    QTreeWidgetItem* parent = item->parent();
    return parent ? parent->text(0) : QString();
}

QStringList MainWindow::listGroups() const
{
    QStringList groups;
    QTreeWidget* tree = ui->treeWidgetFile;
    for (int i = 0; i < tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = tree->topLevelItem(i);
        if (item->data(0, ITEM_KIND_ROLE).toInt() == ITEM_GROUP) groups << item->text(0);
    }
    return groups;
}

QTreeWidgetItem* MainWindow::addGroupItem(const QString& groupName)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, groupName);
    item->setData(0, ITEM_KIND_ROLE, ITEM_GROUP);
    item->setIcon(0, QIcon(":/images/group.svg"));
    item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    int insertAt = 0;
    QTreeWidget* tree = ui->treeWidgetFile;
    while (insertAt < tree->topLevelItemCount())
    {
        QTreeWidgetItem* sib = tree->topLevelItem(insertAt);
        if (sib->data(0, ITEM_KIND_ROLE).toInt() != ITEM_GROUP) break;
        if (sib->text(0).localeAwareCompare(groupName) > 0) break;
        insertAt++;
    }
    tree->insertTopLevelItem(insertAt, item);
    return item;
}

QTreeWidgetItem* MainWindow::addNoteItem(const QString& groupName, const QString& noteName,
                                        uint createTime, uint modifyTime)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, noteName);
    item->setData(0, ITEM_KIND_ROLE, ITEM_NOTE);
    item->setData(0, FILE_CREATE_TIME, createTime);
    item->setData(0, FILE_MODIFY_TIME, modifyTime);
    item->setIcon(0, QIcon(":/images/note.svg"));
    if (groupName.isEmpty())
    {
        ui->treeWidgetFile->addTopLevelItem(item);
    }
    else
    {
        QTreeWidgetItem* group = findGroupItem(groupName);
        if (!group) group = addGroupItem(groupName);
        group->addChild(item);
    }
    return item;
}

void MainWindow::keepTreeAlwaysActive()
{
    QPalette p = QApplication::palette();
    const QPalette::ColorRole roles[] = {
        QPalette::Text, QPalette::WindowText, QPalette::Base,
        QPalette::AlternateBase, QPalette::Button, QPalette::ButtonText
    };
    for (QPalette::ColorRole role : roles)
    {
        p.setColor(QPalette::Inactive, role, p.color(QPalette::Active, role));
    }
    p.setColor(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
    p.setColor(QPalette::Inactive, QPalette::HighlightedText,
               p.color(QPalette::Active, QPalette::Text));
    ui->treeWidgetFile->setPalette(p);
}

void MainWindow::initNoteBook()
{
    ui->treeWidgetFile->clear();

    QString folderPath = notebookPath(m_notebook);
    QDir().mkpath(folderPath);

    QDir bookDir(folderPath);
    QStringList groupDirs = bookDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name);
    foreach (const QString& groupName, groupDirs)
    {
        addGroupItem(groupName);

        QDir groupDir(groupPath(m_notebook, groupName));
        groupDir.setNameFilters(QStringList() << "*.enote");
        QFileInfoList groupFiles = groupDir.entryInfoList(QDir::Files);
        std::sort(groupFiles.begin(), groupFiles.end(),
                  [this](const QFileInfo& a, const QFileInfo& b){ return noteLessThan(a,b,m_sort_type); });
        foreach (const QFileInfo& fi, groupFiles)
        {
            addNoteItem(groupName, fi.completeBaseName(),
                        fi.birthTime().toTime_t(), fi.lastModified().toTime_t());
        }
    }

    QDir rootDir(folderPath);
    rootDir.setNameFilters(QStringList() << "*.enote");
    QFileInfoList rootFiles = rootDir.entryInfoList(QDir::Files);
    std::sort(rootFiles.begin(), rootFiles.end(),
              [this](const QFileInfo& a, const QFileInfo& b){ return noteLessThan(a,b,m_sort_type); });
    foreach (const QFileInfo& fi, rootFiles)
    {
        addNoteItem(QString(), fi.completeBaseName(),
                    fi.birthTime().toTime_t(), fi.lastModified().toTime_t());
    }

    bool noNotes = rootFiles.isEmpty();
    if (noNotes)
    {
        foreach (const QString& groupName, groupDirs)
        {
            QDir groupDir(groupPath(m_notebook, groupName));
            groupDir.setNameFilters(QStringList() << "*.enote");
            if (!groupDir.entryList(QDir::Files).isEmpty())
            {
                noNotes = false;
                break;
            }
        }
    }

    if (noNotes)
    {
        NoteWidget* defaultNote = new NoteWidget(ui->tabWidgetNote,m_notebook,QString(),tr("默认页"),m_editor_font,m_default_pen,m_default_paper);
        ui->tabWidgetNote->addTab(defaultNote,tr("默认页"));
        addNoteItem(QString(), tr("默认页"),
                    QDateTime::currentDateTime().toTime_t(),
                    QDateTime::currentDateTime().toTime_t());
    }
    else
    {
        QString noteHistory = m_setting->value("History/" + m_notebook).toString();
        if (noteHistory.isEmpty())
        {
            newTab();
        }
        else
        {
            QWidget* curWidget = nullptr;
            QString currentNote = m_setting->value("last_open_tab").toString();

            QStringList noteList = noteHistory.split('|', QString::SkipEmptyParts);
            foreach(const QString& entry, noteList)
            {
                QString group, fileName;
                int slash = entry.indexOf('/');
                if (slash >= 0)
                {
                    group = entry.left(slash);
                    fileName = entry.mid(slash + 1);
                }
                else
                {
                    fileName = entry;
                }

                QString filePath = noteFilePath(m_notebook, group, fileName);
                if (QFile::exists(filePath))
                {
                    NoteWidget* note = new NoteWidget(ui->tabWidgetNote,m_notebook,group,fileName,m_editor_font,m_default_pen,m_default_paper);
                    ui->tabWidgetNote->addTab(note,fileName);
                    if (entry == currentNote || fileName == currentNote)
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
}

void MainWindow::refreshMenu()
{
    QDir dir(notesRoot());
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
    m_setting->setValue("PosX", x());
    m_setting->setValue("PosY", y());

    int current = ui->tabWidgetNote->currentIndex();
    if (current >= 0)
    {
        NoteWidget* curWidget = (NoteWidget*)ui->tabWidgetNote->widget(current);
        if (curWidget)
        {
            m_setting->setValue("last_open_tab", tabKey(curWidget->group(), ui->tabWidgetNote->tabText(current)));
        }
    }

    m_setting->beginGroup("History");
    QStringList noteList;
    for (int i = 0; i < ui->tabWidgetNote->count(); i++)
    {
        NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(i);
        if (!widget->isEmpty())
        {
            QString fileName = ui->tabWidgetNote->tabText(i);
            noteList.append(tabKey(widget->group(), fileName));
        }
    }
    QString noteHistory = noteList.join('|');
    m_setting->setValue(m_notebook,noteHistory);
    m_setting->endGroup();

    m_setting->sync();
}

void MainWindow::restoreWindowPlacement()
{
    const bool hasPosX = m_setting->contains("PosX");
    const bool hasPosY = m_setting->contains("PosY");
    if (hasPosX && hasPosY)
    {
        move(m_setting->value("PosX").toInt(), m_setting->value("PosY").toInt());
        return;
    }

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
    {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen)
    {
        return;
    }

    const QRect availableGeometry = screen->availableGeometry();
    const QPoint centeredPos = availableGeometry.center() - rect().center();
    move(centeredPos);
}

bool MainWindow::applyGlobalShortcut(const QString& shortcut, bool showErrorMessage)
{
    if (!m_shortcut)
    {
        return false;
    }

    if (shortcut.trimmed().isEmpty())
    {
        if (showErrorMessage)
        {
            QMessageBox::warning(this,
                                 tr("快捷键设置失败"),
                                 tr("当前平台暂不支持清空后保留未注册状态，请设置一个新的快捷键。"));
        }
        return false;
    }

    const bool ok = m_shortcut->setShortcut(QKeySequence(shortcut));
    if (!ok && showErrorMessage)
    {
        QMessageBox::warning(this,
                             tr("快捷键设置失败"),
                             tr("无法注册全局快捷键，请尝试更换其他按键。"));
    }
    return ok;
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

void MainWindow::newTab(const QString& groupName)
{
    for (int i = 1; i < 100; i++)
    {
        QString newName = QString("新建页 %1").arg(i);
        if (!findNoteItem(groupName, newName))
        {
            NoteWidget* note = new NoteWidget(ui->tabWidgetNote,m_notebook,groupName,newName,m_editor_font,m_default_pen,m_default_paper);
            ui->tabWidgetNote->addTab(note,newName);
            ui->tabWidgetNote->setCurrentWidget(note);

            QTreeWidgetItem* item = addNoteItem(groupName, newName,
                                                QDateTime::currentDateTime().toTime_t(),
                                                QDateTime::currentDateTime().toTime_t());
            ui->treeWidgetFile->setCurrentItem(item);

            // 如果当前有搜索过滤，触发重新过滤以保持列表正确
            if (!ui->lineEditSearch->text().isEmpty())
            {
                sltStartFiltering();
            }

            break;
        }
    }
}

void MainWindow::renameTab(int tabIndex, QString& newName)
{
    QString oldName = ui->tabWidgetNote->tabText(tabIndex);
    QString validName = normalizedEntryName(newName);
    if (!isValidEntryName(validName))
    {
        QMessageBox::information(this, tr("提示"), tr("笔记名称不能为空，且不能包含 / \\ |"));
        return;
    }

    if (oldName == validName)
    {
        return;
    }

    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(tabIndex);
    QString group = widget ? widget->group() : QString();
    QString newNotePath = noteFilePath(m_notebook, group, validName);
    if (QFile::exists(newNotePath))
    {
        QMessageBox::information(this,tr("提示"),tr("命名笔记已存在，请换一个名字"));
        return;
    }

    widget->rename(validName);
    ui->tabWidgetNote->setTabText(tabIndex,validName);
    QTreeWidgetItem* item = findNoteItem(group, oldName);
    if (item) item->setText(0, validName);
    
    // 如果当前有搜索过滤，触发重新过滤以保持列表正确
    if (!ui->lineEditSearch->text().isEmpty())
    {
        sltStartFiltering();
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
    initNoteBook();
}

void MainWindow::sltSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    // 重置定时器，防抖处理
    m_filterTimer->stop();
    
    // 显示等待动画
    m_loadingLabel->show();
    m_loadingMovie->start();
    
    // 开始防抖计时，300ms后执行真正的过滤
    m_filterTimer->start(300);
}

void MainWindow::sltStartFiltering()
{
	QString text = ui->lineEditSearch->text().trimmed();

	if (text.isEmpty())
	{
		// 如果搜索框为空，显示所有节点
		for (int i = 0; i < ui->treeWidgetFile->topLevelItemCount(); ++i)
		{
			QTreeWidgetItem* topItem = ui->treeWidgetFile->topLevelItem(i);
			topItem->setHidden(false);
			for (int j = 0; j < topItem->childCount(); ++j)
			{
				topItem->child(j)->setHidden(false);
			}
		}
	}
	else
	{
		// 遍历并隐藏不匹配的节点，支持“文件名 + 正文纯文本”搜索
		for (int i = 0; i < ui->treeWidgetFile->topLevelItemCount(); ++i)
		{
			QTreeWidgetItem* topItem = ui->treeWidgetFile->topLevelItem(i);
			int itemKind = topItem->data(0, ITEM_KIND_ROLE).toInt();

			if (itemKind == ITEM_NOTE)
			{
				bool noteMatch = noteMatchesSearch(QString(), topItem->text(0), text);
				topItem->setHidden(!noteMatch);
				continue;
			}

			bool groupMatch = topItem->text(0).contains(text, Qt::CaseInsensitive);
			bool hasChildMatch = false;
			QString groupName = topItem->text(0);

			for (int j = 0; j < topItem->childCount(); ++j)
			{
				QTreeWidgetItem* childItem = topItem->child(j);
				bool childMatch = noteMatchesSearch(groupName, childItem->text(0), text);

				childItem->setHidden(!childMatch && !groupMatch);
				if (childMatch)
				{
					hasChildMatch = true;
				}
			}

			topItem->setHidden(!groupMatch && !hasChildMatch);
			if (hasChildMatch || groupMatch)
			{
				topItem->setExpanded(true);
			}
		}
	}

	// 过滤完成，稍微延迟一点关闭动画，确保用户能看到它闪了一下
	QTimer::singleShot(200, this, [this]()
	{
		m_loadingMovie->stop();
		m_loadingLabel->hide();
	});
}

void MainWindow::sltTreeItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item || item->data(0, ITEM_KIND_ROLE).toInt() != ITEM_NOTE) return;

    QString fileName = item->text(0);
    QString group = item->parent() ? item->parent()->text(0) : QString();
    QString key = tabKey(group, fileName);
    NoteWidget* noteItem = ui->tabWidgetNote->findChild<NoteWidget*>(key);
    if (noteItem)
    {
        ui->tabWidgetNote->setCurrentWidget(noteItem);
    }
    else
    {
        NoteWidget* note = new NoteWidget(ui->tabWidgetNote,m_notebook,group,fileName,m_editor_font,m_default_pen,m_default_paper);
        ui->tabWidgetNote->addTab(note,fileName);
        ui->tabWidgetNote->setCurrentWidget(note);
    }
}

void MainWindow::sltRemoveTab(int index)
{
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->widget(index);
    QString group = widget ? widget->group() : QString();
    QString name = ui->tabWidgetNote->tabText(index);
    ui->tabWidgetNote->removeTab(index);
    if (widget && widget->isEmpty())
    {
        QTreeWidgetItem* item = findNoteItem(group, name);
        if (item) delete item;
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

        this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
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
        if (!m_isUos && dlg.m_shortcut != m_hotkey)
        {
            if (applyGlobalShortcut(dlg.m_shortcut, true))
            {
                m_hotkey = dlg.m_shortcut;
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

        // 刷新“新建便签默认样式”，仅影响之后新建的便签。
        m_editor_font = dlg.m_defaultFont;
        m_default_pen = dlg.m_defaultPen;
        m_default_paper = dlg.m_defaultPaper;
    }
}

void MainWindow::sltKeepTop()
{
    if (ui->actionTop->isChecked())
    {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }
    else
    {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    }

    show();
}

void MainWindow::sltAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::sltActionDelete()
{
    NoteWidget* widget = (NoteWidget*)ui->tabWidgetNote->currentWidget();
    if (!widget) return;
    int index = ui->tabWidgetNote->currentIndex();
    QString name = ui->tabWidgetNote->tabText(index);
    QString group = widget->group();
    ui->tabWidgetNote->removeTab(index);
    QTreeWidgetItem* item = findNoteItem(group, name);
    if (item) delete item;
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
    newTab(currentGroup());
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
        QString notebookName = normalizedEntryName(dlg.m_newName);
        if (!isValidEntryName(notebookName))
        {
            QMessageBox::information(this, tr("提示"), tr("记事本名称不能为空，且不能包含 / \\ |"));
            return;
        }

        QString noteFolder = notebookPath(notebookName);
        if (QFile::exists(noteFolder))
        {
            QMessageBox::information(this,tr("提示"),tr("记事本已经存在"));
            return;
        }

        QDir dir;
        dir.mkpath(noteFolder);

        ui->comboBox->addItem(notebookName);
        ui->comboBox->setCurrentText(notebookName);
    }
}

void MainWindow::sltComboActionRename()
{
    RenameDialog dlg(ui->comboBox->currentText(),this);
    dlg.setWindowTitle("改名");
    if (dlg.exec() == QDialog::Accepted)
    {
        QString notebookName = normalizedEntryName(dlg.m_newName);
        if (!isValidEntryName(notebookName))
        {
            QMessageBox::information(this, tr("提示"), tr("记事本名称不能为空，且不能包含 / \\ |"));
            return;
        }

        if (notebookName == ui->comboBox->currentText())
        {
            return;
        }

        QString oldPath = notebookPath(ui->comboBox->currentText());
        QString newPath = notebookPath(notebookName);

        if (QFile::exists(newPath))
        {
            QMessageBox::information(this,tr("提示"),tr("命名记事本已存在，请换一个名字"));
            return;
        }

        QFile::rename(oldPath,newPath);

        QString keyName = QString("/History/%1").arg(ui->comboBox->currentText());
        QString history = m_setting->value(keyName).toString();
        m_setting->remove(keyName);
        keyName = QString("/History/%1").arg(notebookName);
        m_setting->setValue(keyName,history);

        m_notebook = notebookName;

        QAction* renameAction = m_tabMoveMenu->findChild<QAction*>(ui->comboBox->currentText());
        if (renameAction)
        {
            renameAction->setText(notebookName);
        }

        renameAction = m_listMoveMenu->findChild<QAction*>(ui->comboBox->currentText());
        if (renameAction)
        {
            renameAction->setText(notebookName);
        }

        int curIndex = ui->comboBox->currentIndex();
        ui->comboBox->setItemText(curIndex,notebookName);
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
        const QString notebookName = ui->comboBox->currentText();
        const QString notePath = notebookPath(notebookName);

        int index = ui->comboBox->currentIndex();
        ui->comboBox->removeItem(index);
        m_setting->remove(QString("/History/%1").arg(notebookName));

        QDir notebookDir(notePath);
        if (notebookDir.isEmpty())
        {
            notebookDir.rmdir(notePath);
        }
        else
        {
            moveToTrash(notePath);
        }
    }
}

void MainWindow::sltListActonDelete()
{
    QTreeWidgetItem* item = ui->treeWidgetFile->currentItem();
    if (!item)
    {
        return;
    }

    if (item->data(0, ITEM_KIND_ROLE).toInt() == ITEM_GROUP)
    {
        sltGroupActionDelete();
        return;
    }

    QString noteTitle = item->text(0);
    QString group = item->parent() ? item->parent()->text(0) : QString();
    QString key = tabKey(group, noteTitle);
    NoteWidget* noteItem = ui->tabWidgetNote->findChild<NoteWidget*>(key);
    if (noteItem)
    {
        delete item;
        int index = ui->tabWidgetNote->indexOf(noteItem);
        ui->tabWidgetNote->removeTab(index);
        noteItem->deletefile();
        delete noteItem;
    }
    else
    {
        delete item;
        QString filePath = noteFilePath(m_notebook, group, noteTitle);
        if (QFileInfo(filePath).size() == 0)
        {
            QFile::remove(filePath);
        }
        else
        {
            moveToTrash(filePath);
        }
    }
}

void MainWindow::sltCurrentIndexChanged(const QString &text)
{
    save();
    m_notebook = text;
    ui->lineEditSearch->clear(); // 切换记事本时清空搜索框
    closeAllTabs();
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

    int index = ui->tabWidgetNote->indexOf(noteItem);
    QString noteTitle = ui->tabWidgetNote->tabText(index);
    QString srcGroup = noteItem->group();

    QString srcPath = noteFilePath(m_notebook, srcGroup, noteTitle);
    QString dstPath = noteFilePath(noteBookName, QString(), noteTitle);

    if (QFile::exists(dstPath))
    {
        QMessageBox::information(this,tr("提示"),tr("目标记事本已经包含有同名笔记，无法移动"));
        return;
    }

    QTreeWidgetItem* item = findNoteItem(srcGroup, noteTitle);
    if (item) delete item;

    ui->tabWidgetNote->removeTab(index);
    delete noteItem;

    QDir().mkpath(notebookPath(noteBookName));
    QFile::rename(srcPath,dstPath);
}

void MainWindow::sltListActionMove()
{
    QAction* action = static_cast<QAction*>(sender());
    QString noteBookName = action->text();

    QTreeWidgetItem* item = ui->treeWidgetFile->currentItem();
    if (!item || item->data(0, ITEM_KIND_ROLE).toInt() != ITEM_NOTE)
    {
        QMessageBox::information(this,tr("提示"),tr("请选择一个笔记进行移动"));
        return;
    }

    QString noteTitle = item->text(0);
    QString srcGroup = item->parent() ? item->parent()->text(0) : QString();

    QString srcPath = noteFilePath(m_notebook, srcGroup, noteTitle);
    QString dstPath = noteFilePath(noteBookName, QString(), noteTitle);

    if (QFile::exists(dstPath))
    {
        QMessageBox::information(this,tr("提示"),tr("目标记事本已经包含有同名笔记，无法移动"));
        return;
    }

    QString key = tabKey(srcGroup, noteTitle);
    NoteWidget* noteItem = ui->tabWidgetNote->findChild<NoteWidget*>(key);
    if (noteItem)
    {
        int index = ui->tabWidgetNote->indexOf(noteItem);
        ui->tabWidgetNote->removeTab(index);
        delete noteItem;
    }

    delete item;

    QDir().mkpath(notebookPath(noteBookName));
    QFile::rename(srcPath,dstPath);
}

void MainWindow::sltListActionGroupMove()
{
    QAction* action = static_cast<QAction*>(sender());
    QString targetGroup = action->data().toString();
    moveNoteToGroup(ui->treeWidgetFile->currentItem(), targetGroup);
}

void MainWindow::sltNoteDropped(QTreeWidgetItem* noteItem, const QString& targetGroup)
{
    moveNoteToGroup(noteItem, targetGroup);
}

void MainWindow::moveNoteToGroup(QTreeWidgetItem* item, const QString& targetGroup)
{
    if (!item || item->data(0, ITEM_KIND_ROLE).toInt() != ITEM_NOTE) return;

    QString noteTitle = item->text(0);
    QString srcGroup = item->parent() ? item->parent()->text(0) : QString();
    if (srcGroup == targetGroup) return;

    QString srcPath = noteFilePath(m_notebook, srcGroup, noteTitle);
    QString dstPath = noteFilePath(m_notebook, targetGroup, noteTitle);
    if (QFile::exists(dstPath))
    {
        QMessageBox::information(this,tr("提示"),tr("目标分组已经包含有同名笔记，无法移动"));
        return;
    }

    QDir().mkpath(groupPath(m_notebook, targetGroup));
    if (!QFile::rename(srcPath, dstPath))
    {
        QMessageBox::information(this,tr("提示"),tr("移动失败"));
        return;
    }

    QString key = tabKey(srcGroup, noteTitle);
    NoteWidget* noteItem = ui->tabWidgetNote->findChild<NoteWidget*>(key);
    if (noteItem)
    {
        noteItem->setGroup(targetGroup);
        noteItem->rename(noteTitle);
    }

    delete item;
    uint now = QDateTime::currentDateTime().toTime_t();
    QFileInfo fi(dstPath);
    QTreeWidgetItem* newItem = addNoteItem(targetGroup, noteTitle,
                                           fi.birthTime().isValid() ? fi.birthTime().toTime_t() : now,
                                           fi.lastModified().isValid() ? fi.lastModified().toTime_t() : now);
    if (newItem && newItem->parent()) newItem->parent()->setExpanded(true);
    ui->treeWidgetFile->setCurrentItem(newItem);
}

void MainWindow::sltGroupActionNew()
{
    RenameDialog dlg(this);
    dlg.setWindowTitle(tr("新建分组"));
    if (dlg.exec() != QDialog::Accepted) return;

    QString name = normalizedEntryName(dlg.m_newName);
    if (!isValidEntryName(name))
    {
        QMessageBox::information(this, tr("提示"), tr("分组名称不能为空，且不能包含 / \\ |"));
        return;
    }

    QString gp = groupPath(m_notebook, name);
    if (QFile::exists(gp))
    {
        QMessageBox::information(this,tr("提示"),tr("分组已存在"));
        return;
    }

    QDir().mkpath(gp);
    QTreeWidgetItem* item = addGroupItem(name);
    ui->treeWidgetFile->setCurrentItem(item);
    
    // 如果当前有搜索过滤，触发重新过滤以保持列表正确
    if (!ui->lineEditSearch->text().isEmpty())
    {
        sltStartFiltering();
    }
}

void MainWindow::sltGroupActionRename()
{
    QTreeWidgetItem* item = ui->treeWidgetFile->currentItem();
    if (!item || item->data(0, ITEM_KIND_ROLE).toInt() != ITEM_GROUP) return;

    QString oldName = item->text(0);
    RenameDialog dlg(oldName, this);
    dlg.setWindowTitle(tr("重命名分组"));
    if (dlg.exec() != QDialog::Accepted) return;

    QString newName = normalizedEntryName(dlg.m_newName);
    if (!isValidEntryName(newName))
    {
        QMessageBox::information(this, tr("提示"), tr("分组名称不能为空，且不能包含 / \\ |"));
        return;
    }

    if (newName == oldName)
    {
        return;
    }

    QString newPath = groupPath(m_notebook, newName);
    if (QFile::exists(newPath))
    {
        QMessageBox::information(this,tr("提示"),tr("分组已存在"));
        return;
    }

    if (!QDir().rename(groupPath(m_notebook, oldName), newPath))
    {
        QMessageBox::information(this,tr("提示"),tr("重命名失败"));
        return;
    }

    for (int i = 0; i < ui->tabWidgetNote->count(); i++)
    {
        NoteWidget* w = (NoteWidget*)ui->tabWidgetNote->widget(i);
        if (w && w->group() == oldName)
        {
            QString tab = ui->tabWidgetNote->tabText(i);
            w->setGroup(newName);
            w->rename(tab);
        }
    }

    item->setText(0, newName);
    
    // 如果当前有搜索过滤，触发重新过滤以保持列表正确
    if (!ui->lineEditSearch->text().isEmpty())
    {
        sltStartFiltering();
    }
}

void MainWindow::sltGroupActionDelete()
{
    QTreeWidgetItem* item = ui->treeWidgetFile->currentItem();
    if (!item || item->data(0, ITEM_KIND_ROLE).toInt() != ITEM_GROUP) return;

    QString groupName = item->text(0);
    int childCount = item->childCount();
    QString msg = childCount > 0
        ? tr("分组「%1」下还有 %2 条笔记，确认全部删除？").arg(groupName).arg(childCount)
        : tr("确认删除分组「%1」？").arg(groupName);

    if (QMessageBox::Yes != QMessageBox::question(this, tr("提示"), msg,
                                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No))
    {
        return;
    }

    const QString gPath = groupPath(m_notebook, groupName);

    for (int i = ui->tabWidgetNote->count() - 1; i >= 0; i--)
    {
        NoteWidget* w = (NoteWidget*)ui->tabWidgetNote->widget(i);
        if (w && w->group() == groupName)
        {
            ui->tabWidgetNote->removeTab(i);
            delete w;
        }
    }

    QDir groupDir(gPath);
    if (groupDir.isEmpty())
    {
        groupDir.rmdir(gPath);
    }
    else
    {
        moveToTrash(gPath);
    }

    delete item;
}

void MainWindow::sltGroupActionNewNote()
{
    QTreeWidgetItem* item = ui->treeWidgetFile->currentItem();
    if (!item) return;
    QString groupName = (item->data(0, ITEM_KIND_ROLE).toInt() == ITEM_GROUP)
                        ? item->text(0)
                        : (item->parent() ? item->parent()->text(0) : QString());
    newTab(groupName);
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
    QTreeWidgetItem* item = ui->treeWidgetFile->itemAt(pos);
    if (!item)
    {
        ui->treeWidgetFile->setCurrentItem(nullptr);
        m_blankMenu->popup(QCursor::pos());
        return;
    }

    ui->treeWidgetFile->setCurrentItem(item);
    if (item->data(0, ITEM_KIND_ROLE).toInt() == ITEM_GROUP)
    {
        m_groupMenu->popup(QCursor::pos());
        return;
    }

    QList<QAction*> oldActions = m_listGroupMoveMenu->actions();
    qDeleteAll(oldActions);
    m_listGroupMoveMenu->clear();

    QString currentGroup = item->parent() ? item->parent()->text(0) : QString();
    QStringList groups = listGroups();

    auto addGroupTarget = [this, currentGroup](const QString& display, const QString& data){
        QAction* a = new QAction(display, this);
        a->setData(data);
        a->setEnabled(data != currentGroup);
        connect(a, SIGNAL(triggered()), this, SLOT(sltListActionGroupMove()));
        m_listGroupMoveMenu->addAction(a);
    };

    addGroupTarget(tr("(无分组)"), QString());
    for (const QString& group : groups) addGroupTarget(group, group);

    m_listMenu->popup(QCursor::pos());
}
