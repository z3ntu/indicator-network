import QtQuick 1.1
import SystemUI 0.1
import components 1.0


Item {
    width: 300
    height: 800
    MenuStack {
        id: menuStack

        anchors.fill: parent
        Component.onCompleted: menuStack.pushMenu(Qt.createComponent("mainmenu.qml"))
    }
}
