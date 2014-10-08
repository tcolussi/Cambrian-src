import QtQuick 2.3
import QtGraphicalEffects 1.0

ToolButton {
    id: dropdown
    counter: listView.count <= 99 ? listView.count : 99

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
        anchors.top: parent.bottom
        anchors.topMargin: triangleImage.height - 7
        anchors.right: parent.right
        anchors.rightMargin: -5
        width: 330; height: 376
        color: "#333333"
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
            anchors.rightMargin: 18
            source: "qrc:///images/ui/toolsDarkTriangle.png"
            z: 10
        }

        Text {
            id: headText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            height: 33
            verticalAlignment: Text.AlignVCenter
            font.family: "Arial"
            font.pixelSize: 12
            font.bold: true
            elide: Text.ElideRight
            renderType: Text.NativeRendering
            color: "#FFFFFF"
            text: qsTr("Chats")
        }

        ListModel {
            id: testModel
            ListElement { name: "Incoming File Transfer"; content: "corp2014.pdf 4.3Mb"; time: "10:06 am" }
            ListElement { name: "Incoming File Transfer"; content: "corp2014.pdf 4.3Mb"; time: "10:06 am" }
            ListElement { name: "Incoming File Transfer"; content: "corp2014.pdf 4.3Mb"; time: "10:06 am" }
            ListElement { name: "Incoming File Transfer"; content: "corp2014.pdf 4.3Mb"; time: "10:06 am" }
            ListElement { name: "Incoming File Transfer"; content: "corp2014.pdf 4.3Mb"; time: "10:06 am" }
        }

        Scroll {
            id: flickableScroll
            anchors.right: listView.right
            anchors.top: listView.top
            anchors.bottom: listView.bottom
            flickable: listView
            z: 1
        }

        ListView {
            id: listView
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: headText.bottom
            anchors.bottom: footBar.top
            anchors.leftMargin: 3
            anchors.rightMargin: 3
            spacing: 3
            boundsBehavior: ListView.StopAtBounds
            onContentYChanged: flickableScroll.flickableContentYChanged()
            clip: true
            model: testModel

            delegate: Rectangle {
                width: listView.width
                height: 74
                color: "#FFFFFF"

                Rectangle {
                    id: iconRect
                    anchors.top: parent.top
                    anchors.left: parent.left
                    width: 50; height: 50
                    color: iconArea.containsMouse ? "#000000" : "#FF9900"

                    Image {
                        id: icon
                        anchors.centerIn: parent
                        source: iconArea.containsMouse ? "qrc:///images/ui/noImage.png" :
                                                         "qrc:///images/ui/downloadIcon.png"
                    }

                    MouseArea {
                        id: iconArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: rootItem.notificationItemRemoveClicked(index)
                    }
                }

                Text {
                    id: nameText
                    anchors.top: parent.top
                    anchors.left: iconRect.right
                    anchors.right: timeText.left
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    anchors.topMargin: 8
                    elide: Text.ElideRight
                    renderType: Text.NativeRendering
                    font.family: "Arial"
                    font.pixelSize: 12
                    color: "#000000"
                    text: name
                }

                Text {
                    anchors.left: iconRect.right
                    anchors.right: parent.right
                    anchors.bottom: iconRect.bottom
                    anchors.margins: 8
                    font.family: "Arial"
                    font.pixelSize: 9
                    renderType: Text.NativeRendering
                    elide: Text.ElideRight
                    color: "#3F3F3F"
                    text: content
                }

                Text {
                    id: timeText
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.topMargin: 8
                    font.family: "Arial"
                    font.pixelSize: 9
                    renderType: Text.NativeRendering
                    color: "#3F3F3F"
                    text: time
                }

                Row {
                    id: buttonsRow
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    height: 24

                    Text {
                        width: 60; height: parent.height
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.family: "Arial"
                        font.bold: true
                        font.pixelSize: 10
                        renderType: Text.NativeRendering
                        color: "#9E9E9E"
                        text: qsTr("DECLINE")

                        MouseArea {
                            anchors.fill: parent
                            onClicked: rootItem.notificationItemAcceptClicked(index, false)
                        }
                    }

                    Text {
                        width: 60; height: parent.height
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.family: "Arial"
                        font.bold: true
                        font.pixelSize: 10
                        renderType: Text.NativeRendering
                        color: "#0066FF"
                        text: qsTr("ACCEPT")

                        MouseArea {
                            anchors.fill: parent
                            onClicked: rootItem.notificationItemAcceptClicked(index, true)
                        }
                    }
                }
            }
        }

        Rectangle {
            id: footBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 40
            color: "#666666"

            Row {
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                Item {
                    height: parent.height; width: height
                    Image {
                        anchors.centerIn: parent
                        source: magArea.containsMouse ? "qrc:///images/ui/magHoveredIcon.png" :
                                                        "qrc:///images/ui/magIcon.png"
                    }

                    MouseArea {
                        id: magArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: rootItem.viewAllNotificationsClicked()
                    }
                }

                Item {
                    height: parent.height; width: height
                    Image {
                        anchors.centerIn: parent
                        source: clearArea.containsMouse ? "qrc:///images/ui/clearAllHoveredIcon.png" :
                                                          "qrc:///images/ui/clearAllIcon.png"
                    }

                    MouseArea {
                        id: clearArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: rootItem.clearAllNotificationsClicked()
                    }
                }
            }
        }
    }
}
