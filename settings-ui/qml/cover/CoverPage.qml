import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground
{
    Image
    {
        id: image
        source: "/usr/share/icons/hicolor/86x86/apps/harbour-toholed-settings-ui.png"
        anchors.centerIn: parent
    }

    Label
    {
        id: label
        anchors.top: image.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Toholed"
    }
}


