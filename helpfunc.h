#ifndef HELPFUNC_H
#define HELPFUNC_H

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

#endif // HELPFUNC_H
