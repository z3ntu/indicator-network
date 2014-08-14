import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Connectivity 1.0

/*!
    brief MainView with a Label and Button elements.
*/

MainView {
    id: root
    // objectName for functional testing purposes (autopilot-qt5)
    objectName: "mainView"
    applicationName: "CurrencyConverter"

    width: units.gu(100)
    height: units.gu(75)

    property real margins: units.gu(2)
    property real buttonWidth: units.gu(9)

    NetworkingStatus {
        id: networkingStatus
        onStatusChanged: {
            if (status === NetworkingStatus.Offline)
                console.log("Status: Offline")
            if (status === NetworkingStatus.Connecting)
                console.log("Status: Connecting")
            if (status === NetworkingStatus.Online)
                console.log("Status: Online")
        }
    }

    Page {
        title: i18n.tr("Currency Converter")

        Label {
            anchors.centerIn: parent
            text: networkingStatus.status === NetworkingStatus.Online ? "Online" : "Not online"
            fontSize: "large"
        }
//        Label {
//            anchors.centerIn: parent
//            text: "Hello, world!"
//            fontSize: "large"
//        }

    }
}
