import QtQuick 1.1

BasicButton {
    id: button

    property QtObject stack: null
    property Component next: null
    property bool supportBack: false
    property alias caption: basicItem.caption
    property alias description: basicItem.description

    style: NavigationButtonStyle { }
    implicitWidth: backIcon.width + basicItem.implicitWidth + fowardIcon.width
    implicitHeight: Math.max(backIcon.height, fowardIcon.height)

    onClicked: {
        if (!stack)
            return

        if (supportBack) {
            stack.pop()
        } else if (next) {
            stack.push(next)
        }
    }

    Rectangle {
        id: bg

        color: style.backgroundColor
        anchors.fill: parent
    }

    Image {
        id: backIcon

        height: sourceSize.height
        width: sourceSize.width
        source: supportBack ? button.style.backImage : ""
        anchors { left: parent.left; top: parent.top }
    }

    BasicListItem {
        id: basicItem
        style: button.style

        anchors { left: backIcon.right; top: parent.top;  right: fowardIcon.left; bottom: parent.bottom }
    }

    Image {
       id: fowardIcon

       height: sourceSize.height
       width: sourceSize.width
       source: next ? button.style.fowardImage : ""
       anchors { top: parent.top; right: parent.right }
    }
}
