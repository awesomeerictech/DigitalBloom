import QtQuick

import ComponentLibrary

ButtonImpl {
    property color color: EricTheme.colorPrimary

    colorBackground: "transparent"
    colorHighlight: color
    colorBorder: Qt.rgba(color.r, color.g, color.b, 0.5)
    colorText: color
    flat: true
}
