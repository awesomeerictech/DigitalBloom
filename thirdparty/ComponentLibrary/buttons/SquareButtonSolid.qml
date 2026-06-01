import QtQuick

import ComponentLibrary

SquareButtonImpl {
    property color color: EricTheme.colorPrimary

    colorBackground: color
    colorHighlight: "white"
    colorBorder: Qt.darker(color, 1.02)
    colorIcon: "white"
    flat: false
}
