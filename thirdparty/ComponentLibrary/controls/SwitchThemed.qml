import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.Switch {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    topPadding: 4
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 4

    spacing: 12

    font.pixelSize: EricTheme.componentFontSize

    property int www: 48
    property int hhh: 24

    property color colorText: EricTheme.colorText
    property color colorSubText: EricTheme.colorSubText

    ////////////////

    indicator: Rectangle {
        implicitWidth: control.www
        implicitHeight: EricTheme.componentHeight

        x: control.text ?
               (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) :
               (control.leftPadding + (control.availableWidth - width) / 2)
        y: control.topPadding + (control.availableHeight - height) / 2

        width: control.www
        height: control.hhh
        radius: control.hhh

        color: EricTheme.colorComponentBackground
        border.color: EricTheme.colorComponentBorder
        border.width: EricTheme.componentBorderWidth

        opacity: control.enabled ? 1 : 0.8

        Rectangle {
            x: control.checked ? (parent.width - width) : 0
            Behavior on x { NumberAnimation { duration: 133 } }
            width: 24
            height: width
            radius: (width / 2)
            anchors.verticalCenter: parent.verticalCenter

            color: (control.checked && control.enabled) ? EricTheme.colorPrimary : EricTheme.colorComponentBorder

            Rectangle {
                anchors.fill: parent
                anchors.margins: -10
                z: -1
                radius: (width / 2)
                color: parent.color
                opacity: (control.enabled && control.checkable && (control.pressed || control.hovered || control.visualFocus)) ?
                             (control.checked ? 0.16 : 0.32) : 0
                Behavior on opacity { NumberAnimation { duration: 133 } }
            }
        }
    }

    contentItem: Text {
        leftPadding: !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.mirrored ? control.indicator.width + control.spacing : 0

        opacity: control.enabled ? 1 : 0.66

        text: control.text
        textFormat: Text.PlainText
        font: control.font
        color: control.checked ? control.colorText : control.colorSubText
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    ////////////////
}
