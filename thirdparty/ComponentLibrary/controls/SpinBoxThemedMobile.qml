import QtQuick
import QtQuick.Effects
import QtQuick.Controls.impl
import QtQuick.Templates as T

import ComponentLibrary

T.SpinBox {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             up.implicitIndicatorHeight, down.implicitIndicatorHeight)

    leftPadding: padding + (control.mirrored ? (up.indicator ? up.indicator.width : 0) : (down.indicator ? down.indicator.width : 0))
    rightPadding: padding + (control.mirrored ? (down.indicator ? down.indicator.width : 0) : (up.indicator ? up.indicator.width : 0))

    font.pixelSize: EricTheme.componentFontSize

    property string legend

    ////////////////

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    ////////////////

    background: Rectangle {
        implicitWidth: 140
        implicitHeight: EricTheme.componentHeight

        radius: EricTheme.componentRadius
        color: EricTheme.colorComponentBackground
        opacity: control.enabled ? 1 : 0.66

        Rectangle {
            width: control.height
            height: control.height
            anchors.verticalCenter: parent.verticalCenter
            x: control.mirrored ? 0 : control.width - width
            color: control.up.pressed ? EricTheme.colorComponentDown : EricTheme.colorComponent
            opacity: control.up.hovered ? 0.8 : 1
            Behavior on opacity { NumberAnimation { duration: 133 } }
        }
        Rectangle {
            width: control.height
            height: control.height
            anchors.verticalCenter: parent.verticalCenter
            x: control.mirrored ? control.width - width : 0
            color: control.down.pressed ? EricTheme.colorComponentDown : EricTheme.colorComponent
            opacity: control.down.hovered ? 0.8 : 1
            Behavior on opacity { NumberAnimation { duration: 133 } }
        }

        Rectangle {
            anchors.fill: parent
            radius: EricTheme.componentRadius
            color: "transparent"
            border.width: EricTheme.componentBorderWidth
            border.color: control.focus ? EricTheme.colorPrimary : EricTheme.colorComponentBorder
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

    contentItem: Item {
        Row {
            anchors.centerIn: parent
            spacing: 4

            opacity: control.enabled ? 1 : 0.66

            TextInput {
                height: control.height
                anchors.verticalCenter: parent.verticalCenter

                color: EricTheme.colorComponentText
                selectionColor: EricTheme.colorPrimary
                selectedTextColor: "white"
                selectByMouse: control.editable

                text: control.value
                font: control.font
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter

                readOnly: !control.editable
                validator: control.validator
                inputMethodHints: Qt.ImhDigitsOnly

                onEditingFinished: {
                    //var v = parseInt(text)
                    //if (text.length <= 0) v = control.from
                    //if (isNaN(v)) v = control.from
                    //if (v < control.from) v = control.from
                    //if (v > control.to) v = control.to

                    //control.value = v
                    //control.valueModified()

                    control.focus = false
                    focus = false
                }
                Keys.onBackPressed: {
                    control.focus = false
                    focus = false
                }
            }

            Text {
                height: control.height
                anchors.verticalCenter: parent.verticalCenter

                visible: control.legend
                color: EricTheme.colorComponentText
                opacity: 0.66

                text: control.legend
                textFormat: Text.PlainText
                font: control.font
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
            }
        }
    }

    ////////////////

    up.indicator: Item {
        implicitWidth: EricTheme.componentHeight
        implicitHeight: EricTheme.componentHeight

        width: control.height
        height: control.height
        x: control.mirrored ? 0 : control.width - width
        anchors.verticalCenter: control.verticalCenter

        opacity: enabled ? 1 : 0.5

        Rectangle {
            anchors.centerIn: parent
            width: UtilsNumber.round2(parent.height * 0.4)
            height: 2
            color: EricTheme.colorComponentContent
        }
        Rectangle {
            anchors.centerIn: parent
            width: 2
            height: UtilsNumber.round2(parent.height * 0.4)
            color: EricTheme.colorComponentContent
        }
    }

    ////////////////

    down.indicator: Item {
        implicitWidth: EricTheme.componentHeight
        implicitHeight: EricTheme.componentHeight

        width: control.height
        height: control.height
        x: control.mirrored ? control.width - width : 0
        anchors.verticalCenter: control.verticalCenter

        opacity: enabled ? 1 : 0.5

        Rectangle {
            anchors.centerIn: parent
            width: UtilsNumber.round2(parent.height * 0.4)
            height: 2
            color: EricTheme.colorComponentContent
        }
    }

    ////////////////
}
