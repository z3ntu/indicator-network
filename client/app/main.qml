import QtQuick 1.1
import DBusMenu 1.0
import SystemUI 1.0
import components 1.0

Item {
    id: stage

    height: 700
    width: pages.implicitWidth
    onWidthChanged: console.log("WIDTH CHANGED: " + width)

    function addService(description, serviceName, objectPath) {
        servicesModel.append({"description" : description,
                              "serviceName" : serviceName,
                              "objectPath" : objectPath } )
    }

    DBusMenuClientControl {
        id: menuControl
    }

    ListModel {
        id: servicesModel
    }

    PageStack {
        id: pages

        anchors.fill: parent
        pageWidth: 300
        spacing: 3
        visiblePages: 3

        onPageLoaded: {
            if (pageIndex == 0) {
                // First page is the service list
                page.servicesModel = servicesModel
            } else {
                // Second page need dbus control
                page.control = menuControl
            }
        }

        Component.onCompleted: {
            pages.push("ServiceListPage.qml", "Service List")
        }
    }
}
