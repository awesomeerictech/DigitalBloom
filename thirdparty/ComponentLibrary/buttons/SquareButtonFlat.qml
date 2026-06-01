import QtQuick

import ComponentLibrary

SquareButtonImpl {
    property color color: EricTheme.colorPrimary

    colorBackground: color
    colorHighlight: "white"
    colorBorder: color
    colorIcon: "white"
    flat: true
}
