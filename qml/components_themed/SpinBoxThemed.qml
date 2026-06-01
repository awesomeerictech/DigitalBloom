import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates  as T
import Qt5Compat.GraphicalEffects

import ThemeEngine 1.0
import "qrc:/js/UtilsNumber.js" as UtilsNumber

T.SpinBox {
    id: control
    implicitWidth: 128
    implicitHeight: EricTheme.componentHeight

    value: 50
    editable: true
    font.pixelSize: EricTheme.fontSizeComponent

    property string legend

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        implicitWidth: 128
        implicitHeight: EricTheme.componentHeight

        radius: EricTheme.componentRadius
        color: EricTheme.colorComponentBackground

        Rectangle {
            width: control.height
            height: control.height
            anchors.verticalCenter: parent.verticalCenter
            x: control.mirrored ? 0 : control.width - width
            color: control.up.pressed ? EricTheme.colorComponentDown : EricTheme.colorComponent
        }
        Rectangle {
            width: control.height
            height: control.height
            anchors.verticalCenter: parent.verticalCenter
            x: control.mirrored ? control.width - width : 0
            color: control.down.pressed ? EricTheme.colorComponentDown : EricTheme.colorComponent
        }

        Rectangle {
            anchors.fill: parent
            radius: EricTheme.componentRadius
            color: "transparent"
            border.width: EricTheme.componentBorderWidth
            border.color: control.focus ? EricTheme.colorPrimary : EricTheme.colorComponentBorder
        }

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Rectangle {
                x: control.x
                y: control.y
                width: control.width
                height: control.height
                radius: EricTheme.componentRadius
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    contentItem: Item {
        TextInput {
            width: parent.width - (control.height * 2)
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: control.legend ? -(contentWidth / 2) : 0
            anchors.verticalCenter: parent.verticalCenter

            color: EricTheme.colorComponentText
            selectionColor: EricTheme.colorText
            selectedTextColor: "white"

            text: control.textFromValue(control.value, control.locale)
            font: control.font
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter

            readOnly: !control.editable
            validator: control.validator
            inputMethodHints: Qt.ImhFormattedNumbersOnly

            onEditingFinished: {
                control.value = control.valueFromText(text, control.locale)
                control.focus = false
            }

            Text {
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.horizontalCenterOffset: parent.contentWidth
                anchors.verticalCenter: parent.verticalCenter

                visible: control.legend
                color: EricTheme.colorComponentText

                text: control.legend
                textFormat: Text.PlainText
                font: control.font
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    up.indicator: Item {
        implicitWidth: EricTheme.componentHeight
        implicitHeight: EricTheme.componentHeight

        width: control.height
        height: control.height
        anchors.verticalCenter: control.verticalCenter
        x: control.mirrored ? 0 : control.width - width

        Rectangle {
            anchors.centerIn: parent
            width: UtilsNumber.round2(parent.height * 0.4)
            height: 2
            color: enabled ? EricTheme.colorComponentContent : EricTheme.colorSubText
        }
        Rectangle {
            anchors.centerIn: parent
            width: 2
            height: UtilsNumber.round2(parent.height * 0.4)
            color: enabled ? EricTheme.colorComponentContent : EricTheme.colorSubText
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    down.indicator: Item {
        implicitWidth: EricTheme.componentHeight
        implicitHeight: EricTheme.componentHeight

        width: control.height
        height: control.height
        anchors.verticalCenter: control.verticalCenter
        x: control.mirrored ? control.width - width : 0

        Rectangle {
            anchors.centerIn: parent
            width: UtilsNumber.round2(parent.height * 0.4)
            height: 2
            color: enabled ? EricTheme.colorComponentContent : EricTheme.colorSubText
        }
    }

    ////////////////////////////////////////////////////////////////////////////
}
