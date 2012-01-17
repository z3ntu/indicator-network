import QtQuick 1.1
import components 1.0

BasicItem {
    id: sectionTitle

    property alias caption: label.caption
    //operty alias busy: busyIndicator.playing

    implicitHeight: 24
    implicitWidth: label.implicitWidth

    style: SectionTitleStyle { }

    Label {
        id:  label
        style: sectionTitle.style
        dbusModel: sectionTitle.dbusModel
        anchors { left: parent.left; top: parent.top; right: busyIndicator.left; bottom: parent.bottom }

    }

//    Rectangle {
//        id: busyIndicator

//        property bool busy: sectionTitle.dbusModel && sectionTitle.dbusModel.properties.busy ? sectionTitle.dbusModel.properties.busy : false

//        color: busy ? "red" : "blue"
//        height: 10
//        width: 10

//        anchors { verticalCenter: parent.verticalCenter; right: parent.right }
//    }

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

