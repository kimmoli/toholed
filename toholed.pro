TARGET = harbour-toholed

TEMPLATE = app
CONFIG += console link_pkgconfig
CONFIG -= app_bundle
QT += dbus
QT -= gui

PKGCONFIG += libiphb

target.path = /usr/bin/

systemd.path = /etc/systemd/system/
systemd.files = config/$${TARGET}.service

udevrule.path = /etc/udev/rules.d/
udevrule.files = config/95-$${TARGET}.rules

dbusconf.path = /etc/dbus-1/system.d/
dbusconf.files = config/$${TARGET}.conf

ambience.path = /usr/share/ambience/$${TARGET}
ambience.files = ambience/$${TARGET}.ambience

images.path = $${ambience.path}/images
images.files = ambience/images/*

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

message($${DEFINES})

INSTALLS += target systemd udevrule dbusconf ambience images

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

OTHER_FILES += \
    rpm/toholed.spec \
    config/$${TARGET}.conf \
    config/$${TARGET}.service \
    config/95-$${TARGET}.rules \
    ambience/$${TARGET}.ambience \
    ambience/images/toholed.jpg

