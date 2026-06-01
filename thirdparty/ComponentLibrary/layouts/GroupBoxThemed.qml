import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.GroupBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding,
                            implicitLabelWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: 12
    padding: 12

    topPadding: title ? 4 + (implicitLabelWidth > 0 ? implicitLabelHeight + spacing : 0) : padding

    // colors
    property color colorBackground: EricTheme.colorForeground
    property color colorBorder: EricTheme.colorSeparator

    ////////////////

    background: Rectangle {
        y: control.topPadding - control.bottomPadding
        width: parent.width
        height: parent.height - control.topPadding + control.bottomPadding

        radius: EricTheme.componentRadius
        color: control.colorBackground
        border.width: 2
        border.color: control.colorBorder
    }

    ////////////////

    label: Text {
        x: control.leftPadding
        width: control.availableWidth

        text: control.title
        font: control.font
        color: EricTheme.colorText
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    ////////////////
}
