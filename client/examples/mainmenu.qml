import QtQuick 1.1
import components 1.0


Page {
    id: mainMenu

    //title: "MainMenu"
    //stack: pageStack

    header: NavigationButton {
        width: 300
        height: 48
        stack: pageStack
        caption: "Main Menu"
        enableBackward: false
    }

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
        enableFoward: true
        next: Qt.createComponent("submenu.qml")
    }

    Button {
        caption: pageStack.visiblePages == 1 ? "Expand" : "Collapse"
        width: mainMenu.width
        height: 48
        onClicked: {
            if (pageStack.visiblePages == 1)
                pageStack.visiblePages = 3
            else
                pageStack.visiblePages = 1
        }
    }
}
