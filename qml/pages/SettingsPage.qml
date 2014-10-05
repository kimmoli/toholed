import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.toholed.settings.ui 1.0


Page
{
    id: page

    property string daemonVersion : "---"

    SilicaFlickable
    {
        anchors.fill: parent

        PullDownMenu
        {
            MenuItem
            {
                text: "About..."
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"),
                                          { "version": tohosettings.version,
                                              "year": "2014",
                                              "name": "Toholed settings UI",
                                              "imagelocation": "/usr/share/icons/hicolor/86x86/apps/harbour-toholed-settings-ui.png"} )
            }
        }

        contentHeight: column.height

        Column
        {
            id: column

            width: page.width - Theme.paddingLarge
            spacing: Theme.paddingSmall
            anchors.horizontalCenter: parent.horizontalCenter

            PageHeader
            {
                title: "Toholed settings"
            }

            Label
            {
                text: "Daemon version: " + daemonVersion
            }

            SectionHeader
            {
                text: "General"
            }

            TextSwitch
            {
                text: "Blink"
                description: "Blink screen at new notification"
                checked: tohosettings.blink
                automaticCheck: false
                onClicked:
                {
                    tohosettings.blink = !tohosettings.blink
                    tohosettings.writeSettings()
                }
            }

            TextSwitch
            {
                text: "Ambient Light Sensor"
                description: "Enable automatic brightness"
                checked: tohosettings.als
                automaticCheck: false
                onClicked:
                {
                    tohosettings.als = !tohosettings.als
                    tohosettings.writeSettings()
                }
            }

            TextSwitch
            {
                text: "Proximity"
                description: "Shutdown display when proximity"
                checked: tohosettings.prox
                automaticCheck: false
                onClicked:
                {
                    tohosettings.prox = !tohosettings.prox
                    tohosettings.writeSettings()
                }
            }

            TextSwitch
            {
                text: "Display off when main active"
                description: "Toholed display off when main display active"
                checked: tohosettings.displayOffWhenMainActive
                automaticCheck: false
                onClicked:
                {
                    tohosettings.displayOffWhenMainActive = !tohosettings.displayOffWhenMainActive
                    tohosettings.writeSettings()
                }
            }

            SectionHeader
            {
                text: "Experimental"
            }

            TextSwitch
            {
                text: "Show analog clock"
                description: "Experimental analog clock face"
                checked: tohosettings.analogClockFace
                automaticCheck: false
                onClicked:
                {
                    tohosettings.analogClockFace = !tohosettings.analogClockFace
                    tohosettings.writeSettings()
                }
            }

            SectionHeader
            {
                text: "Other"
            }

            TextSwitch
            {
                text: "Screenshot mode"
                description: "Take screenshot from front proximity"
                checked: tohosettings.ssp
                automaticCheck: false
                onClicked:
                {
                    tohosettings.ssp = !tohosettings.ssp
                    tohosettings.writeSettings()
                }
            }
        }
    }

    TohoSettings
    {
        id: tohosettings
        Component.onCompleted:
            daemonVersion = readDaemonVersion()
    }
}


