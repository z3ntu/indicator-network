import QtQuick 1.1
import components 1.0

Item {
    height: 48
    width: 300
    ToggleButton {
        height: implicitHeight
        width: parent.width
        anchors.centerIn: parent
        caption: "Test Toggle"
    }
}


