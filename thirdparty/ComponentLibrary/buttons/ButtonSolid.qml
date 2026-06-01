import QtQuick

import ComponentLibrary

ButtonImpl {
    property color color: EricTheme.colorPrimary

    colorBackground: color
    colorHighlight: "white"
    colorBorder: Qt.darker(color, 1.02)
    colorText: "white"
    flat: false
}
