import QtQuick 1.1

BasicButton {
    property alias caption: label.text

    caption: dbusModel ? dbusModel.label : ""
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

        // Style
        color: sytle.foregroundColor
        font { family: style.fontFamily; pointSize: style.fontPointSize; bold: style.fontBold }
        anchors.margins: style.margin
    }
}
