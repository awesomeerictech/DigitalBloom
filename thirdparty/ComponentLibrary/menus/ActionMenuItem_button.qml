import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.Button {
    id: control

    anchors.left: parent.left
    anchors.leftMargin: EricTheme.componentBorderWidth
    anchors.right: parent.right
    anchors.rightMargin: EricTheme.componentBorderWidth

    leftInset: 8
    rightInset: 8
    rightPadding: 16
    leftPadding: 16

    height: EricTheme.componentHeight

    focusPolicy: Qt.NoFocus

    opacity: control.enabled ? 1 : 0.66

    // settings
    property int index
    property url source
    property int sourceSize: 20
    property int sourceRotation: 0
    property int layoutDirection: Qt.RightToLeft

    ////////////////

    background: Item {
        implicitHeight: EricTheme.componentHeight

        Rectangle {
            anchors.fill: parent
            radius: EricTheme.componentRadius

            color: EricTheme.colorComponent
            //Behavior on color { ColorAnimation { duration: 133 } }

            opacity: control.enabled && control.hovered ? 1 : 0
            //Behavior on opacity { OpacityAnimator { duration: 233 } }
        }

        RippleThemed {
            anchors.fill: parent
            anchor: control

            pressed: control.pressed
            active: control.enabled && (control.down || control.visualFocus)
            color: Qt.rgba(EricTheme.colorComponentDown.r, EricTheme.colorComponentDown.g, EricTheme.colorComponentDown.b, 0.66)
        }

        layer.enabled: true
        layer.effect: MultiEffect {
            maskEnabled: true
            maskInverted: false
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
            maskSpreadAtMax: 0.0
            maskSource: ShaderEffectSource {
                sourceItem: Rectangle {
                    width: control.width
                    height: control.height
                    radius: EricTheme.componentRadius
                }
            }
        }
    }

    ////////////////

    contentItem: RowLayout {
        spacing: EricTheme.componentMargin
        layoutDirection: control.layoutDirection

        IconSvg {
            Layout.preferredWidth: control.sourceSize
            Layout.preferredHeight: control.sourceSize

            source: control.source
            rotation: control.sourceRotation
            color: EricTheme.colorIcon
        }

        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: control.sourceSize

            text: control.text
            textFormat: Text.PlainText
            font.bold: false
            font.pixelSize: EricTheme.componentFontSize
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: EricTheme.colorComponentText
        }
    }

    ////////////////
}
