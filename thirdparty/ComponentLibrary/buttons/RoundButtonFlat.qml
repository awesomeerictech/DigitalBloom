import QtQuick

import ComponentLibrary

RoundButtonImpl {
    property color color: EricTheme.colorPrimary

    colorBackground: color
    colorHighlight: "white"
    colorBorder: color
    colorIcon: "white"
    flat: true
}
