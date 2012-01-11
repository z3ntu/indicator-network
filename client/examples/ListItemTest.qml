import components 1.0
import QtQuick 1.1

Item {
    width: 300
    height: itemList.height

    ListButtonGroup {
        id: listGroup
    }

    Column {
        id: itemList

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

        Text {
            text: "group"
            width: parent.width
            height: 48
        }

        ListItem {
            selectable: true
            group: listGroup
            width: parent.width
            height: 48
            caption: "Group test 0"
        }

        ListItem {
            selectable: true
            group: listGroup
            width: parent.width
            height: 48
            caption: "Group test 1"
        }
    }
}

