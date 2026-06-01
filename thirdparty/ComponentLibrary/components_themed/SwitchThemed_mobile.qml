import QtQuick
import QtQuick.Controls
//import QtQuick.Controls.impl
//import QtQuick.Templates  as T

import ComponentLibrary


Switch {
    id: control

    padding: 4
    spacing: 12
    font.pixelSize: EricTheme.fontSizeComponent

    indicator: Rectangle {
        implicitWidth: 48
        implicitHeight: EricTheme.componentHeight

        x: control.leftPadding
        y: (parent.height / 2) - (height / 2)
        width: 40
        height: 16
        radius: 16

        color: control.checked ? EricTheme.colorSecondary : EricTheme.colorComponentDown
        Behavior on color { ColorAnimation { duration: 133; easing.type: Easing.InOutCirc; } }

        Rectangle {
            x: control.checked ? (parent.width - width) : 0
            Behavior on x { NumberAnimation { duration: 133 } }
            width: 24
            height: width
            radius: (width / 2)
            anchors.verticalCenter: parent.verticalCenter

            color: control.checked ? EricTheme.colorPrimary : EricTheme.colorComponent
            border.width: control.checked ? 0 : 1
            border.color: EricTheme.colorComponentBorder
        }
    }

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        verticalAlignment: Text.AlignVCenter

        text: control.text
        textFormat: Text.PlainText
        font: control.font

        color: control.checked ? EricTheme.colorText : EricTheme.colorSubText
        opacity: enabled ? 1.0 : 0.33
    }
}
