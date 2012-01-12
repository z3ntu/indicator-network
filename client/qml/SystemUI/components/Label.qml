import QtQuick 1.1
import components 1.0

BasicItem {
    id: label

    property alias caption: view.text

    implicitHeight: 48
    implicitWidth: view.implicitWidth

    Rectangle {
        id: bg

        color: style.backgroundColor
        anchors.fill: parent
    }

    Text {
        id: view
        anchors.fill: parent
        anchors.centerIn: parent
        text:  label.hasModel ? label.dbusModel.label : ""

        // Style
        color: label.style.foregroundColor
        font { family: label.style.fontFamily; pointSize: label.style.fontPointSize; bold: label.style.fontBold }
        anchors {margins: label.style.margin; verticalCenter: label.verticalCenter }
    }
}
