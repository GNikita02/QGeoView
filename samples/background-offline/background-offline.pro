TARGET = qgeoview-samples-background-offline
TEMPLATE = app
CONFIG-= console

QT += gui widgets network sql

include(../lib.pri)
include(../shared.pri)

SOURCES += \
    fallbackTileBuilder.cpp \
    main.cpp \
    mainwindow.cpp \
    mytile.cpp

HEADERS += \
    fallbackTileBuilder.h \
    mainwindow.h \
    mytile.h

# app.depends = lib lib2
