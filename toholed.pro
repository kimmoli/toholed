TARGET = toholed

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += dbus
QT -= gui

target.path = /usr/sbin/

systemd.path = /etc/systemd/system/
systemd.files = config/$${TARGET}.service

udevrule.path = /etc/udev/rules.d/
udevrule.files = config/95-$${TARGET}.rules

dbusconf.path = /etc/dbus-1/system.d/
dbusconf.files = config/$${TARGET}.conf

ambience.files = ambience/$${TARGET}.ambience
ambience.path = /usr/share/ambience/$${TARGET}

images.files = ambience/images/*
images.path = $${ambience.path}/images


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
    config/toholed.conf \
    config/toholed.service \
    config/95-toholed.rules \
    ambience/toholed.ambience \
    ambience/images/toholed.jpg

