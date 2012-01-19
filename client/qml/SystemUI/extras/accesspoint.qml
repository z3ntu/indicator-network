import QtQuick 1.1
import components 1.0

BasicNavigationButton {
    id: button

    property alias caption: basicItem.caption
    property alias description: basicItem.description

    implicitHeight: 48

    function getImageFile() {
        var imageName = "nm-signal-100"
        var strength = button.dbusModel.properties.wifi_strength
        if (button.dbusModel.properties.wifi_is_adhoc) {
            imageName = "nm-adhoc"
        } else if (strength == 0) {
            imageName = "nm-signal-00"
        } else if (strength <= 25) {
            imageName = "nm-signal-25"
        } else if (strength <= 50) {
            imageName = "nm-signal-50"
        } else if (strength <= 75) {
            imageName = "nm-signal-75"
        }

        if (button.dbusModel.properties.wifi_is_secure) {
            imageName += "-secure"
        }

        return "images/" + imageName + ".svg";
    }

    Item {
        id: wifiIconFrame

        width: 48
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        Image {
            id: wifiIcon

            height: sourceSize.height
            width: sourceSize.width
            source: button.hasModel  ?  getImageFile(button.dbusModel) : getImageFile(0)
            anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter; }
        }
    }

    ListItem {
        id: basicItem
        selectable: button.dbusModel.toggleType != ""
        dbusModel: button.dbusModel
        anchors { left: wifiIconFrame.right; top: parent.top; right: parent.right; bottom: parent.bottom }
    }
}
