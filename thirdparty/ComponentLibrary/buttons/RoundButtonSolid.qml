import QtQuick

import ComponentLibrary

RoundButtonImpl {
    property color color: EricTheme.colorPrimary

    colorBackground: color
    colorHighlight: "white"
    colorBorder: Qt.darker(color, 1.02)
    colorIcon: "white"
    flat: false
}
