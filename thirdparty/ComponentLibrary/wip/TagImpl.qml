import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.Control {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: 8
    rightPadding: 8

    // settings
    font.pixelSize: EricTheme.componentFontSize
    font.bold: false

    // text
    property string text: "TAG"

    // colors
    property color colorBackground: EricTheme.colorPrimary
    property color colorBorder: EricTheme.colorComponentBorder
    property color colorText: "white"

    ////////////////

    background: Rectangle {
        implicitWidth: 48
        implicitHeight: 26

        radius: EricTheme.componentRadius
        color: control.colorBackground
        border.width: EricTheme.componentBorderWidth
        border.color: control.colorBorder
    }

    ////////////////

    contentItem: Text {
        text: control.text
        textFormat: Text.PlainText

        color: control.colorText
        font: control.font

        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    ////////////////
}
