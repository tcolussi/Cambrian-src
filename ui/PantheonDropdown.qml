import QtQuick 2.3
import QtGraphicalEffects 1.0

Item {
    id: dropdown
    property bool expanded: false
    onExpandedChanged: if(expanded) appWindow.currentItem = dropdown
    function hide() {
        expanded = false;
    }

    function containsPoint(px, py) {
        if(popupRoot.containsPoint(px, py)) return true
        if(px < 0) return false
        if(px > width) return false
        if(py < 0) return false
        if(py > height) return false
        return true
    }

    Image {
        id: logoImage
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 5
        source: "qrc:///images/ui/pantheonLogo.png"
    }

    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: logoImage.right
        anchors.leftMargin: 5
        font.family: "Arial"
        font.pixelSize: 12
        renderType: Text.NativeRendering
        color: "#404040"
        text: qsTr("Pantheon")
    }

    Image {
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        source: "qrc:///images/ui/dropdownIndicatorIcon.png"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: dropdown.expanded = !dropdown.expanded
    }

    ListModel {
        id: dropModel
        ListElement { name: "Pantheon" }
        ListElement { name: "Terran Valley" }
        ListElement { name: "Republic of Doug" }
    }

    DropShadow {
        anchors.fill: popupRoot
        horizontalOffset: 2
        verticalOffset: 2
        radius: 4
        samples: 8
        color: "#80000000"
        source: popupRoot
        visible: dropdown.expanded
    }

    Rectangle {
        id: popupRoot
        anchors.top: dropdown.bottom
        anchors.topMargin: triangleImage.height - 3
        x: 5
        width: dropColumn.width + 2
        height: dropColumn.height + dropColumn.heightDiff * 2 + 2
        border.width: 1
        border.color: "#CCCCCC"
        color: "#FFFFFF"
        visible: dropdown.expanded

        function containsPoint(px, py) {
            if(!visible) return false
            if(px < 0) return false
            if(px > width) return false
            if(py < y) return false
            if(py > y + height) return false
            return true
        }

        Image {
            id: triangleImage
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.top
            anchors.bottomMargin: -1
            source: "qrc:///images/ui/toolsTriangle.png"
            z: 10
        }

        Column {
            id: dropColumn
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 1
            anchors.topMargin: heightDiff
            width: maxWidth + 24 * 2

            property int maxWidth: -1
            property int heightDiff: 24 - (24 - 11) / 2
            Repeater {
                id: dropRepeater
                model: dropModel
                delegate: Rectangle {
                    width: dropColumn.maxWidth + 24 * 2
                    height: 24
                    color: "#FFFFFF"

                    Component.onCompleted: {
                        dropColumn.maxWidth = Math.max(dropColumn.maxWidth, contentText.implicitWidth)
                    }

                    Text {
                        id: contentText
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 24
                        renderType: Text.NativeRendering
                        font.family: "Arial"
                        font.pixelSize: 11
                        color: delegateArea.containsMouse ? "#6D6D6D" : "#3F3F3F"
                        text: name
                    }

                    MouseArea {
                        id: delegateArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            rootItem.pantheonMenuItemClicked(index)
                            dropdown.expanded = false
                        }
                    }
                }
            }
        }
    }
}
