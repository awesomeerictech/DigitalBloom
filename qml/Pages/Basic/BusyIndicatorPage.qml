
import QtQuick
import Fluid as Fluid
import "../.." as Components

Components.StyledPage {
    Fluid.BusyIndicator {
        anchors.centerIn: parent
        running: true
    }
}
