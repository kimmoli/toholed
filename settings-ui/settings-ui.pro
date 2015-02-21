#
# Project toholed-settings-ui, Toholed settings UI
#

TARGET = harbour-toholed-settings-ui

CONFIG += sailfishapp
QT += dbus

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

message($${DEFINES})

SOURCES += src/toholed-settings-ui.cpp \
	src/tohosettings.cpp
	
HEADERS += src/tohosettings.h \
    src/imageProvider.h

OTHER_FILES += qml/toholed-settings-ui.qml \
    qml/cover/CoverPage.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/AboutPage.qml \
    rpm/toholed-settings-ui.spec \
    harbour-toholed-settings-ui.png \
    harbour-toholed-settings-ui.desktop \
    qml/pages/Messagebox.qml

