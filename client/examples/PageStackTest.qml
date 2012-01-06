import QtQuick 1.1
import SystemUI 1.0
import components 1.0


Item {
    width: pageStack.implicitWidth
    height: 800

    PageStack {
        id: pageStack
        height: parent.height
        width: parent.width

        pageWidth: 300
        spacing: 3
        visiblePages: 1

        Rectangle {
            anchors.fill: parent
            color: "gray"
        }

        Component.onCompleted: pageStack.push(Qt.createComponent("mainmenu.qml"))
    }
}
