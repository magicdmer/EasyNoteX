#include "setdialog.h"
#include "ui_setdialog.h"
#include "helpfunc.h"
#include <QDir>
#include <QIntValidator>
#include <QColorDialog>
#include <QFontDialog>
#include <QToolButton>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

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

    // 新建便签默认样式
    m_defaultFont.fromString(m_setting->value("/Editor/font").toString());
    if (m_defaultFont.family().isEmpty())
    {
        m_defaultFont = QFont("微软雅黑", 14);
    }
    m_defaultPen = QColor(m_setting->value("/Editor/pen_color").toString());
    if (!m_defaultPen.isValid())
    {
        m_defaultPen = QColor(Qt::black);
    }
    m_defaultPaper = QColor(m_setting->value("/Editor/paper_color").toString());
    if (!m_defaultPaper.isValid())
    {
        m_defaultPaper = QColor(Qt::white);
    }

    ui->btnDefaultFont->setText(tr("%1  %2pt").arg(m_defaultFont.family())
                                .arg(m_defaultFont.pointSize() > 0 ? m_defaultFont.pointSize() : 14));
    updateColorButton(ui->btnDefaultPen, m_defaultPen);
    updateColorButton(ui->btnDefaultPaper, m_defaultPaper);

    connect(ui->btnDefaultFont,SIGNAL(clicked()),this,SLOT(sltPickDefaultFont()));
    connect(ui->btnDefaultPen,SIGNAL(clicked()),this,SLOT(sltPickDefaultPen()));
    connect(ui->btnDefaultPaper,SIGNAL(clicked()),this,SLOT(sltPickDefaultPaper()));

    connect(ui->pushButtonOk,SIGNAL(clicked()),this,SLOT(sltButtonOkClicked()));
    connect(ui->pushButtonCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

void SetDialog::updateColorButton(QToolButton* button, const QColor& color)
{
    // 与便签工具条一致：按钮上带一个表示当前颜色的实色块。
    QColor c = color.isValid() ? color : QColor(Qt::white);
    QPixmap pix(14, 14);
    QPainter p(&pix);
    p.fillRect(QRect(0, 0, 14, 14), c);
    p.setPen(QColor(120, 120, 120));
    p.drawRect(0, 0, 13, 13);
    p.end();
    button->setIcon(QIcon(pix));
    button->setToolTip(c.name());
}

void SetDialog::sltPickDefaultFont()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, m_defaultFont, this, tr("默认字体"));
    if (ok)
    {
        m_defaultFont = font;
        ui->btnDefaultFont->setText(tr("%1  %2pt").arg(m_defaultFont.family())
                                    .arg(m_defaultFont.pointSize() > 0 ? m_defaultFont.pointSize() : 14));
    }
}

void SetDialog::sltPickDefaultPen()
{
    QColor init = m_defaultPen.isValid() ? m_defaultPen : QColor(Qt::black);
    QColor color = QColorDialog::getColor(init, this, tr("默认文字颜色"));
    if (color.isValid())
    {
        m_defaultPen = color;
        updateColorButton(ui->btnDefaultPen, m_defaultPen);
    }
}

void SetDialog::sltPickDefaultPaper()
{
    QColor init = m_defaultPaper.isValid() ? m_defaultPaper : QColor(Qt::white);
    QColor color = QColorDialog::getColor(init, this, tr("默认背景颜色"));
    if (color.isValid())
    {
        m_defaultPaper = color;
        updateColorButton(ui->btnDefaultPaper, m_defaultPaper);
    }
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

    m_setting->setValue("/Editor/font", m_defaultFont.toString());
    m_setting->setValue("/Editor/pen_color", m_defaultPen.isValid() ? m_defaultPen.name() : QString());
    m_setting->setValue("/Editor/paper_color", m_defaultPaper.isValid() ? m_defaultPaper.name() : QString());

    accept();
}
