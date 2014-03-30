TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += dbus
QT -= gui

REVISION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --dirty)
DEFINES += "GITHASH=\\\"$${REVISION}\\\""

INCLUDEPATH += ./inc

SOURCES += \
    src/toholed.cpp \
    src/charger.cpp \
    src/oled.cpp \
    src/toh.cpp \
    src/toholed-dbus.cpp \
    src/jollafontti.cpp \
    src/worker.cpp \
    src/pienifontti.cpp \
    src/icons.cpp \
    src/tsl2772.cpp \
    src/derp.c

HEADERS += \
    inc/toholed.h \
    inc/toh.h \
    inc/oled.h \
    inc/charger.h \
    inc/toholed-dbus.h \
    inc/jollafontti.h \
    inc/worker.h \
    inc/pienifontti.h \
    inc/icons.h \
    inc/tsl2772.h \
    inc/derp.h

