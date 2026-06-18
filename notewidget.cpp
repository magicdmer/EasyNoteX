#include "notewidget.h"
#include "ui_notewidget.h"
#include "helpfunc.h"
#include <QFile>
#include <QSettings>
#include <QDomDocument>
#include <QImage>
#include <QDir>
#include <QBuffer>
#include <QCryptographicHash>
#include <QHBoxLayout>
#include <QFontComboBox>
#include <QSpinBox>
#include <QToolButton>
#include <QColorDialog>
#include <QFrame>
#include <QFontInfo>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPalette>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

QUrl NoteWidget::originalImageResourceUrl(const QString &name) const
{
    QUrl url(name);
    url.setFragment(QStringLiteral("cutex-original"));
    return url;
}

QImage NoteWidget::imageResource(QTextDocument *document, const QString &name) const
{
    QImage image = document->resource(QTextDocument::ImageResource, originalImageResourceUrl(name)).value<QImage>();

    if (image.isNull())
        image = document->resource(QTextDocument::ImageResource, QUrl(name)).value<QImage>();

    return image;
}

NoteWidget::NoteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NoteWidget)
{
    ui->setupUi(this);
    m_textEdit = new RichTextEdit(this);
    buildToolbar();

    m_textChanged = false;

    m_typingTimer = new QTimer( this );
    m_typingTimer->setSingleShot( true );
    connect(m_typingTimer, SIGNAL(timeout()),this, SLOT(sltFilterEntries()));

    connect(m_textEdit,SIGNAL(textChanged()),this,SLOT(sltTextChanged()));

    syncToolbar();
}

NoteWidget::NoteWidget(QWidget *parent,QString noteName, QString fileName, QFont font,
                       QColor penColor, QColor paperColor) :
    NoteWidget(parent, noteName, QString(), fileName, font, penColor, paperColor)
{
}

NoteWidget::NoteWidget(QWidget *parent,QString noteName, QString groupName, QString fileName, QFont font,
                       QColor penColor, QColor paperColor) :
    QWidget(parent),
    ui(new Ui::NoteWidget)
{
    ui->setupUi(this);

    m_textChanged = false;

    m_textEdit = new RichTextEdit(this);
    buildToolbar();

    m_noteName = noteName;
    m_group = groupName;
    m_penColor = penColor;
    m_paperColor = paperColor;

    setTextFont(font);
    setFile(fileName);

    // 新便签套用全局默认笔色/纸色；已有便签的样式由 load() 从内容还原。
    if (isEmpty())
    {
        if (penColor.isValid()) setPenColor(penColor);
        if (paperColor.isValid()) setPaperColor(paperColor);
    }

    m_typingTimer = new QTimer( this );
    m_typingTimer->setSingleShot( true );
    connect(m_typingTimer, SIGNAL(timeout()),this, SLOT(sltFilterEntries()));

    connect(m_textEdit,SIGNAL(textChanged()),this,SLOT(sltTextChanged()));

    syncToolbar();
}

NoteWidget::~NoteWidget()
{
    if (!m_textEdit->toPlainText().isEmpty())
    {
        if (m_textChanged)
            save();
    }
    else
    {
        QFile::remove(m_filePath);
    }

    delete ui;
}

void NoteWidget::buildToolbar()
{
    QWidget* bar = new QWidget(this);
    QHBoxLayout* h = new QHBoxLayout(bar);
    h->setContentsMargins(4, 4, 4, 4);
    h->setSpacing(6);

    m_fontCombo = new QFontComboBox(bar);
    m_fontCombo->setMaximumWidth(150);
    m_fontCombo->setFixedHeight(26);
    m_fontCombo->setToolTip(tr("字体（整篇）"));

    m_sizeSpin = new QSpinBox(bar);
    m_sizeSpin->setRange(6, 96);
    m_sizeSpin->setMaximumWidth(56);
    m_sizeSpin->setFixedHeight(27);
    m_sizeSpin->setToolTip(tr("字号（整篇）"));

    m_boldBtn = new QToolButton(bar);
    m_boldBtn->setText(tr("B"));
    m_boldBtn->setCheckable(true);
    m_boldBtn->setFixedSize(26, 26);
    QFont boldFont = m_boldBtn->font();
    boldFont.setBold(true);
    m_boldBtn->setFont(boldFont);
    m_boldBtn->setToolTip(tr("加粗（整篇）"));

    m_penBtn = new QToolButton(bar);
    m_penBtn->setFixedSize(26, 26);
    m_penBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_penBtn->setIconSize(QSize(16, 16));
    m_penBtn->setToolTip(tr("文字颜色（整篇）"));

    m_paperBtn = new QToolButton(bar);
    m_paperBtn->setFixedSize(26, 26);
    m_paperBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_paperBtn->setIconSize(QSize(16, 16));
    m_paperBtn->setToolTip(tr("背景颜色（整篇）"));

    h->addWidget(m_fontCombo);
    h->addWidget(m_sizeSpin);
    h->addWidget(m_boldBtn);
    h->addWidget(m_penBtn);
    h->addWidget(m_paperBtn);
    h->addStretch(1);

    m_textEdit->setFrameShape(QFrame::NoFrame);
    // 只保留一个舒服的正文内边距，不破坏扁平感
    m_textEdit->document()->setDocumentMargin(12);

    ui->verticalLayout->addWidget(bar);
    ui->verticalLayout->addWidget(m_textEdit);

    connect(m_fontCombo, SIGNAL(currentFontChanged(QFont)), this, SLOT(sltFontFamilyChanged(QFont)));
    connect(m_sizeSpin, SIGNAL(valueChanged(int)), this, SLOT(sltFontSizeChanged(int)));
    connect(m_boldBtn, SIGNAL(toggled(bool)), this, SLOT(sltBoldToggled(bool)));
    connect(m_penBtn, SIGNAL(clicked()), this, SLOT(sltPickPenColor()));
    connect(m_paperBtn, SIGNAL(clicked()), this, SLOT(sltPickPaperColor()));
}

