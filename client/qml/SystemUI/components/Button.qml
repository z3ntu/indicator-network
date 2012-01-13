import QtQuick 1.1

BasicButton {
    id: button
    property alias caption: label.caption

    style: ButtonStyle { }

    implicitWidth: label.implicitWidth
    implicitHeight: label.implicitHeight

    Label {
        id: label
        style: button.style
        anchors.fill: parent
        dbusModel: button.dbusModel
    }
}
