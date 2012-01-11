import QtQuick 1.1

BasicNavigationButton {
    id: button

    property alias caption: basicItem.caption
    property alias description: basicItem.description

    implicitHeight: 48

    BasicListItem {
        id: basicItem
        dbusModel: button.dbusModel
        anchors.fill: parent
    }
}
