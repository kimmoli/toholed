#
# Project toholed-settings-ui, Toholed settings UI
#

TARGET = toholed-settings-ui

CONFIG += sailfishapp

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

message($${DEFINES})

SOURCES += src/toholed-settings-ui.cpp \
	src/tohosettings.cpp
	
HEADERS += src/tohosettings.h

OTHER_FILES += qml/toholed-settings-ui.qml \
    qml/cover/CoverPage.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/AboutPage.qml \
    rpm/toholed-settings-ui.spec \
	toholed-settings-ui.png \
    toholed-settings-ui.desktop

