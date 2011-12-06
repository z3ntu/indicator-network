import QtQuick 1.1
import SystemUI 1.0

BasicItem {
    id: slider

    property real value: 0
    property real pageStep: 0.3

    style: SliderStyle { }

    onValueChanged: syncIndicator()
    onWidthChanged: syncIndicator()

    function syncIndicator() {
        if (value < 0)
            value = 0.0
        else if (value > 1.0)
            value = 1.0
        indicator.x = Math.max(width*value - indicator.width, 0)
    }

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

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (mouse.x < indicator.x)
                slider.value -= slider.pageStep
            else
                slider.value += slider.pageStep
        }
    }

    Rectangle {
        id: indicator

        width: slider.style.indicatorWidth
        height: parent.height
        color: slider.style.indicatorColor
        onXChanged: {
            slider.value = (indicator.x + indicator.width) / slider.width
        }

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

        MouseArea {
            anchors.fill: parent
            onClicked: slider.value -= slider.pageStep
        }
    }

    Image {
        id: plusIcon

        height: sourceSize.height
        width: sourceSize.width
        source: slider.style.plusImage
        anchors { top: parent.top; right: parent.right; rightMargin: slider.style.margin }

        MouseArea {
            anchors.fill: parent
            onClicked: slider.value += slider.pageStep
        }
    }
}
