import QtQuick
import QtQuick.Templates as T

import ComponentLibrary

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: 12
    rightPadding: 12

    font.pixelSize: EricTheme.componentFontSize

    focusPolicy: Qt.NoFocus

    ////////////////

    background: Rectangle {
        implicitWidth: 80
        implicitHeight: EricTheme.componentHeight

        radius: EricTheme.componentRadius
        opacity: control.enabled ? 1 : 0.66
        color: control.down ? EricTheme.colorComponentDown : EricTheme.colorComponent
        border.width: 2
        border.color: EricTheme.colorComponentBorder
    }

    ////////////////

    contentItem: Text {
        text: control.text
        textFormat: Text.PlainText

        font: control.font
        elide: Text.ElideMiddle
        //wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        opacity: control.enabled ? 1 : 0.66
        color: control.down ? EricTheme.colorComponentContent : EricTheme.colorComponentContent
    }

    ////////////////
}
