import QtQuick 2.3

Item {
    id: headerView
    height: 25

    signal tabAdded(int identifier, string url)
    signal tabRemoved(int identifier, int prevIdentifier)
    signal currentTabChanged(int identifier)

    property int maxTabsCount: 20
    property int maxIndex: 0
    property int tabIdentifier: 0
    property int currentIdentifier: 0
    property var tabObjects: new Array()
    property bool tabMoving: false
    property int itemWidth: {
        var w = (headerView.width + repeater.count * 16 - 40) / repeater.count
        return w > 164 ? 164 : w
    }

    function modifyTab(identifier, url, name) {
        for(var i = 0; i < tabsModel.count; ++i)
            if(tabsModel.get(i).identifier === identifier) {
                tabsModel.get(i).name = name
                tabsModel.get(i).url = url
                return
            }
    }

    function addTab(name, url) {
        var nextIndex = tabsModel.count
        tabsModel.append({"name":name, "url":url, "identifier":headerView.tabIdentifier})
        currentIdentifier = headerView.tabIdentifier
        ++tabIdentifier
        ++maxIndex
    }

    function removeTab(realIndex, tabObjectsIndex) {
        if(tabsModel.count <= 1) {
            //request close app
            return
        }

        tabObjects.splice(tabObjectsIndex, 1)
        tabsModel.remove(realIndex)
        tabObjects.sort(function(a, b){ return a.__index - b.__index })

        var tmp = new Array(); var i = 0
        for(var key in tabObjects)
            if(tabObjects.hasOwnProperty(key)) {
                tmp[i] = tabObjects[key]
                tmp[i].__index = i
                i++
            }

        --maxIndex
        tabObjects = tmp
    }

    ListModel { id: tabsModel }
    Repeater {
        id: repeater
        model: tabsModel
        delegate: Item {
            id: delegate
            property int __identifier: identifier
            property bool completed: false
            property int __index: -1
            property int baseX: __index * headerView.itemWidth - __index * 16
            z: identifier === headerView.currentIdentifier || delegateArea.pressed ? 1000000 : repeater.count - __index
            x: __index !== 0 ? baseX : 0
            height: 25
            width: headerView.itemWidth

            Component.onCompleted: {
                __index = headerView.maxIndex
                tabObjects[__index] = delegate
                completed = true
                headerView.tabAdded(identifier, url)
            }

            Behavior on x {
                enabled: delegate.completed && headerView.tabMoving
                NumberAnimation {
                    id: animation
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }

            Image {
                id: borderLeft
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                source: identifier === headerView.currentIdentifier ? "qrc:///images/ui/currentTabLeftBorder.png" :
                                                                      "qrc:///images/ui/tabLeftBorder.png"
            }

            Rectangle {
                id: contentRect
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: borderLeft.right
                anchors.right: borderRight.left

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 1
                    color: "#A9A9A9"
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#A9A9A9"
                    visible: identifier !== headerView.currentIdentifier
                }

                gradient: Gradient {
                    GradientStop { position: 0.0; color: identifier === headerView.currentIdentifier ? "#F6F6F6" : "#D2D2D2" }
                    GradientStop { position: 1.0; color: identifier === headerView.currentIdentifier ? "#E5E5E5" : "#BEBEBE" }
                }

                FavIcon {
                    id: favIcon
                    width: 16; height: 16
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: -6
                    site: url
                    visible: source !== "" && status === Image.Ready
                }

                Text {
                    id: contentText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: favIcon.visible ? 14 : 0
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    font.family: "Arial"
                    font.pixelSize: 12
                    renderType: Text.NativeRendering
                    elide: Text.ElideRight
                    color: "#6C6C6C"
                    text: name
                }
            }

            Image {
                id: borderRight
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                source: identifier === headerView.currentIdentifier ? "qrc:///images/ui/currentTabRightBorder.png" :
                                                                      "qrc:///images/ui/tabRightBorder.png"
            }

            MouseArea {
                id: delegateArea
                anchors.fill: parent
                hoverEnabled: true
                preventStealing: true
                drag.target: delegate
                drag.axis: Drag.XAxis
                drag.minimumX: 0
                drag.maximumX: headerView.itemWidth * repeater.count - repeater.count * 16

                property int prevX
                onPressed: {
                    delegate.x = delegate.x //removes property binding
                    prevX = delegate.x
                    headerView.currentIdentifier = identifier
                    headerView.tabMoving = true
                    headerView.currentTabChanged(identifier)
                }
                onReleased: {
                    headerView.tabMoving = false
                    delegate.x = Qt.binding(function(){ return delegate.__index !== 0 ? delegate.baseX : 0 })
                }
                onMouseXChanged: {
                    if(!pressed) return
                    var diff = delegate.x - prevX
                    prevX = delegate.x

                    if(diff > 0) {
                        var n = headerView.tabObjects[delegate.__index + 1]
                        if(n !== undefined && delegate.x + delegate.width / 2 > n.x) {
                            var mx = delegate.__index
                            var nx = n.__index

                            delegate.__index = nx
                            n.__index = mx

                            headerView.tabObjects[nx] = headerView.tabObjects[mx]
                            headerView.tabObjects[mx] = n
                        }
                    } else {
                        var n = headerView.tabObjects[delegate.__index - 1]
                        if(n !== undefined && delegate.x < n.x + n.width / 2) {
                            var mx = delegate.__index
                            var nx = n.__index

                            delegate.__index = nx
                            n.__index = mx

                            headerView.tabObjects[nx] = headerView.tabObjects[mx]
                            headerView.tabObjects[mx] = n
                        }
                    }
                }
            }

            Item {
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 14
                width: 10; height: 10

                Image {
                    anchors.centerIn: parent
                    source: "qrc:///images/ui/closeTabIcon.png"
                    opacity: {
                        if(closeArea.pressed) return 0.5
                        return closeArea.containsMouse ? 0.75 : 1
                    }
                }

                MouseArea {
                    id: closeArea
                    hoverEnabled: true
                    anchors.fill: parent
                    onClicked: {
                        var previous = identifier
                        var prev = headerView.tabObjects[delegate.__index - 1]
                        var next = headerView.tabObjects[delegate.__index + 1]

                        if(next !== undefined) {
                            previous = next.__identifier
                        } else if(prev !== undefined) {
                            previous = prev.__identifier
                        }

                        headerView.currentIdentifier = previous
                        headerView.tabRemoved(identifier, previous)
                        headerView.removeTab(index, delegate.__index)
                    }
                }
            }
        }
    }

    Image {
        anchors.verticalCenter: parent.verticalCenter
        x: headerView.itemWidth * repeater.count - 16 * repeater.count + 15
        opacity: headerView.tabMoving || tabsModel.count >= headerView.maxTabsCount + 1 ? 0 : 1

        Behavior on opacity {
            NumberAnimation { duration: 100; easing.type: Easing.InQuad }
        }

        source: {
            if(addButtonArea.pressed)
                return "qrc:///images/ui/addTabButtonPressed.png"
            if(addButtonArea.containsMouse)
                return "qrc:///images/ui/addTabButtonHovered.png"
            return "qrc:///images/ui/addTabButtonNormal.png"
        }

        MouseArea {
            id: addButtonArea
            anchors.fill: parent
            hoverEnabled: true
            enabled: tabsModel.count < headerView.maxTabsCount + 1
            onClicked: headerView.addTab(qsTr("New tab"), "")
        }
    }
}
