import QtQuick 1.1
import SystemUI 1.0
import components 1.0


Page {
    id: page

    property alias servicesModel: serviceList.model

    Repeater {
        id: serviceList

        NavigationButton {
            id: serviceButtom

            height: 48
            width: page.width
            caption: model.description
            next: "ServiceSubPage.qml"
            enableFoward: true
            stack: pages

//                onAboutToLoad: {
//                    while(pages.count > 1) {
//                        pages.pop()
//                    }
//                }

            onPageLoaded: {
                menuControl.disconnectFromServer()
                menuControl.service = model.serviceName
                menuControl.objectPath = model.objectPath

                newPage.menuId = 1 // Skip root menu
                menuControl.connectToServer()
            }
        }
    }
}

