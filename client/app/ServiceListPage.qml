import QtQuick 1.1
import SystemUI 1.0
import components 1.0
import DBusMenu 1.0

Item {
    id: root
    property alias servicesModel: serviceList.model

    DBusMenuClientControl {
        id: menuControl
    }

    Page {
        anchors.fill: parent
        title: "Avaliable Services"

        Repeater {
            id: serviceList


            model: serviceModel
            NavigationButton {
                id: serviceButtom

                height: 48
                width: root.width
                caption: model.description
                next: Qt.createComponent("ServiceSubPage.qml")
                enableFoward: true
                stack: pages

                onPageLoaded: {
                    console.log("Service activated")
                    menuControl.disconnectFromServer()
                    menuControl.service = model.serviceName
                    menuControl.objectPath = model.objectPath

                    newPage.title = model.description
                    newPage.control = menuControl
                    newPage.menuId = 1 // Skip root menu
                    menuControl.connectToServer()
                }

                Component.onCompleted: {
                    console.log("LOADED:", model.description)
                }
            }
        }
    }
}

