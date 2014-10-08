import QtQuick 2.3
import QtWebKit 3.0
import QtWebKit.experimental 1.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "qrc:///components/ui"

Rectangle {
    id: module
    signal pageModified(int identifier, string url, string title)
    signal locationRequest(string txt)
    color: "#FFFFFF"

    Rectangle {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 40; z: 10

        gradient: Gradient {
            GradientStop { position: 0.2; color: "#E6E6E6" }
            GradientStop { position: 0.8; color: "#DADADA" }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: "#AAAAAA"
        }

        Row {
            anchors.fill: parent

            ToolButton {
                id: prevButton
                enabled: webView.canGoBack
                releasedImage: "qrc:///images/ui/prevPageIcon.png"
                disabledImage: "qrc:///images/ui/prevPageDisabledIcon.png"
                hoveredImage: "qrc:///images/ui/prevPageHoveredIcon.png"
                onClicked: webView.goBack()
            }

            ToolButton {
                id: nextButton
                enabled: webView.canGoForward
                releasedImage: "qrc:///images/ui/nextPageIcon.png"
                disabledImage: "qrc:///images/ui/nextPageDisabledIcon.png"
                hoveredImage: "qrc:///images/ui/nextPageHoveredIcon.png"
                onClicked: webView.goForward()
            }

            ToolButton {
                releasedImage: "qrc:///images/ui/refreshPageIcon.png"
                hoveredImage: "qrc:///images/ui/refreshPageHoveredIcon.png"
                onClicked: webView.reload()
            }

            LocationBar {
                id: locationBar
                anchors.verticalCenter: parent.verticalCenter
                property int mainWidth: parent.width - x - (20 + 40 + 40 + 40 + 40 + 40 + 43 + 120 + 120 + 70)
                property int currentWidth: parent.width / 2 - 70 - x
                width: currentWidth
                onAccepted: webView.url = txt.indexOf("http://") === 0 ? txt : "http://" + txt
                text: launchUrl
            }

            Item { // separator
                width: 10; height: 40
            }

            Rectangle { //separator
                anchors.verticalCenter: parent.verticalCenter
                width: 1; height: 26
                color: "#AAAAAA"
            }

            Item { // separator
                width: 10; height: 40
            }

            RolesDropdown {
                anchors.verticalCenter: parent.verticalCenter
                height: 26; width: 120
                onRoleChanged: roleInfo.name = txt
            }

            Item { // separator
                width: 10; height: 40
            }

            RoleInfo {
                id: roleInfo
                anchors.verticalCenter: parent.verticalCenter
                height: 26; width: 120
                image: "qrc:///images/ui/testThumbImage.png"
                name: qsTr("The Real Plato")
            }

            Item { // separator
                width: 10; height: 40
            }

            HomeButton {
                anchors.verticalCenter: parent.verticalCenter
                height: 40; width: 70
                onClicked: {
                    console.log("Home button clicked")
                    //webView.url = <home url>
                }
            }

            Item { //separator
                width: locationBar.mainWidth - locationBar.currentWidth
                height: 26
            }

            Rectangle { //separator
                anchors.verticalCenter: parent.verticalCenter
                width: 1; height: 26
                color: "#AAAAAA"
            }

            CreationStackButton {
                releasedImage: "qrc:///images/ui/creationStackIcon.png"
                hoveredImage: "qrc:///images/ui/creationStackHoveredIcon.png"
            }

            ApplicationsButton {
                releasedImage: "qrc:///images/ui/applicationsIcon.png"
                hoveredImage: "qrc:///images/ui/applicationsHoveredIcon.png"
            }

            Rectangle { //separator
                anchors.verticalCenter: parent.verticalCenter
                width: 1; height: 26
                color: "#AAAAAA"
            }

            PeersButton {
                releasedImage: "qrc:///images/ui/peersIcon.png"
                disabledImage: "qrc:///images/ui/peersDisabledIcon.png"
                hoveredImage: "qrc:///images/ui/peersHoveredIcon.png"
            }

            MessagesButton {
                releasedImage: "qrc:///images/ui/messagesIcon.png"
                disabledImage: "qrc:///images/ui/messagesDisabledIcon.png"
                hoveredImage: "qrc:///images/ui/messagesHoveredIcon.png"
            }

            NotificationsButton {
                releasedImage: "qrc:///images/ui/notificationsIcon.png"
                disabledImage: "qrc:///images/ui/notificationsDisabledIcon.png"
                hoveredImage: "qrc:///images/ui/notificationsHoveredIcon.png"
            }

            MoreButton {
                width: 20
                releasedImage: "qrc:///images/ui/moreIcon.png"
                hoveredImage: "qrc:///images/ui/moreHoveredIcon.png"
            }
        }
    }

    onWidthChanged: webView.returnToBounds()
    onHeightChanged: webView.returnToBounds()

    ScrollView {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom

        WebView {
            id: webView
            anchors.fill: parent
            //experimental.userAgent: "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/33.0.1750.146 Safari/537.36"
            //experimental.userAgent: "Mozilla/5.0 (Linux; U; Android 4.0.3; ko-kr; LG-L160L Build/IML74K) AppleWebkit/534.30    (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30"
            //experimental.userAgent: "Mozilla/5.0 (iPad; CPU OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5355d Safari/8536.25"
            url: launchUrl
            onTitleChanged: module.pageModified(identifier, url, title)
        }
    }
}
