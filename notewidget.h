#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QWidget>
#include <QFont>
#include <QColor>
#include <QImage>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QUrl>
#include "richtextedit.h"
#include <QTimer>

class QFontComboBox;
class QSpinBox;
class QToolButton;

namespace Ui {
class NoteWidget;
}

class NoteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteWidget(QWidget *parent = nullptr);
    NoteWidget(QWidget *parent,QString noteName,QString fileName,QFont font,
               QColor penColor = QColor(),QColor paperColor = QColor());
    NoteWidget(QWidget *parent,QString noteName,QString groupName,QString fileName,QFont font,
               QColor penColor = QColor(),QColor paperColor = QColor());
    ~NoteWidget();

public:
    QString group() const { return m_group; }
    void setGroup(const QString& group) { m_group = group; }
    bool setFile(QString& fileName);
    bool rename(QString& newName);
    bool find(QString& text,QTextDocument::FindFlags flags);
    bool save(QString& filePath, int type = 0);
    void setTextFont(QFont& font);
    void setPenColor(const QColor& color);
    void setPaperColor(const QColor& color);
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
    void sltFontFamilyChanged(const QFont& font);
    void sltFontSizeChanged(int size);
    void sltBoldToggled(bool bold);
    void sltPickPenColor();
    void sltPickPaperColor();

private:
    void buildToolbar();
    void applyCharFormatToWholeNote(const QTextCharFormat& fmt);
    void writePaperToHtml(const QColor& color);
    void syncToolbar();
    void updateColorButton(QToolButton* button, const QColor& color, bool isBackground);
    QUrl originalImageResourceUrl(const QString& name) const;
    QImage imageResource(QTextDocument* document, const QString& name) const;

    Ui::NoteWidget *ui;
    QString m_filePath;
    RichTextEdit* m_textEdit;
    QString m_noteName;
    QString m_group;
    QTimer* m_typingTimer;
    bool m_textChanged;
    QString m_filterText;

    // 当前便签的整篇样式（贴身工具条）
    QFontComboBox* m_fontCombo;
    QSpinBox* m_sizeSpin;
    QToolButton* m_boldBtn;
    QToolButton* m_penBtn;
    QToolButton* m_paperBtn;
    QColor m_penColor;
    QColor m_paperColor;
};

#endif // NOTEWIDGET_H
