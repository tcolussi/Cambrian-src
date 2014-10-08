import QtQuick 2.3

Item {
    property alias name: roleNameText.text
    property alias image: thumbImage.source

    Rectangle {
        id: avatarImage
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        width: 26; height: 26
        border.width: 1
        border.color: "#AAAAAA"
        radius: 3

        Image {
            id: thumbImage
            anchors.fill: parent
            anchors.margins: 1
        }
    }

    Text {
        id: roleNameText
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: avatarImage.right
        anchors.right: parent.right
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        elide: Text.ElideRight
        renderType: Text.NativeRendering
        font.family: "Arial"
        font.pixelSize: 12
        color: "#414141"
    }
}
