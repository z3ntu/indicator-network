import QtQuick 1.1

BasicButton {
    id: button
    property alias caption: label.text

    style: ButtonStyle { }

    Rectangle {
        id: bg

        color: style.backgroundColor
        anchors.fill: parent
    }
    Text {
        id: label
        anchors.fill: parent
        anchors.centerIn: parent
        text:  button.dbusModel ? button.dbusModel.label : ""

        // Style
        color: button.style.foregroundColor
        font { family: button.style.fontFamily; pointSize: button.style.fontPointSize; bold: button.style.fontBold }
        anchors.margins: button.style.margin
    }
}
