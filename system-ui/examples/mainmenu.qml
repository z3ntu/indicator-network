import QtQuick 1.1
import components 1.0


Page {
    id: mainMenu

    title: "MainMenu"
    stack: pageStack

    Repeater {
        model: 10
        ToggleButton {
            height: 48
            width: mainMenu.width
            caption: "Test: " + index
        }
    }

    Repeater {
        model: 10
        ListItem {
            height: 48
            width: mainMenu.width
            caption: "List Item"
            description: "item " + index
            selectable: index > 2
        }
    }

    NavigationButton {
        width: mainMenu.width
        stack: pageStack
        caption: "Next Menu"
        next: Qt.createComponent("submenu.qml")
    }
}
