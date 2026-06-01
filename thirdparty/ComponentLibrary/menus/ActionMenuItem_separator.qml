import QtQuick

import ComponentLibrary

Item { // action menu separator
    anchors.left: parent.left
    anchors.leftMargin: EricTheme.componentMargin - 4
    anchors.right: parent.right
    anchors.rightMargin: EricTheme.componentMargin - 4

    height: (EricTheme.componentMargin / 2) + 1

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        height: 1
        color: EricTheme.colorSeparator
    }
}
