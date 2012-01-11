import QtQuick 1.1

BasicButton {
    id: listButton

    property QtObject group: null
    property QtObject _actualGroup: null

    property alias caption: listItem.caption
    property alias description: listItem.description
    property alias selectable: listItem.selectable
    property alias selected: listItem.selected

    style: listItem.style

    onClicked: {
        if (selectable) {
            selected = !selected
            if (group) {
                group.selectButton(listButton)
            }
        }
    }

    onGroupChanged: {
        if (_actualGroup) {
            _actualGroup.removeButton(listButton)
        }
        group.addButton(listButton)
        _actualGroup = group
    }

    BasicListItem {
        id: listItem

        anchors.fill: parent
    }

    Component.onDestruction: {
        if (group)
            group.removeButton(listButton)
    }
}
