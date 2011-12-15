import QtQuick 1.1
import SystemUI 1.0
import DBusMenu 1.0

Item {
    id: menuScreen

    width: 300
    height: 500

    DBusMenuClientControl {
        id: menuControl

        service: "org.dbusmenu.test"
        objectPath: "/org/test"

        onConnectedChanged: {
            if (connected) {
                pages.push(Qt.createComponent("DBusMenuPage.qml"))
                pages.currentPage.menuId = 0
            }
        }
    }

    PageStack {
        id: pages

        anchors.fill: parent
    }

    Component.onCompleted: menuControl.connectToServer()
}
