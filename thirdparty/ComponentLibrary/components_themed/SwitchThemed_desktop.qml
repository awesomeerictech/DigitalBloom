import QtQuick
import QtQuick.Controls
//import QtQuick.Controls.impl
//import QtQuick.Templates as T

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
        width: 48
        height: (width / 2)
        radius: (width / 2)

        color: EricTheme.colorComponentBackground
        Behavior on color { ColorAnimation { duration: 133; easing.type: Easing.InOutCirc; } }

        border.color: EricTheme.colorComponentBorder
        border.width: EricTheme.componentBorderWidth

        Rectangle {
            x: control.checked ? (parent.width - width) : 0
            Behavior on x { NumberAnimation { duration: 133 } }
            width: 24
            height: width
            radius: (width / 2)
            anchors.verticalCenter: parent.verticalCenter

            color: control.checked ? EricTheme.colorPrimary : EricTheme.colorComponentBorder
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
