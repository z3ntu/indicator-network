import QtQuick 1.1

BasicButton {
    property alias caption: listItem.caption
    property alias description: listItem.description
    property alias selectable:  listItem.selectable
    property alias selected: listItem.selected

    style: listItem.style

    onClicked: {
        if (selectable)
            selected = !selected
    }

    BasicListItem {
        id: listItem

        anchors.fill: parent
    }
}
