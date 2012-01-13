import QtQuick 1.1
import components 1.0

BasicNavigationButton {
    id: button

    property alias caption: basicItem.caption
    property alias description: basicItem.description

    implicitHeight: 48

    function getImageFile() {
        var imageName = "nm-signal-100.svg"
        var strength = button.dbusModel.properties.wifi_strength
        if (button.dbusModel.properties.wifi_is_adhoc) {
            imgageName = "nm-adhoc.svg"
        } else if (strength == 0) {
            imageName = "nm-signal-00.svg"
        } else if (strength <= 25) {
            imageName = "nm-signal-25.svg"
        } else if (strength <= 50) {
            imageName = "nm-signal-50.svg"
        } else if (strength <= 75) {
            imageName = "nm-signal-75.svg"
        }
        return "images/" + imageName;
    }

    Image {
        id: wifiIcon

        height: sourceSize.height
        width: sourceSize.width
        source: button.hasModel  ?  getImageFile(button.dbusModel) : getImageFile(0)
        anchors { left: parent.left; verticalCenter: parent.verticalCenter }
    }

    BasicListItem {
        id: basicItem
        selectable: true
        dbusModel: button.dbusModel
        anchors { left: wifiIcon.right; top: parent.top; right: parent.right; bottom: parent.bottom }
    }
}
