import QtQuick

import ComponentLibrary

SquareButtonImpl {
    colorBackground: EricTheme.colorBackground
    colorHighlight: Qt.lighter(colorBackground, 0.92)

    colorRipple: Qt.rgba(colorHighlight.r, colorHighlight.g, colorHighlight.b, 0.5)
    colorBorder: colorBackground
    colorIcon: EricTheme.colorIcon
    flat: true
}
