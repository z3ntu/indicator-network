import QtQuick 1.1
import DBusMenu 1.0
import components 1.0
import SystemUI 1.0

Item {
    id: page

    property alias menuId: menuModel.menuId
    property alias control: menuModel.control

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
        property int activeItem: -1

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
                        if (mainMenu.activeItem == index) {
                            event.skip = true
                            return;
                        }

                        //while(pages.count > subPage.pageIndex) {
                        //    pages.pop()
                        //}
                        return false;
                    }

                    function onPageLoaded(page)
                    {
                        //if (page.parent.layout == PageStackModel.Stage) {
                        //    mainMenu.activeItem = index
                        //}
                        page.menuId = model.menuId
                        page.pageIndex = pages.count
                        page.loadMenu()
                    }

                    Component.onCompleted: {
                        var comp = mainMenu.createComponent(model, delegate)
                        implicitHeight = comp.implicitHeight
                        comp.anchors.fill = delegate

                        if (hasSubmenu) {
                            comp.pageLoaded.connect(onPageLoaded)
                            //stack.pageLoaded.connect(onPageLoaded)
                            //stack.aboutToLoad.connect(onAboutToLoad)
                        }
                    }
                }
            }
        }
    }
}
