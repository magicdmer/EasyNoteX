#include "helpfunc.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QApplication>
#include <QStandardPaths>
#include <QStringList>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#  ifdef Q_OS_WIN32
#    include <windows.h>
#    include <shellapi.h>
#    include <vector>
#  else
#    include <QProcess>
#  endif
#endif

static QString normalizedPath(const QString &path)
{
    return QDir::toNativeSeparators(QDir::cleanPath(path));
}

static QString readOsReleaseValue(const QString& key)
{
    QFile file(QStringLiteral("/etc/os-release"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return QString();
    }

    while (!file.atEnd())
    {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (!line.startsWith(key + QLatin1Char('=')))
        {
            continue;
        }

        QString value = line.mid(key.size() + 1).trimmed();
        if (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')) && value.size() >= 2)
        {
            value = value.mid(1, value.size() - 2);
        }
        return value;
    }

    return QString();
}

QString notesRoot()
{
#ifdef Q_OS_WIN32
    return normalizedPath(QDir(QCoreApplication::applicationDirPath()).filePath("data"));
#else
    return normalizedPath(QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath("EasyNoteX"));
#endif
}

QString configRoot()
{
#ifdef Q_OS_WIN32
    return normalizedPath(QCoreApplication::applicationDirPath());
#else
    return normalizedPath(QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).filePath("EasyNoteX"));
#endif
}

QString settingsFile()
{
    return normalizedPath(QDir(configRoot()).filePath("EasyNote.ini"));
}

QString notebookPath(const QString &notebookName)
{
    return normalizedPath(QDir(notesRoot()).filePath(notebookName));
}

QString groupPath(const QString &notebookName, const QString &groupName)
{
    if (groupName.isEmpty())
    {
        return notebookPath(notebookName);
    }
    return normalizedPath(QDir(notebookPath(notebookName)).filePath(groupName));
}

QString noteFilePath(const QString &notebookName, const QString &fileName)
{
    return normalizedPath(QDir(notebookPath(notebookName)).filePath(fileName + ".enote"));
}

QString noteFilePath(const QString &notebookName, const QString &groupName, const QString &fileName)
{
    return normalizedPath(QDir(groupPath(notebookName, groupName)).filePath(fileName + ".enote"));
}

QString normalizedEntryName(const QString &name)
{
    return name.trimmed();
}

bool isValidEntryName(const QString &name)
{
    return !name.isEmpty()
        && name != QLatin1String(".")
        && name != QLatin1String("..")
        && !name.contains(QLatin1Char('/'))
        && !name.contains(QLatin1Char('\\'))
        && !name.contains(QLatin1Char('|'));
}

QIcon stableStandardIcon(QStyle::StandardPixmap sp)
{
    QPixmap pm = QApplication::style()->standardIcon(sp).pixmap(16, 16);
    QIcon icon(pm);
    icon.addPixmap(pm, QIcon::Selected);
    return icon;
}

QIcon fileIcon(const QString &path)
{
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo(path));
    QPixmap pm = icon.pixmap(16, 16);
    icon.addPixmap(pm, QIcon::Selected);
    return icon;
}

void ensureAppDirs()
{
    QDir dir;
    dir.mkpath(notesRoot());
    dir.mkpath(configRoot());
}

bool moveToTrash(const QString &path, QString *errorMessage)
{
    const QString nativePath = QDir::toNativeSeparators(path);

    if (!QFileInfo::exists(nativePath))
    {
        if (errorMessage)
        {
            *errorMessage = QStringLiteral("path does not exist: %1").arg(nativePath);
        }
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QFile file(nativePath);
    if (file.moveToTrash())
    {
        return true;
    }
    if (errorMessage)
    {
        *errorMessage = file.errorString();
    }
    return false;
#else
#ifdef Q_OS_WIN32
    // SHFileOperationW requires pFrom to end with a double null.
    QString winPath = QDir::toNativeSeparators(nativePath);
    std::vector<wchar_t> buf(static_cast<size_t>(winPath.size()) + 2, L'\0');
    winPath.toWCharArray(buf.data());

    SHFILEOPSTRUCTW op;
    ZeroMemory(&op, sizeof(op));
    op.wFunc = FO_DELETE;
    op.pFrom = buf.data();
    op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

    const int ret = SHFileOperationW(&op);
    if (ret == 0 && !op.fAnyOperationsAborted)
    {
        return true;
    }
    if (errorMessage)
    {
        *errorMessage = QStringLiteral("SHFileOperation failed, code=%1").arg(ret);
    }
    return false;
#else
    const QStringList candidates = {
        QStringLiteral("gio"),
        QStringLiteral("kioclient5"),
        QStringLiteral("gvfs-trash"),
    };

    for (const QString &tool : candidates)
    {
        QStringList args;
        if (tool == QStringLiteral("gio"))
        {
            args << QStringLiteral("trash") << nativePath;
        }
        else if (tool == QStringLiteral("kioclient5"))
        {
            args << QStringLiteral("move") << nativePath << QStringLiteral("trash:/");
        }
        else
        {
            args << nativePath;
        }

        QProcess proc;
        proc.start(tool, args);
        if (!proc.waitForStarted(2000))
        {
            continue;
        }
        proc.waitForFinished(10000);
        if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)
        {
            return true;
        }
    }

    if (errorMessage)
    {
        *errorMessage = QStringLiteral("no trash backend available (gio/kioclient5/gvfs-trash)");
    }
    return false;
#endif
#endif
}

QChar checklistUncheckedChar()
{
    return QChar(0x2610);
}

QChar checklistCheckedChar()
{
    return QChar(0x2611);
}

QString checklistPrefix(bool checked)
{
    return QString(checked ? checklistCheckedChar() : checklistUncheckedChar()) + QLatin1Char(' ');
}

QString checklistMarkerFontFamily()
{
    return QStringLiteral("Segoe UI Symbol");
}

bool textHasChecklistPrefix(const QString &text)
{
    if (text.isEmpty())
    {
        return false;
    }

    QChar ch = text.at(0);
    return ch == checklistUncheckedChar() || ch == checklistCheckedChar();
}

QString codeBlockFontFamily()
{
    return QStringLiteral("Consolas");
}

QColor codeBlockBackgroundColor()
{
    return QColor(QLatin1String("#f6f8fa"));
}

QColor codeBlockBorderColor()
{
    return QColor(QLatin1String("#d0d7de"));
}

bool isUos()
{
#ifdef Q_OS_LINUX
    static const bool isUosSystem = []() {
        const QString id = readOsReleaseValue(QStringLiteral("ID")).trimmed().toLower();
        if (id == QStringLiteral("uos"))
        {
            return true;
        }

        const QString name = readOsReleaseValue(QStringLiteral("NAME")).trimmed().toLower();
        return name == QStringLiteral("uos");
    }();

    return isUosSystem;
#else
    return false;
#endif
}