void NoteWidget::sltFilterEntries()
{
    if (m_textEdit->toPlainText().isEmpty())
    {
        m_textChanged = false;
        return;
    }

    if (m_filterText.count(" ") == m_filterText.size())
    {
        m_textChanged = false;
        return;
    }

    save();

    m_textChanged = false;
}

void NoteWidget::sltTextChanged()
{
    m_textChanged = true;
    m_filterText = m_textEdit->toPlainText();
    m_typingTimer->start( 10000 );
}

void NoteWidget::sltFontFamilyChanged(const QFont& font)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(font.family());
    applyCharFormatToWholeNote(fmt);
    m_textEdit->setFocus();
}

void NoteWidget::sltFontSizeChanged(int size)
{
    if (size <= 0)
    {
        return;
    }

    QTextCharFormat fmt;
    fmt.setFontPointSize(size);
    applyCharFormatToWholeNote(fmt);
}

void NoteWidget::sltBoldToggled(bool bold)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    applyCharFormatToWholeNote(fmt);
    m_textEdit->setFocus();
}

void NoteWidget::sltPickPenColor()
{
    QColor init = m_penColor.isValid() ? m_penColor : QColor(Qt::black);
    QColor color = QColorDialog::getColor(init, this, tr("文字颜色"));
    if (color.isValid())
    {
        setPenColor(color);
    }
}

void NoteWidget::sltPickPaperColor()
{
    QColor init = m_paperColor.isValid() ? m_paperColor : QColor(Qt::white);
    QColor color = QColorDialog::getColor(init, this, tr("背景颜色"));
    if (color.isValid())
    {
        setPaperColor(color);
    }
}

void NoteWidget::applyCharFormatToWholeNote(const QTextCharFormat& fmt)
{
    QTextCursor cursor = m_textEdit->textCursor();
    int pos = cursor.position();
    int anchor = cursor.anchor();

    QTextCursor all = m_textEdit->textCursor();
    all.select(QTextCursor::Document);
    all.mergeCharFormat(fmt);
    // 同时合并块级字符格式，覆盖空段落，否则在空段落里新输入会沿用旧格式（如粗体取消不掉）。
    all.mergeBlockCharFormat(fmt);

    QTextCursor restore = m_textEdit->textCursor();
    restore.setPosition(anchor);
    restore.setPosition(pos, QTextCursor::KeepAnchor);
    m_textEdit->setTextCursor(restore);

    // 让后续输入也沿用该格式。
    m_textEdit->mergeCurrentCharFormat(fmt);

    // 整篇格式变化后强制重排并重绘，否则可能出现“文字消失、需缩放窗口才刷新”的问题。
    QTextDocument* doc = m_textEdit->document();
    doc->markContentsDirty(0, doc->characterCount());
    m_textEdit->viewport()->update();
}

