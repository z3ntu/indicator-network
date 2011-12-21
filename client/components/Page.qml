import QtQuick 1.1
import SystemUI 1.0

BasicItem {
    id: page

    default property alias itens: contents.children
    property alias title: header.caption
    property alias stack: header.stack

    style: PageStyle { }

    Rectangle {
        color: style.backgroundColor
        anchors.fill: parent
    }


    NavigationButton {
        id: header

        enableBackward: (stack != null && stack.count > 1 && stack.layout == PageStack.Slider)
        style: page.style.headerStyle
        height: page.style.headerHeight
        anchors { top: parent.top; left: parent.left; right: parent.right }
    }

    Flickable {
        clip: true
        contentHeight: contents.height
        contentWidth: parent.width
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; topMargin: page.style.stroke}

        Column {
            id: contents
            spacing: page.style.stroke
        }
    }
}
