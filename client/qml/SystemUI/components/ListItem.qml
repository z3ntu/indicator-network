import QtQuick 1.1

BasicButton {
    id: listButton

    property alias caption: listItem.caption
    property alias description: listItem.description
    property alias selectable: listItem.selectable
    property alias selected: listItem.selected

    style: listItem.style

    onClicked: {
        if (!hasModel && selectable) {
            selected = !selected
        }
    }

    BasicListItem {
        id: listItem

        dbusModel: listButton.dbusModel
        anchors.fill: parent
    }
}
