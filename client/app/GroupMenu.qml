import QtQuick 1.1
import DBusMenu 1.0
import components 1.0
import SystemUI 1.0
import "MenuFactory.js" as Engine

Item {
    id: group

    property alias menuId: menuModel.menuId
    property alias control: menuModel.control
    property alias items: mainMenu.items
    property alias header: mainMenu.header
    property alias index: mainMenu.index

    function loadMenu() {
        menuModel.load()
    }

    DBusMenuClientModel {
        id: menuModel

        menuId: -1
        onControlChanged: {
            control.rootChanged.connect(loadMenu)
        }
    }

    Column {
        Repeater {
            id: list

            model: menuModel
            Item {
                id: item

                width: group.width
                height: delegate.implicitHeight

                Item {
                    id: delegate

                    anchors.fill: parent

                    unction onPageLoaded(page)
                    {
                        page.title = model.label
                        page.menuId = model.menuId
                        page.pageIndex = pages.count
                        page.loadMenu()
                    }

                    Component.onCompleted: {
                        var comp = mainMenu.createComponent(model, parent)
                        implicitHeight = comp.implicitHeight
                        comp.anchors.fill = delegate

                        if (hasSubmenu) {
                            comp.aboutToLoad.connect(onAboutToLoad)
                            comp.pageLoaded.connect(onPageLoaded)
                        }
                    }
                }
            }
        }
    }
}
