

import QtQuick
import Fluid as Fluid
import "../.."

Item {
    Fluid.Button {
        anchors.centerIn: parent
        text: qsTr("Open")
        onClicked: navDrawer.open()
    }
}
