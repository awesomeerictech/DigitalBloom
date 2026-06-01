import QtQuick 2.15
import QtQuick.Controls.impl 2.15
import QtQuick.Templates 2.15 as T

import ThemeEngine 1.0

T.RangeSlider {
    id: control
    implicitWidth: 200
    implicitHeight: EricTheme.componentHeight
    padding: 4
    topPadding: 10

    first.value: 0.25
    second.value: 0.75
    snapMode: T.RangeSlider.SnapAlways

    // colors
    property string colorBg: EricTheme.colorForeground
    property string colorFg: EricTheme.colorPrimary
    property string colorTxt: "white"

    ////////////////////////////////////////////////////////////////////////////

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight / 2) - (height / 2)
        width: control.availableWidth
        height: 4
        radius: 2
        color: control.colorBg
        clip: true

        property int ticksCount: ((to - from) / stepSize)

        Repeater {
            width: control.availableWidth
            model: (background.ticksCount-1)
            Rectangle {
                x: ((control.availableWidth / background.ticksCount) * (index+1))
                width: 1; height: 4;
                color: EricTheme.colorComponentBorder
            }
        }

        Rectangle {
            x: (control.first.visualPosition * control.availableWidth)
            width: (control.second.visualPosition * parent.width) - x
            height: parent.height
            radius: 2
            color: control.colorFg
/*
            clip: true

            Repeater {
                width: control.availableWidth
                model: (background.ticksCount-1)
                Rectangle {
                    x: (((control.availableWidth) / background.ticksCount) * (index+1))
                    width: 1; height: 4;
                    color: EricTheme.colorComponentBackground
                }
            }
*/
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    first.handle: Rectangle {
        x: control.leftPadding + Math.round((first.visualPosition * control.availableWidth) - (width / 2))
        y: 0
        width: 16
        height: 12

        color: first.pressed ? EricTheme.colorSecondary : EricTheme.colorPrimary
/*
        Text {
            anchors.fill: parent
            y: 2

            text: first.value.toFixed(1)
            font.pixelSize: 8
            font.bold: true
            color: control.colorTxt
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
*/
        Rectangle {
            width: 10
            height: 10
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.bottom

            z: -1
            rotation: 45
            color: parent.color
        }
    }

    ////////////////////////////////////////////////////////////////////////////

    second.handle: Rectangle {
        x: control.leftPadding + Math.round((second.visualPosition * control.availableWidth) - (width / 2))
        y: 0
        width: 16
        height: 12

        color: second.pressed ? EricTheme.colorSecondary : EricTheme.colorPrimary
/*
        Text {
            anchors.fill: parent
            y: 2

            text: second.value.toFixed(1)
            font.pixelSize: 8
            font.bold: true
            color: control.colorTxt
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
*/
        Rectangle {
            width: 10
            height: 10
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.bottom

            z: -1
            rotation: 45
            color: parent.color
        }
    }
}
