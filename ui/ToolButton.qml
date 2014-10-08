import QtQuick 2.3

Item {
    id: button
    signal clicked()
    property string releasedImage
    property string disabledImage
    property string hoveredImage
    property int counter: 0
    width: 40; height: 40

    Image {
        id: icon
        anchors.centerIn: parent
        source: {
            if(!button.enabled) return button.disabledImage
            return buttonArea.containsMouse ? button.hoveredImage : button.releasedImage
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 5
        width: 14; height: 14
        radius: 7
        color: "#D2130D"
        visible: button.counter > 0

        Text {
            anchors.centerIn: parent
            font.family: "Arial"
            font.pixelSize: 9
            renderType: Text.NativeRendering
            color: "#FFFFFF"
            text: button.counter
        }
    }

    MouseArea {
        id: buttonArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: button.clicked()
    }
}
