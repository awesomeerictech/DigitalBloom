import QtQuick

import ComponentLibrary

ButtonImpl {
    property color color: EricTheme.colorPrimary

    colorBackground: Qt.rgba(color.r, color.g, color.b, 0.2)
    colorHighlight: color
    colorBorder: colorBackground
    colorText: color
    flat: true
}
