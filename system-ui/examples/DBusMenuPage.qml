import QtQuick 1.1
import components 1.0


Page {
    id: mainMenu

    property alias model: contents.model

    title: "DBusMenuPage"
    stack: pages

    Repeater {
        id: contents
        NavigationButton {
            width: mainMenu.width
            height: 48
            stack: pages
            caption: title
            next: hasSubmenu ? Qt.createComponent("DBusMenuPage.qml") : null
            onClicked: {
                if (hasSubmenu)
                    pages.currentPage.model = submenu
            }
        }
    }
}
