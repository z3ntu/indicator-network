import QtQuick 1.1
import SystemUI 1.0

BasicItem {
    id: pageStack

    signal pageLoaded(int pageIndex, variant page)
    signal pageRemoved(variant page)

    property alias currentIndex: pagesView.currentIndex
    property alias currentPage: pagesView.currentItem
    property alias spacing:  pagesView.spacing
    property alias pageWidth: pagesView.pageWidth
    property int maxWidth: 1240
    property int count: pagesModel.count
    property int layout: 0

    style: PageStackStyle { }
    implicitWidth: ((pageWidth + spacing) * 4)

    function push(pageUrl, title) {
        pagesModel.append({"pageUrl": "qrc:/qml/" + pageUrl, "title": title, "object": null })
        pagesView.incrementCurrentIndex()
        return pagesView.currentIndex
    }

    function pop() {
        pagesView.decrementCurrentIndex()
        return pagesView.currentIndex
    }

    ListModel {
        id: pagesModel
    }

    Rectangle {
        id: headerBackground

        color: pageStack.style.headerStyle.backgroundColor
        height: pageStack.style.headerHeight
        anchors { left: parent.left; top:  parent.top; right: parent.right }
    }

    Rectangle {
        id: contentBackground

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
        property int visibleIndex: 0

        function incrementCurrentIndex() {
            if((pagesModel.count -1) > currentIndex) {
                currentIndex += 1
                visibleIndex = currentIndex
            }
        }

        function decrementCurrentIndex() {
            if (currentIndex > 0)
                currentIndex -= 1
        }

        function updateVisibleIndex(index) {
            if (pagesView.visibleIndex != index) {
                pagesView.visibleIndex = index
                while(pagesModel.count -1 > index) {
                    pagesModel.remove(pagesModel.count - 1)
                }
            }
        }

        Repeater {
            id: pageHeader

            model: pagesModel
            delegate: NavigationButton {
                stack: pageStack
                height: pagesView.headerHeight
                width: pagesView.pageWidth
                style: pageStack.style.headerStyle
                x: (index - pagesView.currentIndex) * (pagesView.pageWidth + pagesView.spacing)
                caption: model.title
                enableBackward: index > 0

                Behavior on x {
                    SequentialAnimation {
                        SmoothedAnimation { velocity: 900 }
                    }
                }
            }
        }

        Repeater {
            id: pageContent

            model: pagesModel
            delegate: Loader {
                id: loader

                y: pagesView.headerHeight + pagesView.spacing
                x: (index - pagesView.currentIndex) * (pagesView.pageWidth + pagesView.spacing)
                width: pagesView.pageWidth
                height: pageStack.height
                source: index <= pagesView.visibleIndex ? model.pageUrl : ""

                onLoaded: {
                    pagesView.currentItem = item
                    pageStack.pageLoaded(index, item)
                }

                Binding {
                    target: loader
                    property: "stack"
                    value: pageStack
                    when: loader.status == Loader.Ready
                }

                Behavior on x {
                    SequentialAnimation {
                        SmoothedAnimation { velocity: 600; }
                        ScriptAction {
                            script: pagesView.updateVisibleIndex(pagesView.currentIndex)
                        }
                    }
                }
            }
        }
    }
}
