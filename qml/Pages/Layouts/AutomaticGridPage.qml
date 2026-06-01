

import QtQuick
import Fluid as Fluid
import "../.."

Flickable {
    clip: true
    contentHeight: Math.max(layout.implicitHeight, height)

    Fluid.ScrollBar.vertical: Fluid.ScrollBar {}

    Fluid.AutomaticGrid {
        id: layout

        anchors.fill: parent

        cellWidth: 100
        cellHeight: cellWidth

        model: 250
        delegate: Rectangle {
            color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1.0)
            width: 100
            height: width

            Text {
                anchors.centerIn: parent
                text: index + 1
            }
        }
    }
}
