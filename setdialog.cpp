#include "setdialog.h"
#include "ui_setdialog.h"
#include "helpfunc.h"
#include <QDir>
#include <QIntValidator>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

SetDialog::SetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);

    QStringList sortTypeList;
    sortTypeList << tr("按名称排序") << tr("按创建时间升序") << tr("按创建时间降序")
                 << tr("按修改时间升序") << tr("按修改时间降序");

    ui->comboBox->insertItems(0,sortTypeList);

    ui->lineEditTabWidth->setValidator( new QIntValidator(ui->lineEditTabWidth) );
    m_setting = new QSettings(settingsFile(),QSettings::IniFormat,this);
    m_setting->setIniCodec("UTF-8");

    int closeToTray = m_setting->value("close_to_tray", 0).toInt();
    int minToTray = m_setting->value("minimize_to_tray", 0).toInt();
    int escToTray = m_setting->value("esc_to_tray").toInt();
    int tabWidth = m_setting->value("tab_width", 4).toInt();
    int sort_type = m_setting->value("sort_type", 0).toInt();

    QString shortcut = m_setting->value("hotkey").toString();

    ui->checkBoxCloseToTray->setChecked(closeToTray);
    ui->checkBoxMiniToTray->setChecked(minToTray);
    ui->checkBoxEscToTray->setChecked(escToTray);
    ui->lineEditTabWidth->setText(QString::number(tabWidth));
    ui->keySequenceEdit->setKeySequence(QKeySequence(shortcut));
    const bool uos = isUos();
    ui->keySequenceEdit->setVisible(!uos);
    ui->labelShortcutHint->setVisible(uos);
    if (uos)
    {
        ui->groupBox_2->setTitle(tr("显示/隐藏主窗口"));
        ui->labelShortcutHint->setText(tr("uos 下请在控制中心的快捷键设置中手动设置 EasyNoteX 的启动快捷键。"));
    }

    ui->comboBox->setCurrentIndex(sort_type);

    connect(ui->pushButtonOk,SIGNAL(clicked()),this,SLOT(sltButtonOkClicked()));
    connect(ui->pushButtonCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

SetDialog::~SetDialog()
{
    delete ui;
}

void SetDialog::sltButtonOkClicked()
{
    m_closeToTray = ui->checkBoxCloseToTray->isChecked();
    m_minToTray = ui->checkBoxMiniToTray->isChecked();
    m_escToTray = ui->checkBoxEscToTray->isChecked();
    m_tabWidth = ui->lineEditTabWidth->text().toInt();
    m_shortcut = ui->keySequenceEdit->isVisible() ? ui->keySequenceEdit->keySequence().toString()
                                                  : m_setting->value("hotkey").toString();
    m_sort_type = ui->comboBox->currentIndex();

    m_setting->setValue("close_to_tray",m_closeToTray);
    m_setting->setValue("minimize_to_tray",m_minToTray);
    m_setting->setValue("tab_width",m_tabWidth);
    m_setting->setValue("esc_to_tray",m_escToTray);
    m_setting->setValue("sort_type", m_sort_type);

    accept();
}
