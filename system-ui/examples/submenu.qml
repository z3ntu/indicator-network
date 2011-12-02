import QtQuick 1.1
import components 1.0

Menu {
    id: mainMenu
    title: "Sub Menu"
    stack: menuStack

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
        Slider {
            height: 48
            width: mainMenu.width
        }
    }
}
