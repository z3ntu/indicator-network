import QtQuick 1.1
import SystemUI 1.0
import DBusMenu 1.0

Item {
    id: menuScreen

    width: 300
    height: 500

    DBusMenuClient {
        id: menuClient
        onConnected: {
            pages.push(Qt.createComponent("DBusMenuPage.qml"))
            pages.currentPage.model = menuClient
        }
    }

    PageStack {
        id: pages

        anchors.fill: parent
    }

    Component.onCompleted: menuClient.connectToServer("org.dbusmenu.test", "/org/test")
}
