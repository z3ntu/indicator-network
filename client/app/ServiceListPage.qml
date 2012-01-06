import QtQuick 1.1
import SystemUI 1.0
import components 1.0


Page {
    id: page

    property alias servicesModel: serviceList.model

    header: NavigationButton {
        caption: "Service List"
        height: 48
        width: page.width
        stack: pages
    }

    Repeater {
        id: serviceList

        NavigationButton {
            id: serviceButtom

            height: 48
            width: page.width
            caption: model.description
            next: Qt.createComponent("ServiceSubPage.qml")
            enableFoward: true
            stack: pages

            onAboutToLoad: {
                var count = pages.count
                while(count > 1) {
                    pages.pop()
                    count--
                }
            }

            onPageLoaded: {
                menuControl.disconnectFromServer()
                menuControl.service = model.serviceName
                menuControl.objectPath = model.objectPath

                page.title = model.description
                page.menuId = 1 // Skip root menu
                menuControl.connectToServer()
            }
        }
    }

    Button {
        height: 48
        width: page.width
        caption: pages.visiblePages == 1 ? "Expand" : "Collapse"
        onClicked: {
            if (pages.visiblePages == 1)
                pages.visiblePages = 3
            else
                pages.visiblePages = 1
        }
    }
}
