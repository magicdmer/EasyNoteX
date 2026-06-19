#include "richtextedit.h"
#include "helpfunc.h"
#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextImageFormat>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

bool RichTextEdit::blockHasChecklistPrefix(const QTextBlock &block)
{
    return textHasChecklistPrefix(block.text());
}

int RichTextEdit::checklistBodyStartOffset(const QString &text)
{
    if (!textHasChecklistPrefix(text))
    {
        return 0;
    }

    // 用户可能删除 checkbox 后面的空格，正文起点需要兼容“有空格”和“无空格”两种状态。
    if (text.size() > 1 && text.at(1) == QLatin1Char(' '))
    {
        return 2;
    }

    return 1;
}

bool RichTextEdit::blockIsEmptyChecklist(const QTextBlock &block)
{
    QString text = block.text();
    return blockHasChecklistPrefix(block) && text.size() <= checklistBodyStartOffset(text);
}

bool RichTextEdit::blockChecklistChecked(const QTextBlock &block)
{
    QString text = block.text();
    return !text.isEmpty() && text.at(0) == checklistCheckedChar();
}

QTextCharFormat RichTextEdit::checklistBodyDefaultFormat(QTextDocument *document)
{
    // 光标紧贴 checkbox 输入正文时，不能继承 checkbox 的 Segoe UI Symbol 字体。
    QTextCharFormat format;
    QFont font = document->defaultFont();
    format.setFont(font);
    format.setFontFamily(font.family());
    format.setFontFamilies(QStringList() << font.family());
    format.setFontStrikeOut(false);
    return format;
}

QRect RichTextEdit::checklistMarkerHitRect(const QTextBlock &block) const
{
    QTextCursor startCursor(document());
    startCursor.setPosition(block.position());

    QTextCursor endCursor(startCursor);
    endCursor.movePosition(QTextCursor::NextCharacter);

    QRect rect = cursorRect(startCursor).united(cursorRect(endCursor));
    rect.adjust(-4, -3, 4, 3);
    return rect;
}

void RichTextEdit::applyChecklistTextState(QTextDocument *document, const QTextBlock &block, bool checked)
{
    QString text = block.text();
    int startOffset = 1;
    if (text.size() <= startOffset)
    {
        return;
    }

    // 删除线只作用在正文非空白字符上，避免“勾选后再输入空格，取消勾选空格仍带删除线”。
    QTextCharFormat spaceFormat;
    spaceFormat.setFontStrikeOut(false);

    QTextCharFormat format;
    format.setFontStrikeOut(checked);

    int runStart = startOffset;
    bool runIsSpace = text.at(runStart).isSpace();
    for (int i = startOffset + 1; i <= text.size(); ++i)
    {
        bool atEnd = (i == text.size());
        bool isSpace = !atEnd && text.at(i).isSpace();
        if (!atEnd && isSpace == runIsSpace)
        {
            continue;
        }

        QTextCursor cursor(document);
        cursor.setPosition(block.position() + runStart);
        cursor.setPosition(block.position() + i, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(runIsSpace ? spaceFormat : format);

        runStart = i;
        runIsSpace = isSpace;
    }
}

void RichTextEdit::applyChecklistMarkerFormat(QTextDocument *document, const QTextBlock &block)
{
    if (!blockHasChecklistPrefix(block))
    {
        return;
    }

    QTextCursor cursor(document);
    cursor.setPosition(block.position());
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    // checkbox 借用正文格式取得字号，但字体/粗体/删除线必须单独固定，避免被整篇格式污染。
    QTextCharFormat format;
    QString text = block.text();
    int startOffset = checklistBodyStartOffset(text);
    if (text.size() > startOffset)
    {
        QTextCursor bodyCursor(document);
        bodyCursor.setPosition(block.position() + startOffset);
        bodyCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        format = bodyCursor.charFormat();
    }
    else
    {
        format = block.charFormat();
        QFont font = document->defaultFont();
        if (format.fontPointSize() <= 0)
        {
            format.setFontPointSize(font.pointSizeF());
        }
        if (format.font().pixelSize() <= 0 && font.pixelSize() > 0)
        {
            QFont currentFont = format.font();
            currentFont.setPixelSize(font.pixelSize());
            format.setFont(currentFont);
        }
    }

    QFont markerFont = format.font();
    markerFont.setFamily(checklistMarkerFontFamily());
    markerFont.setBold(false);
    markerFont.setWeight(QFont::Normal);
    if (markerFont.pointSizeF() > 0)
    {
        markerFont.setPointSizeF(markerFont.pointSizeF() + 2.0);
    }
    else if (markerFont.pixelSize() > 0)
    {
        markerFont.setPixelSize(markerFont.pixelSize() + 2);
    }
    format.setFont(markerFont);
    format.setFontFamily(checklistMarkerFontFamily());
    format.setFontFamilies(QStringList() << checklistMarkerFontFamily());
    format.setFontWeight(QFont::Normal);
    format.setFontStrikeOut(false);
    cursor.mergeCharFormat(format);
}

void RichTextEdit::refreshChecklistBlockFormats(QTextDocument *document, const QTextBlock &block)
{
    if (!blockHasChecklistPrefix(block))
    {
        return;
    }

    applyChecklistTextState(document, block, blockChecklistChecked(block));
    applyChecklistMarkerFormat(document, block);
}

void RichTextEdit::applyChecklistInputFormat()
{
    QTextCursor cursor = textCursor();
    QTextBlock block = cursor.block();
    QString text = block.text();
    if (!textHasChecklistPrefix(text) || cursor.positionInBlock() != 1)
    {
        return;
    }

    // 光标位于 checkbox 后且无空格时，下一次输入要切回正文格式，否则会继续用 checkbox 字体。
    QTextCharFormat format;
    if (text.size() > 1)
    {
        QTextCursor bodyCursor(document());
        bodyCursor.setPosition(block.position() + 1);
        bodyCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        format = bodyCursor.charFormat();
    }
    else
    {
        format = checklistBodyDefaultFormat(document());
    }

    setCurrentCharFormat(format);
}

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

void RichTextEdit::refreshChecklistFormats()
{
    // 加载 HTML、整篇字体变化、保存还原后都需要重新校正待办的 marker 和正文状态。
    QTextDocument* doc = document();
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next())
    {
        refreshChecklistBlockFormats(doc, block);
    }
}

