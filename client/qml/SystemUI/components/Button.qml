import QtQuick 1.1

BasicButton {
    id: button
    property alias caption: label.caption

    style: ButtonStyle { }

    Label {
        id: label
        style: button.style
        dbusModel: button.dbusModel
    }
}
