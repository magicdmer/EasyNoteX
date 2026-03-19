#include "richtextedit.h"
#include <QClipboard>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

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

        QImage image = document()->resource(QTextDocument::ImageResource, fmt.name()).value<QImage>();

        QClipboard* clip = QApplication::clipboard();
        clip->setImage(image);
    }
}

void RichTextEdit::sltSaveImage()
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection() && cursor.charFormat().isImageFormat()) {
        QTextImageFormat fmt = cursor.charFormat().toImageFormat();

        QImage image = document()->resource(QTextDocument::ImageResource, fmt.name()).value<QImage>();

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
