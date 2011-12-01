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
    }

    NavigationButton {
        height: 48
        width: parent.width
        supportBack: true
        caption: "Test"
        description: "Back"
    }

}


