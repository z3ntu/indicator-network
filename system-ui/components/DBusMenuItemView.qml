import QtQuick 1.1
import DBusMenu 1.0

MouseArea {
    id: dbusMouseArea

    property QtObject control: null
    property int menuId: -1

    onClicked: {
        if (control && menuId != -1) {
            console.debug("CONTROL: " + control +  " ID:" + menuId)
            control.sendEvent(menuId, DBusMenuClientControl.Clicked)
        }
    }

    onPositionChanged: {
        if (control && menuId != -1) {
            control.sendEvent(menuId, DBusMenuClientControl.Hovered)
        }
    }
}
