# -------------------------------------------------
# Project created by QtCreator 2010-06-28T08:40:39
# -------------------------------------------------
QT += network
TARGET = equitwebserver

macx {
    TARGET = EquitWebServer
    CONFIG += x86
}

win32 {
}

TEMPLATE = app
SOURCES += \
    bpEditableTreeWidget.cpp \
    Server.cpp \
    Configuration.cpp \
    MainWindow.cpp \
    RequestHandler.cpp \
    main.cpp \
    bpIpListWidget.cpp \
    ConnectionCountLabel.cpp \
    ConfigurationWidget.cpp \
    HostNetworkInfo.cpp
HEADERS += \
    bpEditableTreeWidget.h \
    Server.h \
    Configuration.h \
    RequestHandler.h \
    MainWindow.h \
    RequestHandlerResponseCodes.h \
    bpIpListWidget.h \
    ConnectionCountLabel.h \
    ConfigurationWidget.h \
    HostNetworkInfo.h

RESOURCES += \
    resources.qrc \
    mimeicons.qrc

OTHER_FILES += \
	equitwebserver.desktop \
    lgpl-3.0.txt \
    readme.txt \
    notes.txt \
    CMakeLists.txt \
    linux_build.sh
