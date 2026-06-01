import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Controls.impl
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

import ComponentLibrary

T.ItemDelegate {
    id: control

    implicitWidth: parent.width
    implicitHeight: EricTheme.componentHeightL

    padding: EricTheme.componentMargin
    spacing: EricTheme.componentMargin
    verticalPadding: 0

    property string source
    property int sourceSize: 24
    property int sourceRotation: 0
    property color sourceColor: EricTheme.colorIcon

    //property string text
    property int textSize: 13
    property color textColor: EricTheme.colorText

    property bool iconAnimated: false
    property string iconAnimation // fade or rotate

    ////////////////

    background: Item {
        implicitHeight: EricTheme.componentHeightL

        Item {
            anchors.fill: parent
            anchors.margins: 4
            anchors.leftMargin: 8
            anchors.rightMargin: 8

            RippleThemed {
                anchors.fill: parent
                anchor: control

                pressed: control.pressed
                active: enabled && (control.down || control.visualFocus || control.hovered)
                color: Qt.rgba(EricTheme.colorForeground.r, EricTheme.colorForeground.g, EricTheme.colorForeground.b, 0.5)
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
                        x: background.x
                        y: background.y
                        width: background.width
                        height: background.height
                        radius: EricTheme.componentRadius
                    }
                }
            }
        }
    }

    ////////////////

    contentItem: RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: screenPaddingLeft + EricTheme.componentMargin
        anchors.right: parent.right
        anchors.rightMargin: screenPaddingRight + EricTheme.componentMargin / 2

        opacity: control.enabled ? 1 : 0.66

        Item {
            Layout.preferredWidth: EricTheme.componentHeightL - EricTheme.componentMargin
            Layout.preferredHeight: EricTheme.componentHeightL
            Layout.alignment: Qt.AlignTop

            IconSvg {
                anchors.left: parent.left
                anchors.leftMargin: (32 - control.sourceSize) / 2
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: (control.height !== EricTheme.componentHeightL) ? -(EricTheme.componentMargin / 2) : 0

                width: control.sourceSize
                height: control.sourceSize
                rotation: control.sourceRotation

                source: control.source
                color: control.sourceColor

                NumberAnimation on rotation { // rotate animation // icon only
                    duration: 2000
                    from: 0
                    to: 360
                    loops: Animation.Infinite
                    running: (control.iconAnimated && control.iconAnimation === "rotate")
                    alwaysRunToEnd: true
                }
                SequentialAnimation on opacity { // fade animation // icon only
                    loops: Animation.Infinite
                    running: (control.iconAnimated && control.iconAnimation === "fade")
                    alwaysRunToEnd: true

                    PropertyAnimation { to: 0.33; duration: 750; }
                    PropertyAnimation { to: 1; duration: 750; }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            text: control.text
            color: control.textColor
            wrapMode: Text.WordWrap
            font.bold: true
            font.pixelSize: control.textSize
        }
    }

    ////////////////
}
