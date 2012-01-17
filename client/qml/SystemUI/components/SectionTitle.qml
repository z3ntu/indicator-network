import QtQuick 1.1
import components 1.0

BasicItem {
    id: sectionTitle

    property alias caption: label.caption
    property alias busy: busyIndicator.busy

    implicitHeight: 24
    implicitWidth: label.implicitWidth

    style: SectionTitleStyle { }

    Label {
        id:  label

        style: sectionTitle.style
        dbusModel: sectionTitle.dbusModel
        anchors { left: parent.left; top: parent.top; right: busyIndicator.left; bottom: parent.bottom }
        anchors.rightMargin: sectionTitle.busy ? 6 : 0

    }

    AnimatedImage {
        id: busyIndicator

        property bool busy: sectionTitle.dbusModel && sectionTitle.dbusModel.properties.busy ? sectionTitle.dbusModel.properties.busy : false

        height: 16 //sourceSize.height
        width: busy ? 16 : 0//sourceSize.width

        paused: !busy
        source: sectionTitle.style.busyImage
        visible: !paused

        anchors { verticalCenter: parent.verticalCenter; right: parent.right }
        anchors.rightMargin: busy ? 6 : 0
    }

}

