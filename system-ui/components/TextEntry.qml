import QtQuick 1.1
import DBusMenu 1.0

BasicItem {
    id: textEntry

    property alias text: textInput.text

    style: TextEntryStyle { }

    TextInput {
        id: textInput

        anchors.fill: parent
        anchors.margins: textEntry.style.margin
        font { family: textEntry.style.fontFamily; pointSize: textEntry.style.fontPointSize; bold: textEntry.style.fontBold }

        onAccepted: {
            if (dbusModel) {
                dbusModel.control.sendEvent(dbusModel.id, DBusMenuClientControl.TextChanged, text)
            }
        }

        Text {
            text: dbusModel ? dbusModel.title : ""
            visible: textInput.text == "" && !textInput.activeFocus
            anchors.fill: parent
            font { family: textEntry.style.fontFamily; pointSize: textEntry.style.fontPointSize; bold: textEntry.style.fontBold }
        }
    }
}
