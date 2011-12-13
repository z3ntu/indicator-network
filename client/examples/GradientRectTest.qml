import QtQuick 1.1
import SystemUI 1.0

Item {
    width: 300
    height: 800

    GradientRectangle {
        anchors.fill:  parent
        percentage: 0.5
        colors: [ "blue", "red", "green" ]
    }
}
