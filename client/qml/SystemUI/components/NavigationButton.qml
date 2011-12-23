import QtQuick 1.1

BasicButton {
    id: button

    property QtObject stack: null
    property string next: ""
    property bool enableBackward: false
    property bool enableFoward: dbusModel ? dbusModel.hasSubmenu : false
    property alias caption: basicItem.caption
    property alias description: basicItem.description

    signal pageLoaded(variant newPage)
    signal aboutToLoad(variant accept)

    style: NavigationButtonStyle { }
    implicitWidth: backIcon.width + basicItem.implicitWidth + fowardIcon.width
    implicitHeight: Math.max(backIcon.height, fowardIcon.height)

    QtObject {
        id: pageEvent

        property bool skip: false
        property variant page : null
    }

    onClicked: {
        if (!stack)
            return

        var index = 0
        if (enableBackward) {
            index = stack.pop()
        } else if (enableFoward && next) {
            var event = pageEvent
            event.skip = false
            event.page = next
            aboutToLoad(event)
            if (event.skip) {
                return
            }
            stack.push(next, caption)
            pageLoaded(stack.currentPage)
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
