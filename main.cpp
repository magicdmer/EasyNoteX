#include "mainwindow.h"
#include "helpfunc.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include "singleapplication.h"
#include <QTranslator>
#include <QSharedMemory>
#include <QStyleFactory>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

#define APP_VERSION "1.4.5"

int main(int argc, char *argv[])
{
#ifdef HAVE_WAYLAND
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION > QT_VERSION_CHECK(5,14,0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    SingleApplication a(argc, argv);
    if(a.isRunning())
    {
       return 0;
    }

    a.setApplicationVersion(APP_VERSION);
    ensureAppDirs();

#ifdef Q_OS_LINUX
    a.setStyle(QStyleFactory::create("Fusion"));
#endif


    QTranslator translator;
    translator.load(":/translations/widgets.qm");
    a.installTranslator(&translator);

    MainWindow w;
    a.w = &w;
    w.show();

    return a.exec();
}
