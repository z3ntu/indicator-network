import QtQuick 1.1
import components 1.0


Page {
    id: mainMenu

    property alias model: contents.model

    title: "DBusMenuPage"
    stack: pages

    function createComponent(model, parent)
    {
        var comp
        if (model.checkable) {
            comp = Qt.createQmlObject('import components 1.0; ToggleButton {}', parent, '')
        } else {
            comp = Qt.createQmlObject('import components 1.0; NavigationButton {}', parent, '')
            if (model.hasSubmenu)
                comp.next = Qt.createComponent("DBusMenuPage.qml")
        }
        comp.stack = stack
        comp.caption = model.title
        return comp
    }

    Repeater {
        id: contents
        Item {
            id: item

            width: mainMenu.width
            height: 48

            Component.onCompleted: {
                var comp = createComponent(model, item)
                comp.anchors.fill = item
                comp.clicked.connect(onClicked)
            }

            function onClicked()
            {
                if (!checkable && hasSubmenu)
                    pages.currentPage.model = submenu
            }
        }
    }
}
