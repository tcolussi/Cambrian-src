import QtQuick.Controls.Styles 1.2
import QtQuick.Controls 1.2
import QtQuick 2.3

BorderImage {
    property alias text: input.text
    signal accepted(string txt)
    border.left: 3
    border.right: 3
    border.top: 3
    border.bottom: 3
    source: "qrc:///images/ui/locationBackground.png"

    Image {
        id: searchIcon
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 10
        source: "qrc:///images/ui/searchIcon.png"
    }

    TextField {
        id: input
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: searchIcon.right
        placeholderText: qsTr("Search apps, websites, files and chats...")
        onAccepted: parent.accepted(text)

        style: TextFieldStyle {
            placeholderTextColor: "#AAAAAA"
            font.family: "Arial"
            font.pixelSize: 12
            renderType: Text.NativeRendering
            background: Rectangle { color: "transparent" }
        }
    }
}
