import QtQuick 1.1
import SystemUI 1.0

BasicItem {
    id: page

    default property alias items: contents.children
    property Component header
    property int index: -1

    style: PageStyle { }
    implicitHeight: scroll.height

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
