#include "notewidget.h"
#include "ui_notewidget.h"
#include "helpfunc.h"
#include <QFile>
#include <QSettings>
#include <QDomDocument>
#include <QImage>
#include <QDir>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

NoteWidget::NoteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NoteWidget)
{
    ui->setupUi(this);
    m_textEdit = new RichTextEdit(this);
    ui->verticalLayout->addWidget(m_textEdit);

    m_textChanged = false;

    m_typingTimer = new QTimer( this );
    m_typingTimer->setSingleShot( true );
    connect(m_typingTimer, SIGNAL(timeout()),this, SLOT(sltFilterEntries()));

    connect(m_textEdit,SIGNAL(textChanged()),this,SLOT(sltTextChanged()));
}

NoteWidget::NoteWidget(QWidget *parent,QString noteName, QString fileName, QFont font) :
    NoteWidget(parent, noteName, QString(), fileName, font)
{
}

NoteWidget::NoteWidget(QWidget *parent,QString noteName, QString groupName, QString fileName, QFont font) :
    QWidget(parent),
    ui(new Ui::NoteWidget)
{
    ui->setupUi(this);

    m_textChanged = false;

    m_textEdit = new RichTextEdit(this);
    ui->verticalLayout->addWidget(m_textEdit);

    m_noteName = noteName;
    m_group = groupName;

    setTextFont(font);
    setFile(fileName);

    m_typingTimer = new QTimer( this );
    m_typingTimer->setSingleShot( true );
    connect(m_typingTimer, SIGNAL(timeout()),this, SLOT(sltFilterEntries()));

    connect(m_textEdit,SIGNAL(textChanged()),this,SLOT(sltTextChanged()));
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
        QImage image = m_textEdit->document()->
                resource(QTextDocument::ImageResource,imgUrl).value<QImage>();

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

void NoteWidget::setCurrentFont(QFont& font)
{
    m_textEdit->setCurrentFont(font);
}

void NoteWidget::setFontColor(QColor &color)
{
    m_textEdit->setTextColor(color);
}

void NoteWidget::setBgColor(QPalette &palette)
{
    m_textEdit->setPalette(palette);

    QString fileContent = m_textEdit->toHtml();

    QDomDocument doc;
    doc.setContent(fileContent.toUtf8());

    QDomNodeList bodyNodeList = doc.elementsByTagName("body");
    QDomNode bodyNode = bodyNodeList.at(0);
    QDomElement bodyEle = bodyNode.toElement();
    QString bgColor = m_textEdit->palette().color(QPalette::Base).name();
    bodyEle.setAttribute("bgcolor",bgColor);

    m_textEdit->setHtml(doc.toString(0));
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
