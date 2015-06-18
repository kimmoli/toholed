TARGET = harbour-toholed

CONFIG += console link_pkgconfig
PKGCONFIG += contextkit-statefs

QT += dbus xml xmlpatterns

LIBS += -lrt

system(qdbusxml2cpp -p src/mceiface.h:src/mceiface.cpp 3rdparty/nemo-keepalive/lib/mceiface.xml)

target.path = /usr/bin/

systemd.path = /etc/systemd/system/
systemd.files = config/$${TARGET}.service

udevrule.path = /etc/udev/rules.d/
udevrule.files = config/95-$${TARGET}.rules

dbusconf.path = /etc/dbus-1/system.d/
dbusconf.files = config/$${TARGET}.conf

ambience.path = /usr/share/ambience/harbour-ambience-toholed
ambience.files = ambience/harbour-ambience-toholed.ambience

images.path = $${ambience.path}/images
images.files = ambience/images/*

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

# For testing purposes, uncomment following
#DEFINES += NOTIFICATIONDEBUG

message($${DEFINES})

INSTALLS += target systemd udevrule dbusconf ambience images

INCLUDEPATH += \
    ./src/ \
    ./3rdparty/libdsme/include/ \
    ./3rdparty/mce-dev/include/ \
    ./3rdparty/libiphb/src/ \
    ./3rdparty/nemo-keepalive/lib/


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
    src/derp.c \
    3rdparty/libiphb/src/libiphb.c \
    src/notificationmanager.cpp \
    src/updateTime.c \
    src/mceiface.cpp \
    3rdparty/nemo-keepalive/lib/backgroundactivity.cpp \
    3rdparty/nemo-keepalive/lib/backgroundactivity_p.cpp \
    3rdparty/nemo-keepalive/lib/heartbeat.cpp \
    src/weather.cpp

HEADERS += \
    3rdparty/libiphb/src/libiphb.h \
    3rdparty/libiphb/src/iphb_internal.h \
    3rdparty/libdsme/include/dsme/messages.h \
    3rdparty/mce-dev/include/mce/dbus-names.h \
    src/worker.h \
    src/tsl2772.h \
    src/toholed-dbus.h \
    src/toholed.h \
    src/toh.h \
    src/pienifontti.h \
    src/oled.h \
    src/jollafontti.h \
    src/icons.h \
    src/derp.h \
    src/charger.h \
    src/notificationmanager.h \
    src/updateTime.h \
    src/mceiface.h \
    3rdparty/nemo-keepalive/lib/backgroundactivity.h \
    3rdparty/nemo-keepalive/lib/backgroundactivity_p.h \
    3rdparty/nemo-keepalive/lib/heartbeat.h \
    src/weather.h

OTHER_FILES += \
    rpm/toholed.spec \
    config/$${TARGET}.conf \
    config/$${TARGET}.service \
    config/95-$${TARGET}.rules \
    ambience/harbour-ambience-toholed.ambience \
    ambience/images/harbour-ambience-toholed.jpg \
    3rdparty/nemo-keepalive/lib/mceiface.xml

