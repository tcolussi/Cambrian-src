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
        anchors.topMargin: triangleImage.height - 10
        anchors.right: parent.right
        anchors.rightMargin: -5
        width: 288; height: 379
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
            anchors.rightMargin: 18
            source: "qrc:///images/ui/toolsTriangle.png"
            z: 10
        }

        Grid {
            id: applicationsGrid
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: moreButton.top
            anchors.leftMargin: 1
            anchors.rightMargin: 1
            anchors.topMargin: 1

            columns: 3
            property int itemWidth: width / columns
            property int itemHeight: height / columns

            ListModel {
                id: applicationsModel
                ListElement { name: "Ballot"; toolTip: ""; launchUrl: "http://qt-project.org"; iconUrl: "http://pml.nist.gov/Meetings/SRI2013/images/process6.png" }
                ListElement { name: "Photos"; toolTip: ""; launchUrl: "http://google.com"; iconUrl: "http://pml.nist.gov/Meetings/SRI2013/images/process6.png" }
                ListElement { name: "Contract"; toolTip: ""; launchUrl: "http://qt-project.org"; iconUrl: "http://pml.nist.gov/Meetings/SRI2013/images/process6.png" }
                ListElement { name: "Tools"; toolTip: ""; launchUrl: "http://qt-project.org"; iconUrl: "http://pml.nist.gov/Meetings/SRI2013/images/process6.png" }
                ListElement { name: "Groups"; toolTip: ""; launchUrl: "http://qt-project.org"; iconUrl: "http://pml.nist.gov/Meetings/SRI2013/images/process6.png" }
                ListElement { name: "Games"; toolTip: ""; launchUrl: "http://qt-project.org"; iconUrl: "http://pml.nist.gov/Meetings/SRI2013/images/process6.png" }
                ListElement { name: "Travelation"; toolTip: ""; launchUrl: "http://qt-project.org"; iconUrl: "http://pml.nist.gov/Meetings/SRI2013/images/process6.png" }
            }

            Repeater {
                id: applicationsRepeater
                model: applicationsModel
                delegate: Item {
                    width: applicationsGrid.itemWidth
                    height: applicationsGrid.itemHeight

                    Image {
                        id: appIcon
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.topMargin: 20
                        width: 45; height: 45
                        source: iconUrl
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: appIcon.bottom
                        anchors.topMargin: 15
                        horizontalAlignment: Text.AlignHCenter
                        font.family: "Arial"
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                        renderType: Text.NativeRendering
                        color: delegateArea.containsMouse ? "#6D6D6D" : "#404040"
                        text: name
                    }

                    MouseArea {
                        id: delegateArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            tabViewHeader.addTab(name, launchUrl)
                            //popup.close()
                        }
                    }
                }
            }
        }

        Rectangle {
            id: moreButton
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            border.width: 1
            border.color: "#CCCCCC"
            height: 34
            color: {
                if(moreArea.pressed) return "#FFFFFF"
                return moreArea.containsMouse ? "#F4F4F4" : "#EEEEEE"
            }

            Text {
                anchors.centerIn: parent
                font.family: "Arial"
                font.pixelSize: 12
                renderType: Text.NativeRendering
                text: qsTr("More")
                color: moreArea.pressed || !moreArea.containsMouse ? "#3F3F3F" : "#6D6D6D"
            }

            MouseArea {
                id: moreArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: rootItem.applicationsMoreButtonClicked()
            }
        }
    }
}
