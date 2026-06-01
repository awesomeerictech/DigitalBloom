

import QtQuick
import QtQuick.Controls.Material
import Fluid as Fluid
import "../.."

Item {
    Fluid.Wave {
        id: wave
        anchors.fill: parent
        Rectangle {
            anchors.fill: parent
            color: Material.accentColor
        }
    }
    Fluid.Button {
        anchors.centerIn: parent
        text: qsTr("Toggle")
        onClicked: {
            if (wave.open)
                wave.closeWave(parent.width - wave.size, parent.height - wave.size)
            else
                wave.openWave(0, 0)
        }
    }
}
