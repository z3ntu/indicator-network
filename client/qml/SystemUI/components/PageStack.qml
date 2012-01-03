import QtQuick 1.1
import SystemUI 1.0

BasicItem {
    id: pageStack

    signal pageLoaded(int pageIndex, variant page)
    signal pageRemoved(variant page)

    property alias currentIndex: pagesView.currentIndex
    property alias currentPage: pagesView.currentItem
    property alias visiblePages: pagesView.visiblePages
    property alias spacing:  pagesView.spacing
    property alias pageWidth: pagesView.pageWidth
    property int maxWidth: 1240
    property int count: pagesModel.count

    style: PageStackStyle { }
    implicitWidth: ((pageWidth + spacing) * visiblePages)

    // Push a new page on the stack
    function push(pageUrl, title) {
        pagesModel.append({"pageUrl": "qrc:/qml/" + pageUrl, "title": title, "object": null })
        pagesView.incrementCurrentIndex()
    }

    // Pop a page from the stack
    function pop() {
        pagesView.decrementCurrentIndex()
    }

    // Auxiliar model used to store stack pages information
    ListModel {
        id: pagesModel
    }

    // Header backgroud
    Rectangle {
        id: headerBackground

        color: pageStack.style.headerStyle.backgroundColor
        height: pageStack.style.headerHeight
        anchors { left: parent.left; top:  parent.top; right: parent.right }
    }

    // Body background
    Rectangle {
        id: bodyBackground

        color: pageStack.style.backgroundColor
        anchors { left: parent.left; top: headerBackground.bottom; right: parent.right; bottom: parent.bottom }
    }

    Item {
        id: pagesView

        property int pageWidth: 300
        property int headerHeight: pageStack.style.headerHeight
        property int spacing: 3
        property int currentIndex: -1
        property variant currentItem: null
        property int visiblePages: 1
        property int visibleIndex: 0

        function incrementCurrentIndex() {
            if((pagesModel.count -1) > currentIndex) {
                currentIndex += 1
                visibleIndex = currentIndex
            }
        }

        function decrementCurrentIndex() {
            if (currentIndex > 0) {
                var remove = false
                if (currentIndex < visiblePages) {
                    remove = true
                }

                currentIndex -= 1
                if (remove) {
                    updateVisibleIndex(currentIndex)
                }
            }
        }

        WorkerScript {
            id: engine
            source: "modelworker.js"
            onMessage: {
                if (messageObject.action == "popDone") {
                    pagesModel.remove(messageObject.position)
                }
            }
        }


        function updateVisibleIndex(index) {
            if (index == -1)
                return

            if (pagesView.visibleIndex > index) {
                var count = pagesModel.count
                while(count > (index  + 1)) {
                    var msg = { "action" :  "pop", "position": count - 1 }
                    engine.sendMessage(msg)
                    count--
                }
                pagesView.visibleIndex = index
            }
        }

        Repeater {
            id: pageHeader

            model: pagesModel
            delegate: NavigationButton {
                id: headerButton

                property int indexOffset: pagesView.currentIndex >= pagesView.visiblePages ? (index - (pagesView.currentIndex - (pagesView.visiblePages-1))) : index
                property int xOffset: indexOffset * (pagesView.pageWidth + pagesView.spacing)

                stack: pageStack
                height: pagesView.headerHeight
                width: pagesView.pageWidth
                style: pageStack.style.headerStyle
                x: xOffset
                caption: model.title
                enableBackward: index > 0
                opacity: pagesView.visiblePages > 1 ? pagesView.currentIndex == index ? 1.0 : 0.3 : 1.0

                Behavior on x {
                    SmoothedAnimation { velocity: 900 }
                }
            }
        }

        Repeater {
            id: pageBody

            model: pagesModel
            delegate: Loader {
                id: bodyLoader

                property bool active
                property int indexOffset: pagesView.currentIndex >= pagesView.visiblePages ? (index - (pagesView.currentIndex - (pagesView.visiblePages-1))) : index
                property int xOffset: indexOffset * (pagesView.pageWidth + pagesView.spacing)

                y: pagesView.headerHeight + pagesView.spacing
                x: xOffset
                width: pagesView.pageWidth
                height: pageStack.height
                source: model.pageUrl
                opacity: pagesView.visiblePages > 1 ? pagesView.currentIndex == index ? 1.0 : 0.3 : 1.0

                onLoaded: {
                    pagesView.currentItem = item
                    pageStack.pageLoaded(index, item)
                }

                Binding {
                    target: bodyLoader
                    property: "stack"
                    value: pageStack
                    when: bodyLoader.status == Loader.Ready
                }

                Behavior on x {
                    SequentialAnimation {
                        SmoothedAnimation { velocity: 600; }
                        ScriptAction {
                            script: {
                                if (index == pagesView.currentIndex)
                                    pagesView.updateVisibleIndex(index)
                            }
                        }
                    }
                }
            }
        }
    }
}
