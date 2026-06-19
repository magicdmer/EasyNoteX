#ifndef RICHTEXTEDIT_H
#define RICHTEXTEDIT_H

#include <QImage>
#include <QRect>
#include <QTextCharFormat>
#include <QUrl>
#include "qxtextedit.h"

using namespace cutex;

class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QTextBlock;
class QTextDocument;

class RichTextEdit : public QxTextEdit
{
    Q_OBJECT
public:
    RichTextEdit(QWidget* parent = nullptr);
    void dealBackTab();
    void refreshImageResources();
    void refreshChecklistFormats();

public slots:
    void sltImageRightClicked();
    void sltCopyImage();
    void sltSaveImage();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;

private:
    static bool blockHasChecklistPrefix(const QTextBlock &block);
    static int checklistBodyStartOffset(const QString &text);
    static bool blockIsEmptyChecklist(const QTextBlock &block);
    static bool blockChecklistChecked(const QTextBlock &block);
    static QTextCharFormat checklistBodyDefaultFormat(QTextDocument *document);
    QRect checklistMarkerHitRect(const QTextBlock &block) const;
    static void applyChecklistTextState(QTextDocument *document, const QTextBlock &block, bool checked);
    static void applyChecklistMarkerFormat(QTextDocument *document, const QTextBlock &block);
    static void refreshChecklistBlockFormats(QTextDocument *document, const QTextBlock &block);
    void applyChecklistInputFormat();
    void flattenInactivePalette();
    QUrl originalImageResourceUrl(const QString& name) const;
    QImage imageResource(QTextDocument* document, const QString& name) const;

    QMenu *m_menu;
    QAction* m_action_copy;
    QAction* m_action_save;
};

#endif // RICHTEXTEDIT_H
