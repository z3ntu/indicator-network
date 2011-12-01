import QtQuick 1.1

BasicItem {
    id: menu

    default property alias itens: contents.children
    property alias title: header.caption
    property alias stack: header.stack

    style: MenuStyle { }

    NavigationButton {
        id: header

        supportBack: (stack != null && stack.count > 1)
        style: menu.style.headerStyle
        height: menu.style.headerHeight
        anchors { top: parent.top; left: parent.left; right: parent.right }
    }

    Flickable {
        clip: true
        contentHeight: contents.height
        contentWidth: parent.width
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }

        Column {
            id: contents
            spacing: menu.style.stroke
        }
    }
}
