import QtQuick 2.3

Item {
    id: scrollItem
    property var flickable
    property bool alwaysVisible: false
    property string color: "#333333"
    width: 12
    z: 1

    function flickableContentYChanged() {
        if(flickable === undefined)
            return

        var t = flickable.height - scroll.height
        scroll.y = (flickable.contentY / (flickable.contentHeight - flickable.height)) * t
    }

    MouseArea {
        id: scrollArea
        anchors.fill: parent
        hoverEnabled: true
    }

    Rectangle {
        id: scroll

        width: scrollItem.width
        height: {
            var t = (flickable.height * flickable.height) / flickable.contentHeight
            return t < 20 ? 20 : t
        }
        y: 0; x: 0
        color: scrollItem.color
        opacity: {
            if(scrollItem.alwaysVisible)
                return 1
            return flickable.moving || handleArea.pressed || scrollArea.containsMouse ? 0.5 : 0
        }
        visible: flickable.contentHeight > flickable.height

        Behavior on opacity {
            NumberAnimation { duration: 100; easing.type: Easing.InQuad }
        }

        MouseArea {
            id: handleArea
            anchors.fill: parent
            drag.target: scroll
            drag.axis: Drag.YAxis
            drag.minimumY: 0
            drag.maximumY: flickable.height - height
            propagateComposedEvents: true

            onPositionChanged: {
                if(!pressed) return
                var dy = scroll.y / (flickable.height - scroll.height)
                flickable.contentY = (flickable.contentHeight - flickable.height) * dy
            }
        }
    }
}
