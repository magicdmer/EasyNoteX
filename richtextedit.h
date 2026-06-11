#ifndef RICHTEXTEDIT_H
#define RICHTEXTEDIT_H

#include <QImage>
#include <QUrl>
#include "qxtextedit.h"

using namespace cutex;

class RichTextEdit : public QxTextEdit
{
    Q_OBJECT
public:
    RichTextEdit(QWidget* parent = nullptr);
    void dealBackTab();

public slots:
    void sltImageRightClicked();
    void sltCopyImage();
    void sltSaveImage();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void flattenInactivePalette();
    QUrl originalImageResourceUrl(const QString& name) const;
    QImage imageResource(QTextDocument* document, const QString& name) const;

    QMenu *m_menu;
    QAction* m_action_copy;
    QAction* m_action_save;
};

#endif // RICHTEXTEDIT_H
