import QtQuick 1.1
import SystemUI 0.1

BasicItem {
    id: slider

    property real value: (indicator.x + indicator.width) / width

    style: SliderStyle { }

    GradientRectangle {
        id: gradient

        anchors.fill: parent
        percentage: value
        colors: [
            slider.style.startColor,
            slider.style.centerColor,
            slider.style.finalColor
        ]
    }

    Rectangle {
        id: indicator

        width: slider.style.indicatorWidth
        height: parent.height
        color: slider.style.indicatorColor

        MouseArea {
            anchors.fill: parent
            drag.target: indicator
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: slider.width - indicator.width
        }
    }

    Image {
        id: minusIcon

        height: sourceSize.height
        width: sourceSize.width
        source: slider.style.minusImage
        anchors { left: parent.left; top: parent.top; leftMargin: slider.style.margin }
    }

    Image {
        id: plusIcon

        height: sourceSize.height
        width: sourceSize.width
        source: slider.style.plusImage
        anchors { top: parent.top; right: parent.right; rightMargin: slider.style.margin }
    }
}
