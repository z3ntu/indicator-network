import QtQuick 1.1

BasicItem {
    id: item

    property bool selectable: false
    property bool selected:  false
    property alias caption: label.text
    property alias description: labelDescription.text

    state: selected ? "SELECTED" : "UNSELECTED"
    style: BasicListItemStyle { }

    Rectangle {
        id: bg

        color: style.backgroundColor
        anchors.fill: parent

        Text {
            id: label

            text: hasModel ? dbusModel.label : ""
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            anchors { left: parent.left; top: parent.top; right: labelDescription.left; bottom: parent.bottom; margins: item.style.margin }

            // Style
            color: item.style.foregroundColor
            font { family: item.style.fontFamily; pointSize: item.style.fontPointSize; bold: item.style.fontBold }
       }

        Text {
            id: labelDescription

            width: implicitWidth
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            anchors { top: parent.top; right: selectImage.left; bottom: parent.bottom; margins: item.style.margin}

            // Style
            color: item.style.descriptionForegroundColor
            font { family: item.style.descriptionFontFamily; pointSize: item.style.descriptionFontPointSize; bold: item.style.descriptionFontBold; italic: item.style.descriptionFontItalic }
        }

        Image {
            id: selectImage

            visible: selectable
            height: sourceSize.height
            width: selectable ? sourceSize.width : 48
            anchors { top:  parent.top; right: parent.right; bottom: parent.bottom }
        }
    }

    states: [
        State {
            name: "SELECTED"
            PropertyChanges { target: selectImage; source: item.style.selectedImage }
        },
        State {
            name: "UNSELECTED"
            PropertyChanges { target: selectImage; source: item.style.unselectedImage }
        }
    ]
}
