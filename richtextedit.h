#ifndef RICHTEXTEDIT_H
#define RICHTEXTEDIT_H

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

private:
    QMenu *m_menu;
    QAction* m_action_copy;
    QAction* m_action_save;
};

#endif // RICHTEXTEDIT_H
