import QtQuick 1.1
import DBusMenu 1.0
import components 1.0

Item {
    id: subPage

    property alias menuId: menuModel.menuId
    property alias title: mainMenu.title
    property alias control: menuModel.control

    function loadMenu() {
        menuModel.load()
    }

    DBusMenuClientModel {
        id: menuModel
        menuId: -1
        onControlChanged: control.rootChanged.connect(loadMenu)
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
                comp.next = Qt.createComponent("ServiceSubPage.qml")
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
                        if (hasSubmenu) {
                            comp.pageLoaded.connect(pageLoaded)
                        }
                    }

                    function pageLoaded(newPage)
                    {
                        newPage.control = subPage.control
                        newPage.menuId = model.menuId
                        newPage.title = label
                        newPage.loadMenu()
                    }
                }
            }
        }
    }
}
