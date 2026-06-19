#ifndef HELPFUNC_H
#define HELPFUNC_H

#include <QChar>
#include <QColor>
#include <QIcon>
#include <QString>
#include <QStyle>

QString notesRoot();
QString configRoot();
QString settingsFile();
QString notebookPath(const QString &notebookName);
QString groupPath(const QString &notebookName, const QString &groupName);
QString noteFilePath(const QString &notebookName, const QString &fileName);
QString noteFilePath(const QString &notebookName, const QString &groupName, const QString &fileName);
QString normalizedEntryName(const QString &name);
bool isValidEntryName(const QString &name);
QIcon stableStandardIcon(QStyle::StandardPixmap sp);
QIcon fileIcon(const QString &path);
void ensureAppDirs();
bool isUos();
bool moveToTrash(const QString &path, QString *errorMessage = nullptr);
QChar checklistUncheckedChar();
QChar checklistCheckedChar();
QString checklistPrefix(bool checked);
QString checklistMarkerFontFamily();
bool textHasChecklistPrefix(const QString &text);
QString defaultGlobalHotkey();
QString defaultTableShortcut();
QString defaultChecklistShortcut();
QString defaultCodeBlockShortcut();
QString codeBlockFontFamily();
QColor codeBlockBackgroundColor();
QColor codeBlockBorderColor();

#endif // HELPFUNC_H
