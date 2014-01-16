TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
#CONFIG -= qt
QT += dbus

INCLUDEPATH += ./inc

SOURCES += \
    src/toholed.cpp \
    src/charger.cpp \
    src/frontled.cpp \
    src/oled.cpp \
    src/tca8424.cpp \
    src/toh.cpp \
    src/toholed-dbus.cpp \
    src/jollafontti.cpp \
    src/worker.cpp

HEADERS += \
    inc/toholed.h \
    inc/toh.h \
    inc/tca8424.h \
    inc/oled.h \
    inc/frontled.h \
    inc/charger.h \
    inc/toholed-dbus.h \
    inc/jollafontti.h \
    inc/worker.h

