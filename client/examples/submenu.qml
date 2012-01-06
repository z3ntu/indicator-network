import QtQuick 1.1
import components 1.0

Page {
    id: mainMenu
    property string title

    header: NavigationButton {
        width: 300
        height: 48
        stack: pageStack
        enableBackward: true
        Component.onCompleted: caption = "SubMenu" + pageStack.count
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
        Slider {
            height: 48
            value: Math.random()
            width: mainMenu.width
        }
    }
    NavigationButton {
        width: 300
        height: 48
        stack: pageStack
        caption: "Next Menu"
        enableFoward: true
        next: Qt.createComponent("submenu.qml")
    }
}
