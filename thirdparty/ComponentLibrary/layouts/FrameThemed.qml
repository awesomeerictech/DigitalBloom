import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.Frame {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    padding: 12

    // colors
    property color colorBackground: EricTheme.colorForeground
    property color colorBorder: EricTheme.colorSeparator

    ////////////////

    background: Rectangle {
        radius: EricTheme.componentRadius
        color: control.colorBackground
        border.width: 2
        border.color: control.colorBorder
    }

    ////////////////
}
