import QtQuick 1.1
import DBusMenu 1.0
import components 1.0
import SystemUI 1.0

import "TypeDiscovery.js" as TypeDiscovery

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

                property QtObject delegate: null

                width: mainMenu.width
                height: delegate ? delegate.implicitHeight : 0

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
                    delegate = TypeDiscovery.createComponent(model, item)
                    delegate.anchors.fill = item

                    if (hasSubmenu && properties.group-type == "inline") {
                        delegate.aboutToLoad.connect(onAboutToLoad)
                        delegate.pageLoaded.connect(onPageLoaded)
                    }
                }
            }
        }
    }
}
