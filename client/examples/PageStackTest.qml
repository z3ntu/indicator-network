import QtQuick 1.1
import SystemUI 1.0
import components 1.0


Item {
    width: 300
    height: 800
    PageStack {
        id: pageStack

        anchors.fill: parent
        Component.onCompleted: pageStack.push(Qt.createComponent("mainmenu.qml"))
    }
}
