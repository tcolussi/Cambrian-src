import QtQuick 2.3
import QtGraphicalEffects 1.0

ToolButton {
    id: dropdown

    property bool expanded: false
    onExpandedChanged: if(expanded) appWindow.currentItem = dropdown
    function hide() {
        expanded = false;
        subPopupRoot.visible = false
    }

    function containsPoint(px, py) {
        if(popupRoot.containsPoint(px, py)) return true
        if(subPopupRoot.containsPoint(px, py)) return true
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
        id: dropModel1
        ListElement { name: "New Tab"; hasChild: false }
        ListElement { name: "Advanced"; hasChild: true }
        ListElement { name: "Exit"; hasChild: false }
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
        anchors.right: parent.right
        anchors.rightMargin: 3
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
            anchors.right: parent.right
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
                model: dropModel1
                delegate: Rectangle {
                    id: popupDelegate
                    width: dropColumn.width
                    height: 24
                    color: "#FFFFFF"

                    Component.onCompleted: {
                        dropColumn.maxWidth = Math.max(dropColumn.maxWidth, contentText.implicitWidth + (hasChild ? 24 : 0))
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

                    Image {
                        id: subMenuIcon
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 14
                        visible: hasChild
                        source: "qrc:///images/ui/subMenuIndicator.png"
                    }

                    MouseArea {
                        id: delegateArea
                        anchors.fill: parent
                        hoverEnabled: true

                        Timer {
                            id: timer
                            interval: 500
                            running: false
                            repeat: false
                            onTriggered: subPopupRoot.visible = true
                        }

                        onContainsMouseChanged: {
                            if(containsMouse && hasChild) timer.start()
                            else timer.stop()
                        }

                        onClicked: {
                            if(hasChild) {
                                timer.stop()
                                subPopupRoot.visible = true
                            } else {
                                rootItem.moreMenuItemClicked(index)
                                dropdown.hide()
                            }
                        }
                    }
                }
            }
        }

        ListModel {
            id: dropModel2
            ListElement { name: "Play Sounds"; hasCheckBox: false }
            ListElement { name: "Show"; hasCheckBox: false }
            ListElement { name: "Message Log"; hasCheckBox: true }
            ListElement { name: "Recent Errors"; hasCheckBox: true }
            ListElement { name: "Developer Apps"; hasCheckBox: true }
            ListElement { name: "Backup Configuration"; hasCheckBox: false }
            ListElement { name: "Restore Configuration"; hasCheckBox: false }
            ListElement { name: "Display Certicicates"; hasCheckBox: false }
        }

        DropShadow {
            anchors.fill: subPopupRoot
            horizontalOffset: 2
            verticalOffset: 2
            radius: 4
            samples: 8
            color: "#80000000"
            source: subPopupRoot
            visible: subPopupRoot.visible
        }

        Rectangle {
            id: subPopupRoot
            width: subDropColumn.width + 2
            height: subDropColumn.y + subDropColumn.height + subDropColumn.heightDiff + 1

            anchors.right: popupRoot.left
            anchors.rightMargin: -24
            y: popupRoot.y
            border.width: 1
            border.color: "#CCCCCC"
            color: "#FFFFFF"
            visible: false

            function containsPoint(px, py) {
                if(!visible) return false
                if(px < popupRoot.x + x) return false
                if(px > popupRoot.x + x + width) return false
                if(py < popupRoot.y + y) return false
                if(py > popupRoot.y + y + height) return false
                return true
            }

            Column {
                id: subDropColumn
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 1
                anchors.topMargin: heightDiff
                width: maxWidth + 24 * 2

                property int maxWidth: -1
                property int heightDiff: 24 - (24 - 11) / 2
                Repeater {
                    id: subDropRepeater
                    model: dropModel2
                    delegate: Rectangle {
                        width: subDropColumn.maxWidth + 24 * 2
                        height: 24
                        color: "#FFFFFF"

                        Component.onCompleted: {
                            subDropColumn.maxWidth = Math.max(subDropColumn.maxWidth, subContentText.implicitWidth + (hasCheckBox ? checkItem.width + 10 : 0))
                        }

                        Text {
                            id: subContentText
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: hasCheckBox ? checkItem.right : parent.left
                            anchors.leftMargin: hasCheckBox ? 10 : 24
                            renderType: Text.NativeRendering
                            font.family: "Arial"
                            font.pixelSize: 11
                            color: subDelegateArea.containsMouse ? "#6D6D6D" : "#3F3F3F"
                            text: name
                        }

                        Item {
                            id: checkItem
                            property bool checked: true

                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.leftMargin: 24
                            width: height
                            visible: hasCheckBox

                            Image {
                                anchors.centerIn: parent
                                source: parent.checked ? "qrc:///images/ui/checkIcon.png" : ""
                            }
                        }

                        MouseArea {
                            id: subDelegateArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                checkItem.checked = !checkItem.checked
                                rootItem.moreSubMenuItemClicked(index, checkItem.checked)
                                if(!hasCheckBox)
                                    dropdown.hide()
                            }
                        }
                    }
                }
            }
        }
    }
}
