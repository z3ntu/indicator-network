import QtQuick 1.1
import components 1.0


Menu {
    id: mainMenu

    title: "MainMenu"
    stack: menuStack

    Column {
        id: menuList

        Column {
            Repeater {
                model: 10
                ToggleButton {
                    height: 48
                    width: mainMenu.width
                    caption: "Test: " + index
                }
            }
        }

        Column {
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
        }

        NavigationButton {
            width: mainMenu.width
            stack: menuStack
            caption: "Next Menu"
            next: Qt.createComponent("submenu.qml")
        }
    }
}
