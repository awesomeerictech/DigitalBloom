import QtQuick

import ComponentLibrary

ButtonImpl {
    id: sidebarMenu

    anchors.left: parent.left
    anchors.right: parent.right

    height: EricTheme.componentHeightXL

    layoutAlignment: Qt.AlignLeft

    leftPadding: 16
    rightPadding: 16
    spacing: 12

    text: sidebarMenu.text
    font.bold: checked

    source: "qrc:/IconLibrary/material-symbols/menu.svg"
    sourceSize: 20

    property color color: checked ? EricTheme.colorPrimary : EricTheme.colorSidebarContent
    colorBackground: Qt.rgba(color.r, color.g, color.b, checked ? 0.2 : 1)
    colorHighlight: checked ? EricTheme.colorPrimary : EricTheme.colorSidebarHighlight
    colorBorder: colorBackground
    colorText: checked ? EricTheme.colorPrimary : EricTheme.colorText
    flat: true
}
