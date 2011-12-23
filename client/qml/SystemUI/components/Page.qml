import QtQuick 1.1
import SystemUI 1.0

BasicItem {
    id: page

    default property alias itens: contents.children

    style: PageStyle { }
    implicitHeight: scroll.height

//    Rectangle {
//        color: style.backgroundColor
//        anchors.fill: parent
//    }

    Flickable {
        id: scroll

        clip: true
        contentHeight: contents.height
        contentWidth: parent.width
        anchors.fill:  parent

        Column {
            id: contents
            spacing: page.style.stroke
        }
    }
}
