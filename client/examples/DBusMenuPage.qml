import QtQuick 1.1
import DBusMenu 1.0
import components 1.0

Item {
    property alias menuId: menuModel.menuId
    property alias title: mainMenu.title

    DBusMenuClientModel {
        id: menuModel

        control: menuControl
        menuId: -1
        onMenuIdChanged: load()
    }

    Page {
        id: mainMenu

        title: "System Settings"
        stack: pages
        anchors.fill: parent

        function createComponent(model, parent)
        {
            var comp
            if (model.type == "TextEntry") {
                comp = Qt.createQmlObject('import components 1.0; TextEntry {}', parent, '')
            } else if (model.type == "ToggleButton") {
                comp = Qt.createQmlObject('import components 1.0; ToggleButton {}', parent, '')
            } else {
                comp = Qt.createQmlObject('import components 1.0; NavigationButton {}', parent, '')
                comp.stack = stack
                comp.next = Qt.createComponent("DBusMenuPage.qml")
            }
            comp.dbusModel = model
            return comp
        }

        Repeater {
            id: contents

            model: menuModel
            Item {
                id: item

                width: mainMenu.width
                height: delegate.implicitHeight

                Item {
                    id: delegate

                    anchors.fill: parent

                    Component.onCompleted: {
                        var comp = mainMenu.createComponent(model, delegate)
                        implicitHeight = comp.implicitHeight
                        comp.anchors.fill = delegate
                        if (hasSubmenu)
                            comp.clicked.connect(onClicked)
                    }

                    function onClicked(mouse)
                    {
                        if (hasSubmenu) {
                            pages.currentPage.menuId = model.menuId
                            pages.currentPage.title = label
                        }
                    }

                }
            }
        }
    }
}
