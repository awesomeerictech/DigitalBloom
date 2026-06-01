import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.CheckBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 8
    spacing: 8
    font.pixelSize: EricTheme.componentFontSize

    property int layoutDirection: Qt.LeftToRight

    ////////////////

    indicator: Rectangle {
        implicitWidth: EricTheme.componentHeight
        implicitHeight: EricTheme.componentHeight

        x: (layoutDirection === Qt.LeftToRight) ?
               (control.leftPadding) :
               (control.width - control.padding - width)
        y: (parent.height / 2) - (height / 2)
        width: 24
        height: 24
        radius: EricTheme.componentRadius

        opacity: control.enabled ? 1 : 0.66
        color: EricTheme.colorComponentBackground
        border.width: EricTheme.componentBorderWidth
        border.color: (control.enabled && control.checkable &&
                       (control.down || control.hovered)) ?
                          EricTheme.colorSecondary : EricTheme.colorComponentBorder

        Rectangle {
            anchors.centerIn: parent
            width: 12
            height: 12
            radius: (EricTheme.componentRadius / 2)

            color: EricTheme.colorSecondary
            opacity: control.checked ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 133 } }
        }
    }

    ////////////////

    contentItem: Text {
        leftPadding: (layoutDirection === Qt.LeftToRight) ? control.indicator.width + control.spacing : 0
        rightPadding: (layoutDirection === Qt.RightToLeft) ? control.indicator.width + control.spacing : 0
        verticalAlignment: Text.AlignVCenter

        text: control.text
        textFormat: Text.PlainText
        font: control.font
        wrapMode: Text.WordWrap

        opacity: control.enabled ? 1 : 0.66
        color: control.checked ? EricTheme.colorText : EricTheme.colorSubText
    }

    ////////////////
}
