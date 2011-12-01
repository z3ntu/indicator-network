import components 1.0
import QtQuick 1.1

Column {
    width: 300
    ListItem {
        height: 48
        width: parent.width
        caption: "Test without description"
    }

    ListItem {
        height: 48
        width: parent.width
        caption: "Test"
        description: "with description"
    }

    ListItem {
        selectable: true
        height: 48
        width: parent.width
        caption: "Test"
        description: "selectable"
    }
}


