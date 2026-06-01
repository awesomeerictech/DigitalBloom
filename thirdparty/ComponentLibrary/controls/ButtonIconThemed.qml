import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T
import QtQuick.Layouts

import ComponentLibrary

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    leftPadding: 12
    rightPadding: 12+6
    spacing: 6

    font.pixelSize: EricTheme.componentFontSize
    font.bold: false

    focusPolicy: Qt.NoFocus

    // settings
    property url source
    property int sourceSize: UtilsNumber.alignTo(height * 0.666, 2)
    property int layoutDirection: Qt.LeftToRight

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

    contentItem: RowLayout {
        spacing: control.spacing
        layoutDirection: control.layoutDirection

        IconSvg {
            source: control.source
            width: control.sourceSize
            height: control.sourceSize

            visible: control.source.toString().length
            Layout.maximumWidth: control.sourceSize
            Layout.maximumHeight: control.sourceSize
            Layout.alignment: Qt.AlignVCenter

            opacity: control.enabled ? 1 : 0.66
            color: EricTheme.colorComponentContent
        }

        Text {
            text: control.text
            textFormat: Text.PlainText

            visible: control.text
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            font: control.font
            elide: Text.ElideMiddle
            //wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            opacity: control.enabled ? 1 : 0.66
            color: EricTheme.colorComponentContent
        }
    }

    ////////////////
}
