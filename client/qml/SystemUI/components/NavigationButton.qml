import QtQuick 1.1

BasicButton {
    id: button

    property QtObject stack: null
    property QtObject next: null
    property bool enableBackward: false
    property bool enableFoward: dbusModel ? dbusModel.hasSubmenu : false
    property alias caption: basicItem.caption
    property alias description: basicItem.description

    signal pageLoaded(variant page)
    signal aboutToLoad(variant event)

    style: NavigationButtonStyle { }
    implicitWidth: backIcon.width + basicItem.implicitWidth + fowardIcon.width
    implicitHeight: Math.max(backIcon.height, fowardIcon.height)

    QtObject {
        id: pageEvent

        property bool skip: false
        property QtObject page : null
    }

    onClicked: {
        if (!stack)
            return

        if (enableBackward) {
            stack.pop()
        } else if (enableFoward && next) {
            var event = pageEvent
            event.skip = false
            event.page = next
            aboutToLoad(event)
            if (event.skip) {
                return
            }
            var page = stack.push(next)
            pageLoaded(page)
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
