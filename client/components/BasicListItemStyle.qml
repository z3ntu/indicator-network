import QtQuick 1.1

ButtonStyle {
    property string selectedImage: "images/selected.png"
    property string unselectedImage: "images/unselected.png"

    property color descriptionForegroundColor: "gray"
    property string descriptionFontFamily: "helvetica"
    property int descriptionFontPointSize: 18
    property bool descriptionFontBold: false
    property bool descriptionFontItalic: true

    backgroundColor: "#ffffff"
    foregroundColor: "#000000"
}
