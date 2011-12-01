import QtQuick 1.1

BasicButton {
    id: button

    property bool toggled: false
    property alias caption: label.text

    style: ToggleButtonStyle { }
    state: toggled ? "ON" : "OFF"
    onClicked: toggled = !toggled
    implicitWidth: label.implicitWidth + toggle.width
    implicitHeight: toggle.height

    Rectangle {
        id: bg

        anchors { top: parent.top; left: parent.left; right: toggle.left; bottom: parent.bottom }
        Text {
            id: label

            font { family: button.style.fontFamily; pointSize: button.style.fontPointSize; bold: button.style.fontBold }
            anchors { margins: button.style.margin; left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
        }
    }

    Rectangle {
        id: toggle

        width: 48
        height: 48
        anchors.right: parent.right
        Text {
            id: toggleLabel

            font { family: button.style.fontFamily; pointSize: button.style.fontPointSize; bold: button.style.fontBold }
            anchors.centerIn: parent
        }
    }

    states: [
        State {
            name: "OFF"
            PropertyChanges { target: bg; color: button.style.backgroundColor }
            PropertyChanges { target: label; color: button.style.foregroundColor }
            PropertyChanges { target: toggle; color: button.style.toggleDeactiveBackgroundColor }
            PropertyChanges { target: toggleLabel; color: button.style.toggleDeactiveForegroundColor; text: "Off" }
        },
        State {
            name: "ON"
            PropertyChanges { target: bg; color: button.style.activeBackgroundColor }
            PropertyChanges { target: label; color: button.style.activeForegroundColor }
            PropertyChanges { target: toggle; color: button.style.toggleActiveBackgroundColor }
            PropertyChanges { target: toggleLabel; color: button.style.toggleActiveForegroundColor; text: "On" }
        }
    ]
}
