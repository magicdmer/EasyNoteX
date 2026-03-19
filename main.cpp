#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include "singleapplication.h"
#include <QTranslator>
#include <QSharedMemory>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

int main(int argc, char *argv[])
{
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

    //这是为了保险，因为有时候第一个SingleApplication卡主后，第二个还能运行
    QSharedMemory mem(QApplication::applicationFilePath());
    if(!mem.create(1))
    {
       return 0;
    }

    //a.setQuitOnLastWindowClosed(false);

    QDir::setCurrent(a.applicationDirPath());


    QTranslator translator;
    translator.load(":/translations/widgets.qm");
    a.installTranslator(&translator);

    MainWindow w;
    a.w = &w;
    if (argc != 2)
    {
        w.show();
    }

    return a.exec();
}