bool NoteWidget::load()
{
    if (m_filePath.isEmpty())
    {
        return false;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }
    QString fileContent = QString::fromUtf8(file.readAll());
    file.close();

    QDomDocument doc;
    doc.setContent(fileContent.toUtf8());

    QDomNodeList imgNodeList = doc.elementsByTagName("img");
    for (int i = 0; i < imgNodeList.count(); i++)
    {
        QDomNode node = imgNodeList.at(i);
        QDomElement imgEle = node.toElement();
        QString imgBase64Data = imgEle.attribute("src").mid(sizeof("data:image/png;base64,") - 1);

        QByteArray array = QByteArray::fromBase64(imgBase64Data.toLatin1());

        QImage image;
        image.loadFromData(array,"png");

        QCryptographicHash Hash(QCryptographicHash::Sha1);   //此处采用Sha1,若有不同可自行选择
        Hash.addData(array);
        QByteArray HASH1 = Hash.result();
        QString fileHash = HASH1.toHex();

        QString url = "image://" + fileHash + ".png";

        m_textEdit->document()->addResource(QTextDocument::ImageResource, QUrl(url), image);

        imgEle.setAttribute("src",url);
    }

    QString content = doc.toString(0);
    m_textEdit->setHtml(content);
    m_textEdit->refreshImageResources();

    // 还原该便签保存的纸色（body bgcolor）到编辑器调色板。
    QDomNodeList bodyNodeList = doc.elementsByTagName("body");
    if (!bodyNodeList.isEmpty())
    {
        QString bg = bodyNodeList.at(0).toElement().attribute("bgcolor");
        QColor paper(bg);
        if (paper.isValid())
        {
            QPalette palette = m_textEdit->palette();
            palette.setColor(QPalette::Base, paper);
            m_textEdit->setPalette(palette);
            m_paperColor = paper;
        }
    }

    return true;
}

bool NoteWidget::save()
{
    if (m_filePath.isEmpty())
    {
        return false;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QString fileContent = m_textEdit->toHtml();

    QDomDocument doc;
    doc.setContent(fileContent.toUtf8());

    QDomNodeList imgNodeList = doc.elementsByTagName("img");
    for (int i = 0; i < imgNodeList.count(); i++)
    {
        QDomNode node = imgNodeList.at(i);
        QDomElement imgEle = node.toElement();
        QString imgUrl = imgEle.attribute("src");
        QImage image = imageResource(m_textEdit->document(), imgUrl);

        QByteArray array;
        QBuffer buffer(&array);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer,"png");
        QString imageBase64 = array.toBase64();

        imgEle.setAttribute("src","data:image/png;base64," + imageBase64);
    }

    QByteArray contentArray = doc.toByteArray(0);
    file.write(contentArray);
    file.close();

    return true;
}

bool NoteWidget::setFile(QString &fileName)
{
    setObjectName(m_group.isEmpty() ? fileName : (m_group + QLatin1Char('/') + fileName));
    QDir().mkpath(groupPath(m_noteName, m_group));
    m_filePath = noteFilePath(m_noteName, m_group, fileName);
    if (!QFile::exists(m_filePath))
    {
        return save();
    }
    else
    {
        return load();
    }
}

bool NoteWidget::rename(QString &newName)
{
    QString newFilePath = noteFilePath(m_noteName, m_group, newName);
    if (QFile::exists(m_filePath) && !QFile::rename(m_filePath,newFilePath))
    {
        return false;
    }

    setObjectName(m_group.isEmpty() ? newName : (m_group + QLatin1Char('/') + newName));
    m_filePath = newFilePath;

    return true;
}

bool NoteWidget::find(QString &text, QTextDocument::FindFlags flags)
{
    if(!m_textEdit->find(text,flags))
    {
        if (flags & QTextDocument::FindBackward)
        {
            m_textEdit->moveCursor(QTextCursor::End);
        }
        else
        {
            m_textEdit->moveCursor(QTextCursor::Start);
        }

        if (!m_textEdit->find(text,flags))
        {
            return false;
        }
    }

    QPalette palette = m_textEdit->palette();
    palette.setColor(QPalette::Highlight,palette.color(QPalette::Active,QPalette::Highlight));
    m_textEdit->setPalette(palette);

    return true;
}

bool NoteWidget::save(QString& filePath, int type)
{
    if (!type)
    {
        save();
        QFile::copy(m_filePath, filePath);
    }
    else
    {
        QString fileContent;
        fileContent = m_textEdit->toPlainText();

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly))
        {
            return false;
        }

        file.write(fileContent.toUtf8());
        file.close();
    }

    return true;
}

void NoteWidget::setTextFont(QFont &font)
{
    m_textEdit->setFont(font);
    m_textEdit->setCurrentFont(font);

    QSettings setting(settingsFile(),QSettings::IniFormat,this);
    int tabWidth = setting.value("tab_width",4).toInt();
    setTabWidth(tabWidth);
}

void NoteWidget::setPenColor(const QColor& color)
{
    if (!color.isValid())
    {
        return;
    }

    m_penColor = color;

    QTextCharFormat fmt;
    fmt.setForeground(color);
    applyCharFormatToWholeNote(fmt);

    updateColorButton(m_penBtn, color, false);
}

