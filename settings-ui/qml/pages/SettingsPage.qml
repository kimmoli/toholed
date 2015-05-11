import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.toholed.settings.ui 1.0


Page
{
    id: page

    property string daemonVersion : "---"
    property int cnt : 0

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
                                              "year": "2014-2015",
                                              "name": "Toholed settings UI",
                                              "imagelocation": "/usr/share/icons/hicolor/86x86/apps/harbour-toholed-settings-ui.png"} )
            }
        }

        contentHeight: column.height

        Messagebox
        {
            id: messagebox
        }

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
                width: parent.width
                text: "Daemon version: " + daemonVersion
                truncationMode: TruncationMode.Fade
            }

            SectionHeader
            {
                text: "Current display"
            }

            Image
            {
                id: screenshot
                width: 256
                height: 128
                anchors.horizontalCenter: parent.horizontalCenter
                source: "image://screenshot/screenshot"
                BackgroundItem
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        cnt++
                        screenshot.source = "image://screenshot/screenshot" + cnt
                    }
                    onPressAndHold:
                    {
                        tohosettings.saveOledScreen()
                        messagebox.showMessage("OLED Screenshot saved", 2500)
                    }
                }
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
                    tohosettings.writeSettings("blink", tohosettings.blink)
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
                    tohosettings.writeSettings("als", tohosettings.als)
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
                    tohosettings.writeSettings("proximity", tohosettings.prox)
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
                    tohosettings.writeSettings("displayOffWhenMainActive", tohosettings.displayOffWhenMainActive)
                }
            }

            TextSwitch
            {
                text: "Show analog clock"
                description: "Analog clock face"
                checked: tohosettings.analogClockFace
                automaticCheck: false
                onClicked:
                {
                    tohosettings.analogClockFace = !tohosettings.analogClockFace
                    tohosettings.writeSettings("analogClockFace", tohosettings.analogClockFace)
                }
            }

            TextSwitch
            {
                text: "Show alarms present"
                description: "Bell icon shown if any alarm is active"
                checked: tohosettings.showAlarmsPresent
                automaticCheck: false
                onClicked:
                {
                    tohosettings.showAlarmsPresent = !tohosettings.showAlarmsPresent
                    tohosettings.writeSettings("showAlarmsPresent", tohosettings.showAlarmsPresent)
                }
            }

            TextSwitch
            {
                text: "Show current temperature"
                description: "Show temperature of current location"
                checked: tohosettings.showCurrentTemperature
                automaticCheck: false
                onClicked:
                {
                    tohosettings.showCurrentTemperature = !tohosettings.showCurrentTemperature
                    tohosettings.writeSettings("showCurrentTemperature", tohosettings.showCurrentTemperature)
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
                    tohosettings.writeScreenCapture()
                }
            }
        }
    }

    TohoSettings
    {
        id: tohosettings
        Component.onCompleted:
            daemonVersion = readDaemonVersion()

        onScreenShotChanged:
        {
            cnt++
            screenshot.source = "image://screenshot/screenshot" + cnt
        }
    }
}


