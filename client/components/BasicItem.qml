import QtQuick 1.1

Item {
    property QtObject style: null
    property QtObject dbusModel: null
    property bool hasModel: (dbusModel != null)
}
