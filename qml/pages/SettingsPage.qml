import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.toholed.settings.ui 1.0


Page
{
    id: page

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

            width: page.width
            spacing: Theme.paddingSmall
            PageHeader
            {
                title: "Toholed settings"
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
                onClicked: tohosettings.blink = !tohosettings.blink
            }

            TextSwitch
            {
                text: "ALS"
                description: "Enable automatic brightness"
                checked: tohosettings.als
                automaticCheck: false
                onClicked: tohosettings.als = !tohosettings.als
            }

            TextSwitch
            {
                text: "Proximity"
                description: "Shutdown display when proximity"
                checked: tohosettings.prox
                automaticCheck: false
                onClicked: tohosettings.prox = !tohosettings.prox
            }

            TextSwitch
            {
                text: "Display off when main active"
                description: "Toholed display off when main display active"
                checked: tohosettings.displayOffWhenMainActive
                automaticCheck: false
                onClicked: tohosettings.displayOffWhenMainActive = !tohosettings.displayOffWhenMainActive
            }

            TextSwitch
            {
                text: "Show analog clock"
                description: "Experimental analog clock face"
                checked: tohosettings.analogClockFace
                automaticCheck: false
                onClicked: tohosettings.analogClockFace = !tohosettings.analogClockFace
            }

            TextSwitch
            {
                text: "Screenshot mode"
                description: "Take screenshot from front proximity"
                checked: tohosettings.ssp
                automaticCheck: false
                onClicked: tohosettings.ssp = !tohosettings.ssp
            }

            Button
            {
                text: "Apply"
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: tohosettings.writeSettings()
            }


        }
    }

    TohoSettings
    {
        id: tohosettings
    }
}


