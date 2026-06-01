import QtQuick
import QtQuick.Controls
//import QtQuick.Controls.impl
//import QtQuick.Templates  as T

import ThemeEngine 1.0

CheckBox {
    id: control

    padding: 4
    spacing: 12
    font.pixelSize: EricTheme.fontSizeComponent

    indicator: Rectangle {
        implicitWidth: EricTheme.componentHeight
        implicitHeight: EricTheme.componentHeight

        x: control.leftPadding
        y: (parent.height / 2) - (height / 2)
        width: 24
        height: 24
        radius: EricTheme.componentRadius

        color: EricTheme.colorComponentBackground
        border.width: EricTheme.componentBorderWidth
        border.color: control.down ? EricTheme.colorSecondary : EricTheme.colorComponentBorder

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

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        verticalAlignment: Text.AlignVCenter

        text: control.text
        textFormat: Text.PlainText
        font: control.font
        wrapMode: Text.WordWrap

        color: control.checked ? EricTheme.colorText : EricTheme.colorSubText
        opacity: enabled ? 1.0 : 0.33
    }
}
