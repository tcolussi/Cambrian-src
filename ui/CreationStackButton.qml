import QtQuick 2.3
import QtGraphicalEffects 1.0

ToolButton {
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

    Connections {
        target: dropdown
        onClicked: dropdown.expanded = !dropdown.expanded
    }

    ListModel {
        id: dropModel
        ListElement { name: "Identity and Roles" }
        ListElement { name: "Comunications" }
        ListElement { name: "Socjal Network" }
        ListElement { name: "Banking and Storage" }
        ListElement { name: "Rulescape" }
        ListElement { name: "Marketplace" }
        ListElement { name: "Registy" }
        ListElement { name: "Reputation" }
        ListElement { name: "Fundraising" }
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
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.bottom
        anchors.topMargin: triangleImage.height - 10
        width: dropColumn.width + 2
        height: dropColumn.y + dropColumn.height + dropColumn.heightDiff + 1
        border.width: 1
        border.color: "#CCCCCC"
        color: "#FFFFFF"
        visible: dropdown.expanded

        function containsPoint(px, py) {
            if(!visible) return false
            if(px < x) return false
            if(px > x + width) return false
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

        Text {
            id: headerText
            anchors.top: parent.top
            anchors.topMargin: 24
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 24
            font.family: "Arial"
            font.pixelSize: 14
            font.bold: true
            elide: Text.ElideRight
            renderType: Text.NativeRendering
            color: "#666666"
            text: qsTr("Creation stack")

            Component.onCompleted: dropColumn.maxWidth = Math.max(dropColumn.maxWidth, implicitWidth)
        }

        Column {
            id: dropColumn
            anchors.left: parent.left
            anchors.top: headerText.bottom
            anchors.leftMargin: 1
            anchors.topMargin: (24 - 11) / 2
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
                            rootItem.creationStackMenuItemClicked(index)
                            dropdown.expanded = false
                        }
                    }
                }
            }
        }
    }
}
