import QtQuick 1.1
import DBusMenu 1.0

BasicItem {
    id: button

    signal clicked

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (dbusModel) {
                dbusModel.control.sendEvent(dbusModel.menuId, DBusMenuClientControl.Clicked, "")
            }
            button.clicked()
        }

        onPositionChanged: {
            if (dbusModel) {
                dbusModel.control.sendEvent(dbusModel.menuId, DBusMenuClientControl.Hovered, "")
            }
        }
    }
}
