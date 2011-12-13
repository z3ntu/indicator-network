import QtQuick 1.1

BasicItem {
    id: page

    default property alias itens: contents.children
    property alias title: header.caption
    property alias stack: header.stack

    style: PageStyle { }

    NavigationButton {
        id: header

        enableBackward: (stack != null && stack.count > 1)
        style: page.style.headerStyle
        height: page.style.headerHeight
        anchors { top: parent.top; left: parent.left; right: parent.right }
    }

    Flickable {
        clip: true
        contentHeight: contents.height
        contentWidth: parent.width
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }

        Column {
            id: contents
            spacing: page.style.stroke
        }
    }
}
