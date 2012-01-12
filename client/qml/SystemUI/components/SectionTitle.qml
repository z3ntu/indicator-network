import QtQuick 1.1
import components 1.0

BasicItem {
    id: sectionTitle

    property alias caption: label.caption

    implicitHeight: 24
    implicitWidth: label.implicitWidth
    style: SectionTitleStyle { }

    Label {
        id:  label
        style: sectionTitle.style
        dbusModel: sectionTitle.dbusModel
        anchors.fill: parent
    }
}
