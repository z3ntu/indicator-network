import QtQuick 1.1

BasicButton {
    id: button

    property QtObject stack: null
    property Component next: null
    property bool enableBackward: false
    property bool enableFoward: dbusModel ? dbusModel.hasSubmenu : false
    property alias caption: basicItem.caption
    property alias description: basicItem.description

    signal pageLoaded(variant newPage)
    signal aboutToLoad(variant accept)

    style: NavigationButtonStyle { }
    implicitWidth: backIcon.width + basicItem.implicitWidth + fowardIcon.width
    implicitHeight: Math.max(backIcon.height, fowardIcon.height)

    onClicked: {
        if (!stack)
            return

        if (enableBackward) {
            stack.pop()
        } else if (enableFoward && next) {
            var accept = true;
            aboutToLoad(accept);
            if (!accept) {
                return
            }

            stack.push(next)
        }
        pageLoaded(stack.currentPage)
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
        source: enableBackward ? button.style.backImage : ""
        anchors { left: parent.left; top: parent.top }
    }

    BasicListItem {
        id: basicItem
        style: button.style
        dbusModel: button.dbusModel

        anchors { left: backIcon.right; top: parent.top;  right: fowardIcon.left; bottom: parent.bottom }
    }

    Image {
       id: fowardIcon

       height: sourceSize.height
       width: sourceSize.width
       source: next && enableFoward ? button.style.fowardImage : ""
       anchors { top: parent.top; right: parent.right }
    }
}