bool RichTextEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange)
    {
        flattenInactivePalette();
    }
    return QxTextEdit::eventFilter(watched, event);
}

void RichTextEdit::mousePressEvent(QMouseEvent *event)
{
    QTextCursor clickCursor = cursorForPosition(event->pos());
    QTextBlock block = clickCursor.block();
    // 只响应 checkbox 字符附近的点击，避免点到正文开头也误触发勾选。
    if (event->button() == Qt::LeftButton && block.isValid() && blockHasChecklistPrefix(block) &&
        checklistMarkerHitRect(block).contains(event->pos()))
    {
        QTextCursor cursor(document());
        cursor.beginEditBlock();
        cursor.setPosition(block.position());
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

        QString mark = cursor.selectedText();
        if (mark == QString(checklistUncheckedChar()))
        {
            cursor.insertText(QString(checklistCheckedChar()));
            refreshChecklistBlockFormats(document(), block);
            cursor.endEditBlock();
            return;
        }

        if (mark == QString(checklistCheckedChar()))
        {
            cursor.insertText(QString(checklistUncheckedChar()));
            refreshChecklistBlockFormats(document(), block);
            cursor.endEditBlock();
            return;
        }

        cursor.endEditBlock();
    }

    QxTextEdit::mousePressEvent(event);
}

void RichTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (!textCursor().hasSelection() && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter))
    {
        QTextCursor cursor = textCursor();
        QTextBlock block = cursor.block();
        if (blockHasChecklistPrefix(block))
        {
            if (blockIsEmptyChecklist(block))
            {
                // 空待办回车表示退出待办模式，删除 marker 和可选空格。
                int removeSize = checklistBodyStartOffset(block.text());
                cursor.beginEditBlock();
                cursor.setPosition(block.position());
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, removeSize);
                cursor.removeSelectedText();

                QTextCharFormat charFormat = currentCharFormat();
                charFormat.setFontStrikeOut(false);
                setCurrentCharFormat(charFormat);

                QTextCharFormat blockCharFormat = cursor.blockCharFormat();
                blockCharFormat.setFontStrikeOut(false);
                cursor.setBlockCharFormat(blockCharFormat);
                cursor.endEditBlock();
                setTextCursor(cursor);
                return;
            }

            QxTextEdit::keyPressEvent(event);

            cursor = textCursor();
            // 新建下一条待办前清掉删除线状态，避免勾选项回车后新行继承删除线。
            QTextCharFormat charFormat = currentCharFormat();
            charFormat.setFontStrikeOut(false);
            setCurrentCharFormat(charFormat);

            QTextCharFormat blockCharFormat = cursor.blockCharFormat();
            blockCharFormat.setFontStrikeOut(false);
            cursor.setBlockCharFormat(blockCharFormat);
            cursor.insertText(checklistPrefix(false));
            refreshChecklistBlockFormats(document(), cursor.block());
            setTextCursor(cursor);
            return;
        }
    }

    if (!textCursor().hasSelection())
    {
        // 普通键盘输入前先修正输入格式，处理“删掉 checkbox 后空格再打字”的情况。
        applyChecklistInputFormat();
    }

    int revision = document()->revision();
    QxTextEdit::keyPressEvent(event);
    if (document()->revision() != revision)
    {
        refreshChecklistBlockFormats(document(), textCursor().block());
    }
}

void RichTextEdit::inputMethodEvent(QInputMethodEvent *event)
{
    if (!textCursor().hasSelection())
    {
        // 中文输入法不一定走 keyPressEvent，也需要在 IME 提交前修正输入格式。
        applyChecklistInputFormat();
    }

    int revision = document()->revision();
    QxTextEdit::inputMethodEvent(event);
    if (document()->revision() != revision)
    {
        refreshChecklistBlockFormats(document(), textCursor().block());
    }
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
