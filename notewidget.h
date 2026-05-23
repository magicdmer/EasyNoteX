#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QWidget>
#include <QFont>
#include <QTextDocument>
#include "richtextedit.h"
#include <QTimer>

namespace Ui {
class NoteWidget;
}

class NoteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteWidget(QWidget *parent = nullptr);
    NoteWidget(QWidget *parent,QString noteName,QString fileName,QFont font);
    NoteWidget(QWidget *parent,QString noteName,QString groupName,QString fileName,QFont font);
    ~NoteWidget();

public:
    QString group() const { return m_group; }
    void setGroup(const QString& group) { m_group = group; }
    bool setFile(QString& fileName);
    bool rename(QString& newName);
    bool find(QString& text,QTextDocument::FindFlags flags);
    bool save(QString& filePath, int type = 0);
    void setTextFont(QFont& font);
    void setCurrentFont(QFont& font);
    void setFontColor(QColor& color);
    void setBgColor(QPalette& palette);
    void setTabWidth(int width);
    bool isEmpty();
    void deletefile();
    bool load();
    bool save();
    void dealBackTab();
    void insertTable(int row, int col, int percent);

public slots:
    void sltTextChanged();
    void sltFilterEntries();

private:
    Ui::NoteWidget *ui;
    QString m_filePath;
    RichTextEdit* m_textEdit;
    QString m_noteName;
    QString m_group;
    QTimer* m_typingTimer;
    bool m_textChanged;
    QString m_filterText;
};

#endif // NOTEWIDGET_H
