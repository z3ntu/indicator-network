import QtQuick 1.1

BasicItem {
    id: button

    signal clicked

    MouseArea {
        anchors.fill: parent
        onClicked: button.clicked()
    }
}