void NoteWidget::setPaperColor(const QColor& color)
{
    if (!color.isValid())
    {
        return;
    }

    m_paperColor = color;

    QPalette palette = m_textEdit->palette();
    palette.setColor(QPalette::Base, color);
    m_textEdit->setPalette(palette);

    writePaperToHtml(color);
    updateColorButton(m_paperBtn, color, true);

    m_textChanged = true;
}

void NoteWidget::writePaperToHtml(const QColor& color)
{
    QString fileContent = m_textEdit->toHtml();

    QDomDocument doc;
    doc.setContent(fileContent.toUtf8());

    QDomNodeList bodyNodeList = doc.elementsByTagName("body");
    if (bodyNodeList.isEmpty())
    {
        return;
    }

    QDomElement bodyEle = bodyNodeList.at(0).toElement();
    bodyEle.setAttribute("bgcolor", color.name());

    m_textEdit->setHtml(doc.toString(0));
    m_textEdit->refreshImageResources();
}

void NoteWidget::syncToolbar()
{
    m_textEdit->moveCursor(QTextCursor::Start);
    QFont f = m_textEdit->currentFont();

    m_fontCombo->blockSignals(true);
    m_sizeSpin->blockSignals(true);
    m_fontCombo->setCurrentFont(f);
    int pt = f.pointSize();
    if (pt <= 0)
    {
        pt = QFontInfo(f).pointSize();
    }
    m_sizeSpin->setValue(pt);
    m_fontCombo->blockSignals(false);
    m_sizeSpin->blockSignals(false);

    m_boldBtn->blockSignals(true);
    m_boldBtn->setChecked(f.bold());
    m_boldBtn->blockSignals(false);

    QColor pen = m_textEdit->textColor();
    if (pen.isValid())
    {
        m_penColor = pen;
    }
    QColor paper = m_textEdit->palette().color(QPalette::Base);
    if (paper.isValid())
    {
        m_paperColor = paper;
    }
    updateColorButton(m_penBtn, m_penColor, false);
    updateColorButton(m_paperBtn, m_paperColor, true);
}

void NoteWidget::updateColorButton(QToolButton* button, const QColor& color, bool isBackground)
{
    const int size = 16;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const QColor swatch = color.isValid() ? color : QColor(Qt::white);

    QFont f = button->font();
    f.setPixelSize(11);
    f.setBold(true);
    p.setFont(f);

    if (isBackground)
    {
        // 背景色：实心色块上压一个对比色的“A”，色块颜色实时反映当前背景色。
        const QRectF block(0.5, 0.5, size - 1, size - 1);
        p.fillRect(block, swatch);
        p.setPen(QColor(180, 180, 180));
        p.drawRect(block);
        // 依据色块亮度选黑/白字，保证“A”始终可读。
        const int lum = (swatch.red() * 299 + swatch.green() * 587 + swatch.blue() * 114) / 1000;
        p.setPen(lum > 128 ? QColor(Qt::black) : QColor(Qt::white));
        p.drawText(block, Qt::AlignCenter, QStringLiteral("A"));
    }
    else
    {
        // 文字色：常规色“A” + 底部一条反映当前文字颜色的粗下划线。
        p.setPen(button->palette().color(QPalette::ButtonText));
        p.drawText(QRectF(0, 0, size, size - 4), Qt::AlignCenter, QStringLiteral("A"));
        p.fillRect(QRectF(2, size - 3, size - 4, 3), swatch);
    }

    p.end();
    button->setIcon(QIcon(pix));
}

void NoteWidget::setTabWidth(int width)
{
    QFontMetrics metrics(m_textEdit->font());
    m_textEdit->setTabStopWidth(width * metrics.width(' '));
}

bool NoteWidget::isEmpty()
{
    return m_textEdit->toPlainText().isEmpty();
}

void NoteWidget::deletefile()
{
    if (!m_textEdit->toPlainText().isEmpty())
    {
        moveToTrash(m_filePath);
    }

    m_textEdit->clear();
}

void NoteWidget::dealBackTab()
{
    m_textEdit->dealBackTab();
}

void NoteWidget::insertTable(int row, int col, int percent)
{
    QTextTableFormat xFmt;
    xFmt.setCellPadding(3);
    xFmt.setCellSpacing(0);
    xFmt.setBorder(1.0);
    xFmt.setBorderStyle(QTextTableFormat::BorderStyle_Solid);
    xFmt.setBorderBrush(QBrush(Qt::gray));
    xFmt.setWidth(QTextLength(QTextLength::PercentageLength, percent));

    m_textEdit->insertTable(row, col, xFmt);
}
