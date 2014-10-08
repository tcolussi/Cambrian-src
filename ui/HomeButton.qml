import QtQuick 2.3

Item {
    signal clicked()

    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        elide: Text.ElideRight
        renderType: Text.NativeRendering
        font.family: "Arial"
        font.pixelSize: 12
        color: buttonArea.containsMouse ? "#6D6D6D" : "#404040"
        text: qsTr("Home")
    }

    MouseArea {
        id: buttonArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
    }
}
