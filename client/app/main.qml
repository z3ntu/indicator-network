import QtQuick 1.1
import SystemUI 1.0

Flickable {
    id: stage

    property int maxWidth: 900

    height: 700
    width: Math.min(content.width, maxWidth)
    contentWidth: content.width

    function addService(description, serviceName, objectPath) {
        servicesModel.append({"description" : description,
                              "serviceName" : serviceName,
                              "objectPath" : objectPath } )
    }

    Behavior on contentX {
        SmoothedAnimation { velocity: 200 }
    }

    onMovementEnded: {
        var newX = contentX % (pages.pageWidth + pages.spacing)
        contentX -= newX
    }

    Rectangle {
        id: content

        height: parent.height
        width: pages.width
        color: "gray"

        ListModel {
            id: servicesModel
        }

        PageStack {
            id: pages

            height: parent.height
            pageWidth: 300
            spacing: 3
            layout: PageStack.Stage

            onAboutToRemovePage: {
                stage.contentX -= pageWidth
            }

            onCountChanged: {
                stage.contentX = pages.width - stage.width
            }

            Component.onCompleted: {
                pages.push(Qt.createComponent("ServiceListPage.qml"))
                pages.currentPage.servicesModel = servicesModel
            }
        }
    }
}
