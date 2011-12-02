import QtQuick 1.1
import DBusMenu 1.0
import components 1.0

Item {
    property alias menuId: menuModel.menuId

    DBusMenuClientModel {
        id: menuModel

        control: menuControl
        menuId: -1
        onMenuIdChanged: load()
    }

    Page {
        id: mainMenu

        title: "DBusMenuPage"
        stack: pages
        anchors.fill: parent

        function createComponent(model, parent)
        {
            var comp
            if (model.checkable) {
                comp = Qt.createQmlObject('import components 1.0; ToggleButton {}', parent, '')
            } else {
                comp = Qt.createQmlObject('import components 1.0; NavigationButton {}', parent, '')
                comp.stack = stack
                if (model.hasSubmenu)
                    comp.next = Qt.createComponent("DBusMenuPage.qml")
            }
            comp.caption = model.title
            return comp
        }

        Repeater {
            id: contents

            model: menuModel
            Item {
                id: item

                width: mainMenu.width
                height: 48

                DBusMenuItemView {
                    id: view

                    anchors.fill: parent
                    control: menuModel.control
                    menuId: menuModel.menuId
                }


                Item {
                    id: delegate

                    anchors.fill: parent

                    Component.onCompleted: {
                        var comp = mainMenu.createComponent(model, delegate)
                        comp.anchors.fill = delegate
                        comp.clicked.connect(onClicked)
                    }

                    function onClicked(mouse)
                    {
                        view.clicked(mouse)
                        if (!checkable && hasSubmenu)
                            pages.currentPage.menuId = id
                    }

                }
            }
        }
    }
}
