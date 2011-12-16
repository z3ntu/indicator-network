import QtQuick 1.1
import SystemUI 1.0

Item {
    height: 700
    width: 300

    function addService(description, serviceName, objectPath) {
        servicesModel.append({"description" : description,
                              "serviceName" : serviceName,
                              "objectPath" : objectPath } )
    }

    ListModel {
        id: servicesModel
    }

    PageStack {
        id: pages

        anchors.fill: parent
        Component.onCompleted: {
            pages.push(Qt.createComponent("ServiceListPage.qml"))
            pages.currentPage.servicesModel = servicesModel
            pages.currentPage.anchors.fill = pages
        }
    }
}
