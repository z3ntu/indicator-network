import QtQuick 1.1

QtObject {
    property color backgroundColor: "gray"
    property int margin: 6
    property string fontFamily: "helvetica"
    property int fontPointSize: 18
    property bool fontBold: false

    property string minusImage: "images/minus.png"
    property string plusImage: "images/plus.png"

    // Indicator
    property color indicatorColor: "#f25126"
    property int indicatorWidth: 12

    // Gradient
    property color startColor: "#f3f1ef"
    property color centerColor: "#f9b29f"
    property color finalColor: "#f9a87d"
}
