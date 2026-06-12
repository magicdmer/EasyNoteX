#include "richtextedit.h"
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextImageFormat>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

QUrl RichTextEdit::originalImageResourceUrl(const QString &name) const
{
    QUrl url(name);
    url.setFragment(QStringLiteral("cutex-original"));
    return url;
}

QImage RichTextEdit::imageResource(QTextDocument *document, const QString &name) const
{
    QImage image = document->resource(QTextDocument::ImageResource, originalImageResourceUrl(name)).value<QImage>();

    if (image.isNull())
        image = document->resource(QTextDocument::ImageResource, QUrl(name)).value<QImage>();

    return image;
}

RichTextEdit::RichTextEdit(QWidget *parent):QxTextEdit(parent)
{
    m_menu = new QMenu(this);
    m_action_copy = new QAction(tr("复制"), m_menu);
    m_action_save = new QAction(tr("另存为"), m_menu);
    m_menu->addAction(m_action_copy);
    m_menu->addAction(m_action_save);

    connect(this, SIGNAL(imageRightClicked()), this, SLOT(sltImageRightClicked()));
    connect(m_action_copy, SIGNAL(triggered()), this, SLOT(sltCopyImage()));
    connect(m_action_save, SIGNAL(triggered()), this, SLOT(sltSaveImage()));

    flattenInactivePalette();
    qApp->installEventFilter(this);
}

void RichTextEdit::flattenInactivePalette()
{
    QPalette p = QApplication::palette();
    const QPalette::ColorRole roles[] = {
        QPalette::Text, QPalette::Base, QPalette::WindowText,
        QPalette::Window, QPalette::Highlight, QPalette::HighlightedText
    };
    for (QPalette::ColorRole role : roles)
    {
        p.setColor(QPalette::Inactive, role, p.color(QPalette::Active, role));
    }
    setPalette(p);
    viewport()->setPalette(p);
    viewport()->update();
}

void RichTextEdit::refreshImageResources()
{
    QTextDocument* doc = document();

    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next())
    {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
        {
            QTextFragment fragment = it.fragment();
            if (!fragment.isValid() || !fragment.charFormat().isImageFormat())
            {
                continue;
            }

            QTextImageFormat fmt = fragment.charFormat().toImageFormat();
            QString name = fmt.name();
            if (name.isEmpty())
            {
                continue;
            }

            QUrl imageUrl(name);
            QUrl originalUrl = originalImageResourceUrl(name);
            QImage image = doc->resource(QTextDocument::ImageResource, originalUrl).value<QImage>();

            if (image.isNull())
            {
                image = doc->resource(QTextDocument::ImageResource, imageUrl).value<QImage>();
                if (!image.isNull())
                {
                    doc->addResource(QTextDocument::ImageResource, originalUrl, image);
                }
            }

            if (image.isNull())
            {
                continue;
            }

            double width = fmt.width();
            double height = fmt.height();
            if (width <= 0.0)
            {
                width = image.width();
            }
            if (height <= 0.0)
            {
                height = image.height();
            }

            QSize targetSize(qMax(1, qRound(width)), qMax(1, qRound(height)));
            QImage displayImage = image;
            if (targetSize != image.size())
            {
                displayImage = image.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }

            doc->addResource(QTextDocument::ImageResource, imageUrl, displayImage);
        }
    }

    doc->markContentsDirty(0, doc->characterCount());
    viewport()->update();
}

bool RichTextEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange)
    {
        flattenInactivePalette();
    }
    return QxTextEdit::eventFilter(watched, event);
}

void RichTextEdit::sltImageRightClicked()
{
    m_menu->popup(QCursor::pos());
}

void RichTextEdit::sltCopyImage()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection() && cursor.charFormat().isImageFormat()) {
        QTextImageFormat fmt = cursor.charFormat().toImageFormat();

        QImage image = imageResource(document(), fmt.name());

        QClipboard* clip = QApplication::clipboard();
        clip->setImage(image);
    }
}

void RichTextEdit::sltSaveImage()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection() && cursor.charFormat().isImageFormat()) {
        QTextImageFormat fmt = cursor.charFormat().toImageFormat();

        QImage image = imageResource(document(), fmt.name());

        QString filePath = QFileDialog::getSaveFileName(this, tr("图片另存为"), tr("图片"), tr("Image (*.png)"));
        if (filePath.isEmpty())
        {
            return;
        }

        image.save(filePath, "png");
    }
}

void RichTextEdit::dealBackTab()
{
    QTextCursor cur = textCursor();
    int pos = cur.position();
    int anchor = cur.anchor();

    cur.setPosition(pos);

    cur.setPosition(pos-1,QTextCursor::KeepAnchor);

    if (cur.selectedText() == "\t")
    {
        cur.removeSelectedText();
        cur.setPosition(anchor-1);
        cur.setPosition(pos-1,QTextCursor::KeepAnchor);
    }
    else
    {
        cur.setPosition(anchor);
        cur.setPosition(anchor-1,QTextCursor::KeepAnchor);
        if (cur.selectedText() == "\t")
        {
            cur.removeSelectedText();
            cur.setPosition(anchor-1);
            cur.setPosition(pos-1,QTextCursor::KeepAnchor);
        }
        else
        {
            cur.setPosition(anchor);
            cur.setPosition(pos,QTextCursor::KeepAnchor);
        }
    }
}
