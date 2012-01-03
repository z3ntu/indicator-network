import QtQuick 1.1
import DBusMenu 1.0
import components 1.0
import SystemUI 1.0

Item {
    id: page

    property alias menuId: menuModel.menuId
    property alias control: menuModel.control
    property alias items: mainMenu.items
    property alias pageIndex: mainMenu.pageIndex
    property alias active: mainMenu.active

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

    Page {
        id: mainMenu

        property int pageIndex: 2
        property string activeUrl: ""

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
                comp.stack = pages
                comp.next = "ServiceSubPage.qml"
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

                    function onAboutToLoad(event)
                    {
                        var count = pages.count
                        while(count > page.pageIndex) {
                            pages.pop()
                            count--
                        }
                    }

                    function onPageLoaded(url, page)
                    {
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
