import QtQuick

import ComponentLibrary

Item { // padded separator
    anchors.left: parent.left
    anchors.right: parent.right
    height: EricTheme.componentMargin + 1

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 1
        color: EricTheme.colorSeparator
    }
}
