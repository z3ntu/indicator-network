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
    property int maxItens: 4

    property int index: -1
    property QtObject dbusModel:  null

    visible: dbusModel ? dbusModel.visible : true
    implicitHeight: header.height + sectionContents.childrenRect.height

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

        Binding {
            target: header.delegate
            property: "dbusModel"
            value: section.dbusModel
            when: header.delegate != null
        }

        width: section.width
        height: delegate ? delegate.implicitHeight : 0

        onSourceChanged: {
            if (source.length > 0) {
                delegate = Qt.createQmlObject(source, header, '')
                if (!delegate) {
                    console.log("Fail to loader section header: " + delegate.errorString())
                } else {
                    delegate.anchors.fill = header
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
                visible: ((section.maxItens == -1) || (index < section.maxItens))

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

        NavigationButton {
            id: moreItens

            visible: section.maxItens > -1 && sectionModel.count > section.maxItens
            height: visible ? 48 : 0
            width: section.width
            caption: "More items"
            next: Qt.createComponent("ServiceSubPage.qml")
            enableFoward: true
            stack: pages

            onAboutToLoad: {
                var count = pages.count
                while(count > section.index) {
                    pages.pop()
                    count--
                }
            }

            onPageLoaded: {
                page.title = header.delegate ? header.delegate.caption : ""
                page.menuId = sectionModel.menuId
                page.index = pages.count
                page.loadMenu()
            }
        }
    }
}
