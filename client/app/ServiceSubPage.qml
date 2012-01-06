import QtQuick 1.1
import DBusMenu 1.0
import components 1.0
import SystemUI 1.0

Item {
    id: page

    property alias menuId: menuModel.menuId
    property alias control: menuModel.control
    property alias items: mainMenu.items
    property alias header: mainMenu.header
    property alias index: mainMenu.index

    property string title:  "Service"


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
                comp.next = Qt.createComponent("ServiceSubPage.qml")
            }
            comp.dbusModel = model
            return comp
        }

        header: NavigationButton {
            caption: page.title
            height: 48
            width: page.width
            stack: pages
            enableBackward: true
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
                        while(count > page.index + 1) {
                            pages.pop()
                            count--
                        }
                    }

                    function onPageLoaded(page)
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
