import QtQuick 1.1
import DBusMenu 1.0
import components 1.0
import SystemUI 1.0

import "TypeDiscovery.js" as TypeDiscovery

Item {
    id: section

    property alias menuId: sectionModel.menuId
    property alias control: sectionModel.control
    property alias headerSource: header.source

    property int index: -1
    property QtObject dbusModel:  null

    implicitHeight: header.height + sectionContents.childrenRect.height
    onImplicitHeightChanged: {
        console.log("HEIGHT CHANGED: " + implicitHeight)
    }

    function load() {
        sectionModel.load()
    }

    DBusMenuClientModel {
        id: sectionModel

        menuId: -1
        onControlChanged: {
            control.rootChanged.connect(loadMenu)
        }
    }

    Item {
        id: header

        property QtObject delegate: null
        property string source

        width: section.width
        height: delegate ? delegate.implicitHeight : 0
        onSourceChanged: {
            if (source.length > 0) {
                console.debug("LOAD OBJ: " + source)
                delegate = Qt.createQmlObject(source, header, '')
                if (!delegate) {
                    console.log("FAIL TO LOADER SECTION HEADER: " + delegate.errorString())
                } else {
                    delegate.anchors.fill = header
                    delegate.dbusModel = section.dbusModel
                }
            }
        }
    }

    Column {
        id: sectionContents

        anchors { left: parent.left; top: header.bottom; right: parent.right }
        spacing: 1

        Repeater {
            model: sectionModel
            Item {
                id: item

                property QtObject delegate: null
                width: section.width
                height: delegate ? delegate.implicitHeight : 0

                function onAboutToLoad(event)
                {
                    var count = pages.count
                    while(count > section.index + 1) {
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

                    if (hasSubmenu) {
                        delegate.aboutToLoad.connect(section.aboutToLoad)
                        delegate.pageLoaded.connect(section.pageLoaded)
                    }
                }
            }
        }
    }
}
