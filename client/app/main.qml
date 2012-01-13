import QtQuick 1.1
import DBusMenu 1.0
import SystemUI 1.0
import components 1.0

Item {
    id: stage

    height: 600
    width: pages.implicitWidth

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
        visiblePages: 1

        Rectangle {
            anchors.fill: parent
            color: "#646464"
        }

        onPageLoaded: {
            if (page.index > 0) {
                page.control = menuControl
            }
        }

        Component.onCompleted: {
            var page = pages.push(Qt.createComponent("ServiceListPage.qml"))
            page.servicesModel = servicesModel
        }
    }
}
