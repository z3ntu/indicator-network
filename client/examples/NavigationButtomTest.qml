import components 1.0
import QtQuick 1.1

Column {
    width: 300

    NavigationButton {
        height: 48
        width: parent.width
        next: Item { }
        caption: "Test"
        description: "Foward"
        enableFoward: true
    }

    NavigationButton {
        height: 48
        width: parent.width
        enableBackward: true
        caption: "Test"
        description: "Back"
    }

    BasicNavigationButton {
        height: 48
        width: parent.width
        enableFoward: true
        ListItem {
            caption: "Test"
            description: "Back"
            selectable: true
            anchors.fill: parent
        }
    }
}


