QT       += core gui network xml sql help

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = EasyNoteX

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(QxtGobalShortcut/qxtglobalshortcut.pri)

INCLUDEPATH += \
    cutex/include



win32:CONFIG(debug,debug|release): {
    LIBS += \
        cutex/lib/cutex_$${QT_ARCH}_d.lib
    QMAKE_CXXFLAGS += /MP
}

win32:CONFIG(release,debug|release): {
    LIBS += \
        cutex/lib/cutex_$${QT_ARCH}_r.lib
    QMAKE_CXXFLAGS += /MP
}

SOURCES += \
    aboutdialog.cpp \
    finddialog.cpp \
    helpdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    notewidget.cpp \
    renamedialog.cpp \
    richtextedit.cpp \
    setdialog.cpp \
    settabledialog.cpp \
    singleapplication.cpp

HEADERS += \
    aboutdialog.h \
    finddialog.h \
    helpdialog.h \
    mainwindow.h \
    notewidget.h \
    renamedialog.h \
    richtextedit.h \
    setdialog.h \
    settabledialog.h \
    singleapplication.h

FORMS += \
    aboutdialog.ui \
    finddialog.ui \
    helpdialog.ui \
    mainwindow.ui \
    notewidget.ui \
    renamedialog.ui \
    setdialog.ui \
    settabledialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_FILE = EasyNoteX.rc

RESOURCES += \
    resource.qrc
